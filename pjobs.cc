#include <mutex>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "options_parser/options_parser.h"
// #include "clog/clog.hpp"

using std::string;

std::map<string, string> envs;
std::set<string> clear_envs;

// TODO: mutex with fork
std::mutex stderr_mutex;
void error_message(const string& m) {
  std::lock_guard<std::mutex> lock(stderr_mutex);
  std::cerr << m << std::endl;
}

template <class H, class... More>
typename std::enable_if<!std::is_convertible<H, string>::value>::type
error_message(const string& m, const H& h, const More&... more) {
  error_message(m + std::to_string(h), more...);
}

template <class H, class... More>
typename std::enable_if<std::is_convertible<H, string>::value>::type
error_message(const string& m, const H& h, const More&... more) {
  error_message(m + h, more...);
}

// for every k, v in dict, replace %{k} to v, concurrently.
string replace_pattern(string s, const std::map<string, string>& dict) {
  string r;
  size_t off = 0;
  while (off < s.size()) {
    auto c = s[off];
    if (c != '%') {
      r += c;
      ++off;
      continue;
    }
    if (off + 1 >= s.size() || s[off + 1] == '%') {
      r += c;
      off += 2;
      continue;
    }
    if (s[off + 1] != '{') {
      r += c;
      off += 1;
      continue;
    }
    auto n = s.find('}', off + 1);
    if (n == string::npos) {
      r += s.substr(off);
      off = s.size();
    }
    auto name = s.substr(off + 2, n - off - 2);
    auto it = dict.find(name);
    if (it != dict.end()) {
      r += it->second;
      off = n + 1;
      continue;
    }
    r += "%{";  // invalid/unkonwn pattern are ignored and kept
    off += 2;
  }
  return r;
}

struct ProgramPattern {
  std::vector<string> prog;
  std::vector<std::map<string, string>> dicts;
};

string escape_double_quote(const string& s) {
  string r;
  for (int c : s) {
    if (c == '\\') {
      r += "\\\\";
    } else if (c == '"') {
      r += "\\\"";
    } else if (c == '$') {
      r += "\\$";
    } else {
      r += static_cast<char>(c);
    }
  }
  return r;
}

string quote_argument(const string& s) {
  if (std::all_of(s.begin(), s.end(), ::isalnum)) {
    return s;
  }
  return "\"" + escape_double_quote(s) + "\"";
}

int run_prog_pattern(std::vector<string> prog, std::map<string, string>& dict,
                     bool dry_run) {
  int pid = dry_run ? 0 : fork();
  if (pid == -1) {
    error_message("fork failed:", strerror(errno));
    exit(1);
  }
  if (pid == 0) {
    dict["PID"] = std::to_string(getpid());
    for (auto& a : prog) {
      a = replace_pattern(a, dict);
    }
    if (dry_run) {
      error_message(options_parser::join(prog, " ", &quote_argument));
    } else {
      std::vector<const char*> prog_cargv;
      for (const auto& a : prog) {
        prog_cargv.push_back(a.c_str());
      }
      prog_cargv.push_back(nullptr);
      for (const auto& n_v : envs) {
        auto n = n_v.first;
        auto v = replace_pattern(n_v.second, dict);
        setenv(n.c_str(), v.c_str(), true);
      }
      for (const auto& n : clear_envs) {
        int r = unsetenv(n.c_str());
        if (r != 0) {
          error_message("unset env ", n, " failed: ", strerror(errno));
        }
      }
      execvp(prog_cargv.front(), (char* const*)&prog_cargv[0]);
      error_message("execvp ", options_parser::join(prog, " ", quote_argument),
                    " failed: ", strerror(errno));
      exit(127);
    }
  }
  return pid;
}

