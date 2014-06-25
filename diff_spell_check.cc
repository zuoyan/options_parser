#include <algorithm>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

#include "options_parser/options_parser.h"
#include "clog/clog.hpp"

struct Source {
  std::string file_;
  int row_;
  int col_;

  Source(const Source&) = default;
  Source(Source&&) = default;
  Source(Source&) = default;
  Source& operator=(const Source&) = default;
  Source& operator=(Source&&) = default;

  Source(std::string file, int row, int col)
      : file_(file), row_(row), col_(col) {}

  friend std::ostream& operator<<(std::ostream& os, const Source& s) {
    return os << s.file_ << ':' << s.row_ << ':' << s.col_;
  }
};

struct Word {
  std::string word_;
  std::vector<Source> sources_;
};

struct Empty {
  template <class T>
  void operator()(const T&) {}
};

template <class WithWord>
void SplitUnitAsWords(const std::string& str, WithWord&& with_word,
                      size_t col_off = 0) {
  int num_alpha = 0;
  int num_lower = 0;
  int num_upper = 0;
  for (auto c : str) {
    if (isalpha(c)) {
      num_alpha ++;
      if (isupper(c)) {
        num_upper++;
      } else if (islower(c)) {
        num_lower++;
      }
    }
  }
  if (num_alpha != str.size()) {
    size_t off = 0;
    while (off < str.size()) {
      if (!isalpha(str[off])) {
        ++off;
        continue;
      }
      auto n = off + 1;
      while (n < str.size() && isalpha(str[n])) {
        ++n;
      }
      auto sub = str.substr(off, n - off);
      SplitUnitAsWords(sub, with_word, col_off + off);
      off = n;
    }
    return;
  }
  if (num_lower == str.size() || num_upper == str.size()) {
    std::string word = str;
    if (num_upper) {
      for (char& c : word) {
        c = tolower(c);
      }
    }
    with_word(word, col_off);
    return;
  }
  // CamelCase
  size_t off = 0;
  while (off < str.size()) {
    size_t n = off + 1;
    while (n < str.size() && ::islower(str[n])) {
      ++n;
    }
    auto word = str.substr(off, n - off);
    word[0] = tolower(word[0]);
    with_word(word, col_off + off);
    off = n;
  }
}

size_t find_unit(const std::string& line, size_t off) {
  size_t n = off + 1;
  while (n < line.size()) {
    auto c = line[n];
    if (isalpha(c)) {
      ++n;
      continue;
    }
    if (strchr("_-", c)) {
      if (n + 1 < line.size()
          && isalpha(line[n+1])
          && isalpha(line[n - 1])) {
        ++n;
        continue;
      }
    }
    break;
  }
  return n;
}

template <class WithUnit>
void SplitUnits(const std::string& line, WithUnit&& with_unit,
                size_t col_off = 0) {
  size_t off = 0;
  while (off < line.size()) {
    if (isspace(line[off])) {
      ++off;
      continue;
    }
    if (!isalpha(line[off])) {
      ++off;
      continue;
    }
    size_t n = find_unit(line, off);
    auto unit = line.substr(off, n - off);
    with_unit(unit, col_off + off);
    off = n;
  }
}

