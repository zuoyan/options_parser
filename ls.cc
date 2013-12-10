#include "options_parser/options_parser_lib.h"
#include "options_parser/any.h"

#include <map>

typedef std::map<std::string, options_parser::Any> Store;

template <class T>
struct assign {
  assign(T* ptr) : ptr(ptr) {}

  void operator()(T v) const { *ptr = v; }
  T * ptr;
};

template <class T>
void add_store_flag(Store& store, options_parser::Parser& cli,
                    const std::string& flags, const T& default_value,
                    const std::string& name, const std::string& doc) {
  auto opts = options_parser::split(flags, "|");
  std::string flag = opts.back();
  for (auto& o : opts) {
    if (o.size() == 1) {
      o = "-" + o;
    } else {
      o = "--" + o;
    }
  }
  store[flag] = default_value;
  auto p = store[flag].mutable_get<T>();
  cli.add_option(opts, assign<T>{p},
                 {options_parser::join(opts, ", ") + "=" + name, doc});
}

template <class T>
void add_store_flag(Store& store, options_parser::Parser& cli,
                    const std::string& flags, const T& default_value,
                    const std::string& doc) {
  add_store_flag(store, cli, flags, default_value, "?", doc);
}

void add_store_flag(Store& store, options_parser::Parser& cli,
                    const std::string& flags, const bool& default_value,
                    const std::string& doc) {
  auto opts = options_parser::split(flags, "|");
  std::string flag = opts.back();
  for (auto& o : opts) {
    if (o.size() == 1) {
      o = "-" + o;
    } else {
      o = "--" + o;
    }
  }
  store[flag] = default_value;
  auto p = store[flag].mutable_get<bool>();
  cli.add_option(opts, [p]() { *p = true; },
                 {options_parser::join(opts, ", "), doc});
}

void add_store_flag(Store& store, options_parser::Parser& cli,
                    const std::string& flags, const std::string& doc) {
  add_store_flag(store, cli, flags, false, doc);
}

template <class T>
T get(const Store& store, const std::string& name) {
  if (!store.count(name)) return T{};
  return *store.find(name)->second.get<T>();
}

int main(int argc, char* argv[]) {
  options_parser::Parser app("ls", "try ls command");
  app.add_help();

  Store store;

  add_store_flag(store, app, "a|all", "do not ignore entries starting with .");

  add_store_flag(store, app, "A|almost-all", "do not list implied . and ..");

  add_store_flag(store, app, "author",
                 "with -l, print the author of each file");
  add_store_flag(store, app, "b|escape",
                 "print C-style escapes for nongraphic characters");

  auto str_to_size = [](std::string bs) {
    std::istringstream is(bs);
    size_t v;
    std::string post;
    is >> v;
    if (is.fail()) {
      v = 1;
      is.clear();
    }
    is >> post;
    if (post == "G") v *= 1024 * 1024 * 1024;
    if (post == "M") v *= 1024 * 1024;
    if (post == "K") v *= 1024;
    return v;
  };

  app.add_option("--block-size",
                 [&](std::string bs) { store["block-size"] = str_to_size(bs); },
                 {"--block-size=SIZE",
                  "scale sizes by SIZE before printing them.  E.g.,"
                  " `--block-size=M' prints sizes in units of"
                  " 1,048,576 bytes.  See SIZE format below."});

  add_store_flag(store, app, "B|ignore-backups",
                 "do not list implied entries ending with ~");
  add_store_flag(store, app, "c",
                 "with -lt: sort by, and show, ctime (time of last"
                 " modification of file status information)"
                 " with -l: show ctime and sort by name"
                 " otherwise: sort by ctime, newest first");
  add_store_flag(store, app, "C", "list entries by columns");

  store["color"] = std::string("never");

  app.add_option("--color", options_parser::optional_value().apply([&](
                                options_parser::Maybe<std::string> v) {
                              std::string when = "always";
                              if (v) {
                                when = get_value(v);
                              }
                              store["color"] = when;
                            }),
                 {"--color[=WHEN]",
                  "colorize the output.  WHEN defaults to 'always'"
                  " or can be 'never' or 'auto'.  More info below"});

  add_store_flag(store, app, "d|directory", false,
                 "list directory entries instead of contents,"
                 " and do not dereference symbolic links");

  add_store_flag(store, app, "D|dired", false,
                 "generate output designed for Emacs' dired mode");

  app.add_option("-f", [&]() {
      store["all"] = false;
      store["sort"] = false;
      store["list"] = false;
      store["size"] = 0;
      store["color"] = std::string("never");
                       },
                 "do not sort, enable -aU, disable -ls --color");

  add_store_flag(store, app, "F|classify", false,
                 "append indicator (one of */=>@|) to entries");

  add_store_flag(store, app, "file-type", false,
                 "likewise, except do not append '*'");

  add_store_flag(store, app, "format", std::string(), "WORD",
                 "across -x, commas -m, horizontal -x, long -l,"
                 " single-column -1, verbose -l, vertical -C");
  add_store_flag(store, app, "full-time", false,
                 "like -l --time-style=full-iso");

  add_store_flag(store, app, "g", false,
                 "like -l, but do not list owner");

  add_store_flag(store, app, "group-directories-first", false,
                 "group directories before files."
                 " augment with a --sort option, but any"
                 " use of --sort=none (-U) disables grouping");

  add_store_flag(store, app, "G|no-group", false,
                 "in a long listing, don't print group names");

  add_store_flag(store, app, "h|human-readable", false,
                 "with -l, print sizes in human readable format"
                 " (e.g., 1K 234M 2G)");

  add_store_flag(store, app, "si", "likewise, but use powers of 1000 not 1024");

  add_store_flag(store, app, "H|dereference-command-line",
                 false, "follow symbolic links listed on the command line");

  add_store_flag(store, app, "dereference-command-line-symlink-to-dir", false,
                 "follow each command line symbolic link"
                 " that points to a directory");

  add_store_flag(store, app, "hide", std::string(), "PATTERN",
                 "do not list implied entries matching shell PATTERN"
                 " (overridden by -a or -A)");

  add_store_flag(store, app, "indicator-style", std::string(), "WORD",
                 "append indicator with style WORD to entry names:"
                 " none (default), slash (-p),"
                 " file-type (--file-type), classify (-F)");

  add_store_flag(store, app, "i|inode", "print the index number of each file");

  add_store_flag(store, app, "I|ignore", std::string(), "PATTERN",
                 "do not list implied entries matching shell PATTERN");

  app.add_option("-k|--kibibytes",
                 [&]() { store["block-size"] = (size_t)1024; },
                 {"-k, --kibibytes", "use 1024-byte blocks"});

  add_store_flag(store, app, "l", "use a long listing format");

  add_store_flag(store, app, "L|dereference",
                 "when showing file information for a symbolic"
                 " link, show information for the file the link"
                 " references rather than for the link itself");
  add_store_flag(store, app, "m",
                 "fill width with a comma separated list of entries");
  add_store_flag(store, app, "n|numeric-uid-gid",
                 "like -l, but list numeric user and group IDs");

  app.parse(argc, argv);

  auto parse_result = app.parse(argc, argv);

  if (parse_result.error) {
    std::cerr << *parse_result.error.get() << std::endl;
    if (parse_result.error_full) {
      std::cerr << *parse_result.error_full.get() << std::endl;
    }
    return 1;
  }

  for (auto k_v : store) {
    std::cout << k_v.first << ": " << k_v.second.to_str() << std::endl;
  }

  return 0;
}