void run_prog_patterns(std::vector<ProgramPattern>& prog_patterns,
                       size_t parallel_number, bool dry_run) {
  if (prog_patterns.size() == 0) return;
  size_t num_job = 0;
  for (auto& pp : prog_patterns) {
    if (!pp.dicts.size()) {
      pp.dicts.emplace_back();
    }
    for (auto& dict : pp.dicts) {
      dict["JOB_INDEX"] = std::to_string(num_job++);
    }
  }
  for (auto& pp : prog_patterns) {
    for (auto& dict : pp.dicts) {
      dict["JOB_SIZE"] = std::to_string(num_job);
    }
  }
  size_t pattern_index = 0;
  auto dict_it = prog_patterns[pattern_index].dicts.begin();
  size_t curr_job = 0;
  std::set<int> jobs;
  while (curr_job < num_job || jobs.size()) {
    while (curr_job < num_job && jobs.size() < parallel_number) {
      curr_job++;
      if (dict_it == prog_patterns[pattern_index].dicts.end()) {
        ++pattern_index;
        dict_it = prog_patterns[pattern_index].dicts.begin();
      }
      int pid = run_prog_pattern(prog_patterns[pattern_index].prog, *dict_it++,
                                 dry_run);
      if (pid > 0) {
        jobs.insert(pid);
      }
    }
    if (!jobs.size()) break;
    int status = 0;
    int pid = waitpid(0, &status, 0);
    if (jobs.count(pid)) {
      if (WIFEXITED(status) || WIFSIGNALED(status)) {
        jobs.erase(pid);
        // CLOG(INFO, "wait pid=" << pid, " rest #" << jobs.size());
      }
    }
  }
}

std::map<char, char> sep_left_rights = {{'{', '}'}, {'[', ']'}, {'(', ')'}};

string right_sep(string sep) {
  if (sep.size() && sep_left_rights.count(sep.front())) {
    sep = sep.substr(1) + sep_left_rights[sep.front()];
  } else if (sep.size() && sep_left_rights.count(sep.back())) {
    sep = sep_left_rights[sep.back()] + sep.substr(0, sep.size() - 1);
  }
  return sep;
}