bool starts_with(const std::string& s, const std::string& prefix) {
  return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

bool starts_with_one(const std::string& s) { return false; }

template <class... More>
bool starts_with_one(const std::string& s, const std::string& prefix,
                     const More&... more) {
  return starts_with(s, prefix) || starts_with_one(s, more...);
}

void shout_sub() {}

template <class H, class... More>
void shout_sub(const H& h, const More&... more) {
  std::cerr << h;
  shout_sub(more...);
}

template <class... More>
void shout(const More&... more) {
  shout_sub(more...);
  std::cerr << std::endl;
}

struct InputFile {
  std::unique_ptr<std::ifstream> ifs_;
  std::istream* is_;

  std::string open(const std::string& file) {
    is_ = nullptr;
    if (file == "-") {
      is_ = &std::cin;
      return "";
    }
    if (file.size()) {
      ifs_.reset(new std::ifstream(file));
      if (ifs_->good()) {
        is_ = ifs_.get();
        return "";
      } else {
        return "open file '" + file + "' failed: " + strerror(errno);
      }
    }
    return "empty filename";
  }

  std::string line() {
    std::string line;
    if (is_->good()) {
      std::getline(*is_, line);
    }
    return line;
  }

  std::istream& is() { return *is_; }

  explicit operator bool() { return is_ && is_->good(); }
};

std::string get_relpath(std::string file, std::string start) {
  if (!start.size() || !file.size()) return file;
  auto start_p = realpath(start.c_str(), NULL);
  auto file_p = realpath(file.c_str(), NULL);
  if (!start_p || !file_p) {
    if (!start_p) {
      CLOG(ERROR, "realpath", start, "failed", strerror(errno));
    } else {
      CLOG(ERROR, "realpath", file, "failed", strerror(errno));
    }
    free(start_p);
    free(file_p);
    return file;
  }
  start = start_p;
  file = file_p;
  free(start_p);
  free(file_p);
  auto file_list = options_parser::split(file, "/");
  auto start_list = options_parser::split(start, "/");
  size_t common = 0;
  while (common < file_list.size()
         && common < start_list.size()
         && file_list[common] == start_list[common]) {
    ++common;
  }
  static const std::string pardir = "../";
  std::string par;
  for (size_t i = common; i < start_list.size(); ++i) {
    par += pardir;
  }
  file = par +
         options_parser::join(file_list.begin() + common, file_list.end(), "/");
  return file;
}

template <class T>
struct LayerVariable {
  int* current_layer_;
  int layer_;
  T value_;

  LayerVariable() = delete;

  LayerVariable(int* current_layer) : current_layer_(current_layer) {
    layer_ = *current_layer_;
  }

  LayerVariable(int *current_layer, const T& v) : LayerVariable(current_layer) {
    value_ = v;
  }

  LayerVariable(const LayerVariable&) = default;
  LayerVariable(LayerVariable&&) = default;

  LayerVariable& operator=(const T& v) {
    if (*current_layer_ >= layer_) {
      value_ = v;
      layer_ = *current_layer_;
    }
    return *this;
  }

  LayerVariable& operator=(const LayerVariable&) = default;
  LayerVariable& operator=(LayerVariable&&) = default;

  const T& Value() const { return value_; }

  T& Value() { return value_; }

  friend const T& get_value(const LayerVariable& t) { return t.Value(); }
};

int main(int argc, char* argv[]) {
  std::unordered_map<std::string, std::vector<Source>> more_words;
  std::unordered_set<std::string> known_words;
  bool learn_words_from_diff_context = false;
  bool learn_words_from_minus = true;
  int default_layer = 1, command_layer = 2;
  int current_layer = 0;
  LayerVariable<std::string> dict_file(&current_layer, "/usr/share/dict/words");
  std::string home_dir;
  {
    if (getenv("HOME")) {
      home_dir = getenv("HOME");
    }
    if (home_dir.empty()) {
      auto pwd = getpwuid(geteuid());
      if (pwd) {
        home_dir = pwd->pw_dir;
      }
    }
  }
  LayerVariable<std::string> conf_file(
      &current_layer, home_dir + "/" + ".diff_spell_check.conf");
  current_layer = command_layer;

  auto load_dict = [&](const std::string& file) -> std::string {
    InputFile input;
    auto m = input.open(file);
    while (input) {
      auto line = input.line();
      known_words.insert(line);
    }
    return m;
  };

  auto learn_line = [&](const std::string& line) {
    SplitUnits(line, [&](const std::string& unit, int unit_off) {
      if (known_words.insert(unit).second) {
        VCLOG(1, "add unit", unit);
      }
      SplitUnitAsWords(unit, [&](const std::string& word, int word_off) {
        if (known_words.insert(word).second) {
          VCLOG(1, "add word", word);
        }
      });
    });
  };

  auto check_line = [&](const std::string& line, const std::string& file,
                           int row, int col = 0) {
    SplitUnits(line, [&](const std::string& unit, int unit_off) {
      if (known_words.count(unit)) return;
      SplitUnitAsWords(unit, [&](const std::string& word, int word_off) {
        if (known_words.count(word)) return;
        auto it_new = more_words.emplace(word, std::vector<Source>{});
        int c = unit_off + word_off + col;
        if (it_new.second) {
          VCLOG(1, "add new check word", word, "@", file, row, c);
        } else {
          VCLOG(2, "add old check word", word, "@", file, row, c);
        }
        it_new.first->second.emplace_back(file, row, c);
      });
    });
  };

  std::string diff_file;
  int diff_row = 0;
  int diff_row_num = 0;
  int diff_row_index = 0;
  auto take_diff_line = [&](const std::string& line) {
    if (!line.size()) return;
    if (diff_row_index < diff_row_num) {
      if (!starts_with_one(line, " ", "-", "+")) {
        shout("expect diff line starts with ' ', '-', '+': ", line);
        return;
      }
      if (line[0] == ' ') {
        diff_row_index++;
        if (learn_words_from_diff_context) {
          learn_line(line);
        }
      }
      if (line[0] == '+') {
        int row = diff_row + diff_row_index++;
        // off start with 0, but col start at 1
        check_line(line, diff_file, row);
      }
      if (line[0] == '-') {
        if (learn_words_from_minus) {
          learn_line(line);
        }
      }
      return;
    }
    if (starts_with(line, "+++ ")) {
      auto pos = line.rfind(' ');
      if (pos == std::string::npos) {
        shout("expect a space to separate file name: ", line);
        return;
      }
      auto filename = line.substr(pos + 1);
      if (starts_with(filename, "b/")) {
        filename = filename.substr(2);
      }
      diff_file = filename;
      diff_row_num = 0;
      return;
    }
    if (starts_with(line, "@@ ")) {
      auto pos = line.find('+');
      if (pos == std::string::npos) {
        shout("expect a +line,#lines: ", line);
        return;
      }
      auto start = atoi(line.c_str() + pos + 1);
      pos = line.find(',', pos);
      if (pos == std::string::npos) {
        shout("expect a +line,#lines: ", line);
        return;
      }
      auto num = atoi(line.c_str() + pos + 1);
      diff_row = start;
      diff_row_num = num;
      diff_row_index = 0;
      return;
    }
  };

  auto take_diff = [&](std::istream& is) {
    diff_row_index = 0;
    diff_row_num = 0;
    diff_row = 0;
    std::string line;
    while (is.good()) {
      std::getline(is, line);
      if (is.fail()) break;
      take_diff_line(line);
    }
  };

  bool print_raw_more_words = false;

  bool has_check = false;

  std::string relpath_start;
  std::string filepath_start;

  options_parser::Parser app(
      "Usage: diff_spell_check [option]...\n"
      "Spell check over diff.\n"
      "Learn words from diff minus, context parts or file, command line,"
      " and check the spelling over new words,"
      " print all not known words line by line as"
      " 'file:row:col: TYPO-WORD'.",
      "Example:\n"
      "  diff_spell_check --help\n"
      "  git diff -U1000 | diff_spell_check\n"
      "\n"
      "The config file are just lines as like comand line, see --help. For "
      "example:\n"
      "  --default-dict-file $HOME/.words\n"
      "  --load-dict $HOME/.prog.words\n"
      "  --words $USER\n"
      "\n"
      "Wrote by Changsheng Jiang(jiangzuoyan@gmail.com),"
      " use it at your own risk.");
  options_parser::Parser app_default(
      "default options, must be at first to override", "");
  app.add_parser(options_parser::parser());

  app_default.add_option("--default-dict-file FILE", &dict_file,
                         "set/clear the default dict file");
  app_default.add_option("--default-config-file FILE", &conf_file,
                         "set/clear the default config file");
  bool app_default_done = false;
  auto check_load_default = [&]() -> std::string {
    if (app_default_done) {
      return "";
    }
    app_default_done = true;
    current_layer = default_layer;
    if (conf_file.Value().size()) {
      auto pr = app.parse_file(conf_file.Value());
      if (pr.error && get_value(pr.error) != "open-failed") {
        auto error = pr.error_full ? pr.error_full : pr.error;
        return get_value(error);
      }
    }
    current_layer = command_layer;
    if (dict_file.Value().size()) {
      load_dict(dict_file.Value());
    }
    return "";
  };
  app_default.add_option(options_parser::value().peek().apply([&](std::string) {
                           return app_default_done ? 0 : 1;
                         }),
                         check_load_default,
                         "parse default value")->hide_from_help();
  app.add_parser(app_default, 1);
  app.add_help();
  app.add_option("--load-dict <words file>", load_dict, "Load words from file");
  app.add_option(
      "-l, --learn-words-from-file <file>...",
      options_parser::value()
          .cons(options_parser::value().not_option().many())
          .apply([&](const std::vector<std::string>& files) -> std::string {
            for (const auto& file : files) {
              InputFile input;
              auto m = input.open(file);
              while (input) {
                auto line = input.line();
                learn_line(line);
              }
              if (m.size()) {
                return m;
              }
            }
            return "";
          }),
      "learn words from file");
  app.add_option("--word WORD...",
                 options_parser::value()
                     .not_option()
                     .apply([&](std::string w) { known_words.insert(w); })
                     .many(),
                 "add words as known");
  app.add_option(
      "--check-file FILE...",
      options_parser::value()
          .cons(options_parser::value().not_option().many())
          .apply([&](const std::vector<std::string>& files) -> std::string {
            has_check = true;
            for (const auto& file: files) {
              InputFile input;
              auto m = input.open(file);
              size_t row = 0;
              while (input) {
                auto line = input.line();
                check_line(line, file, ++row, 1);
              }
              if (m.size()) {
                return m;
              }
            }
            return "";
          }),
      "check words from file");
  app.add_option(
      "--check-diff FILE...",
      options_parser::value()
          .cons(options_parser::value().not_option().many())
          .apply([&](const std::vector<std::string>& files) -> std::string {
            for (const auto& file : files) {
              has_check = true;
              InputFile input;
              auto m = input.open(file);
              if (input) {
                take_diff(input.is());
              }
              if (m.size()) {
                return m;
              }
            }
            return "";
          }),
      "check diff");
  app.add_option("-c, --learn-words-from-diff-context=[true/false]",
                 options_parser::value<bool>()
                     .optional(true)
                     .assign(&learn_words_from_diff_context)
                     .ignore_value(),
                 "learn words from diff context")
      ->append_value_document(&learn_words_from_diff_context);
  app.add_option("-m, --learn-words-from-minus=[true/false]",
                 options_parser::value<bool>()
                     .optional(true)
                     .assign(&learn_words_from_minus)
                     .ignore_value(),
                 "learn words from minus in diff")
      ->append_value_document(&learn_words_from_minus);
  app.add_option("--relpath PATH", &relpath_start,
                 "print file path related to PATH");
  app.add_option("--filepath PATH", &filepath_start,
                 "all file path related to PATH");
  app.add_option("--print-raw-more-words",
                 [&]() { print_raw_more_words = true; },
                 "print more words in raw then exit")->hide_from_help();
  app.add_option("--config-file FILE...",
                 options_parser::value()
                     .bind(&options_parser::config_file)
                     .cons(options_parser::value()
                               .not_option()
                               .bind(&options_parser::config_file)
                               .many())
                     .ignore_value(),
                 "take command line options as config file");

  app.parse(argc, argv).check_print();

  if (!has_check) {
    check_load_default();
    take_diff(std::cin);
  }

  if (print_raw_more_words) {
    for (const auto& word_sources : more_words) {
      std::cout << word_sources.first << ':'
                << options_parser::join(word_sources.second, ", ") << std::endl;
    }
    return 1;
  }

  if (filepath_start.size()) {
    if (filepath_start.back() != '/') {
      filepath_start += '/';
    }
    for (auto& word_sources : more_words) {
      for (auto& source : word_sources.second) {
        if (source.file_.size() && source.file_.front() != '/') {
          source.file_ = filepath_start + source.file_;
        }
      }
    }
  }

  if (relpath_start.size()) {
    for (auto& word_sources : more_words) {
      for (auto& source : word_sources.second) {
        source.file_ = get_relpath(source.file_, relpath_start);
      }
    }
  }

  {
    auto more_words_tmp = std::move(more_words);
    for (auto& word_sources : more_words_tmp) {
      if (known_words.count(word_sources.first)) {
        continue;
      }
      more_words.insert(word_sources);
    }
  }

  for (auto& word_sources : more_words) {
    for (auto& source : word_sources.second) {
      std::cout << source << ": " << word_sources.first << std::endl;
    }
  }
  if (more_words.size()) {
    return 1;
  }

  return 0;
}