int main(int argc, char* argv[]) {
  options_parser::Parser app(
      string() + "Usage: " + argv[0] +
          " [NAME=VALUE] [options]\n"
          "Run program pattern replaced by variables in parallel.\n",
      "\n"

      "Some built in variables:\n"
      "  JOB_INDEX: job index of the program, starts from zero.\n"
      "  JOB_SIZE: job size of this run\n"
      "  PID: child pid\n"
      "\n"

      "SEP:\n"
      "  For options acceting lots of values, we have to use a separator to"
      " mark the end. SEP can be any raw string. If the SEP starts/ends with "
      "one of '{', '(', '[', we'll expect next sep will ends/starts with one of"
      " '}', ')', ']' respectively.\n"
      "\n"

      "Example:\n"
      "  # run echo 10 times\n"
      "  pjobs --prog --{ echo 'pid=%{PID}' 'seq=%{SEQ}' }-- --values seq -- "
      "$(seq 10) --\n"
      "  # run wget in parallel\n"
      "  pjobs --parallel 100 --prog {-- wget 'www.example.com/%{ID}' --} "
      "--values ID -- $(seq 1000) --\n"
      "\n"

      "Wrote by jiangzuoyan@gmail.com, use it at your own risk.");

  std::vector<ProgramPattern> prog_patterns;
  int parallel_number = 2;
  bool dry_run = false;

  auto sep_values = [](string sep) {
    return options_parser::value()
        .check([sep](string arg) { return arg != right_sep(sep); })
        .many(1)
        .bind([](std::vector<string> vs) {
          return options_parser::value().apply([vs](string ignore) {
            // TODO: clang++ return error here with 'return vs'
            return std::vector<string>{vs};
          });
        });
  };

  auto extend_cross_values = [&](const string& name, std::vector<string> vs) {
    if (!vs.size()) return;
    std::vector<std::map<string, string>> gen;
    if (!prog_patterns.size()) {
      prog_patterns.emplace_back();
    }
    auto& dicts = prog_patterns.back().dicts;
    if (dicts.size() == 0) {
      dicts.emplace_back();
    }
    for (auto v : vs) {
      for (auto kvs : dicts) {
        if (kvs.count(name)) {
          gen.emplace_back(kvs);
        }
        kvs[name] = v;
        gen.emplace_back(std::move(kvs));
      }
    }
    dicts.swap(gen);
  };

  auto extend_list_values = [&](const std::vector<string> vs) {
    if (!vs.size()) return;
    auto first = vs.front();
    first = first.substr(0, first.find('='));
    std::map<string, string> kvs;
    if (!prog_patterns.size()) {
      prog_patterns.emplace_back();
    }
    auto& dicts = prog_patterns.back().dicts;
    for (string kv : vs) {
      auto eq = kv.find('=');
      if (eq == string::npos) {
        error_message("fix me, internal error, --list-values without '='");
        continue;
      }
      auto k = kv.substr(0, eq);
      auto v = kv.substr(eq + 1);
      if (kvs.size() && k == first) {
        dicts.push_back(kvs);
        kvs.clear();
      }
      kvs[k] = v;
    }
    if (kvs.size()) {
      dicts.push_back(kvs);
    }
  };

  auto add_prog_pattern = [&](std::vector<string> prog) {
    if (!prog_patterns.size() || prog_patterns.back().prog.size()) {
      prog_patterns.emplace_back();
    }
    prog_patterns.back().prog = prog;
  };

  auto bool_value = [](bool dft) {
    return options_parser::value<bool>().optional().apply([dft](
        options_parser::Maybe<bool> v) { return v ? get_value(v) : dft; });
  };

  app.add_parser(options_parser::parser());
  app.add_help();
  app.add_option("--dry-run[=true/false]",
                 bool_value(true).assign(&dry_run).ignore_value(),
                 "dry run, i.e. print and exit");
  app.add_option("--prog SEP <prog and argument pattern>... SEP",
                 options_parser::value().bind(sep_values).apply([&](
                     std::vector<string> vs) { add_prog_pattern(vs); }),
                 "add program and arguments pattern,"
                 " where all %{NAME} will be replaced by variable NAME,"
                 " %% will be replaced by %.");
  app.add_option("-v, --values NAME SEP <value>... SEP",
                 value_gather(options_parser::value(),
                              options_parser::value().bind(sep_values))
                     .apply(options_parser::check_invoke(extend_cross_values)),
                 "add variable values, every value add to every dictionary,"
                 "get a cross product.");
  app.add_option("-l, --list-values NAME=VALUE...",
                 options_parser::value()
                     .check([](string a) {
                       return a.find('=') != string::npos && a.front() != '-';
                     })
                     .many()
                     .apply(extend_list_values),
                 "add variable value lists");
  app.add_option("-p, --parallel NUM", &parallel_number,
                 "set number of process in once");
  auto env_option = app.add_option("-e, --env NAME=VALUE",
                                   [](string nv) -> string {
                                     auto eq = nv.find('=');
                                     if (eq == string::npos) {
                                       return "no '=' find";
                                     }
                                     nv[eq] = 0;
                                     envs[nv.c_str()] = nv.c_str() + eq + 1;
                                     return "";
                                   },
                                   "set environment");
  app.add_option(
      options_parser::value().peek().apply([](string a) {
        return a.find('=') != string::npos && a.front() != '-'
                   ? options_parser::MATCH_EXACT
                   : 0;
      }),
      env_option->take,
      {"NAME=VALUE", "set environment, NAME should not start with '-'"});
  app.add_option("-i, --ignore-environment", []() { return !clearenv(); },
                 "start with an empty environment");
  app.add_option("-u, --unset NAME", [](string n) { clear_envs.insert(n); },
                 "remove variable from environment");
  app.parse(argc, argv).check_print();

  run_prog_patterns(prog_patterns, parallel_number, dry_run);

  return 1;
}
