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
  auto p = store[flag].mutable_get<T>();
  cli.add_option(opts, assign<T>{p}, {options_parser::join(opts, ", "), doc});
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

template <class T>
T get(const Store& store, const std::string& name) {
  if (!store.count(name)) return T{};
  return *store.find(name)->second.get<T>();
}

int main(int argc, char* argv[]) {
  options_parser::Parser app("ls", "try ls command");
  app.add_help();

  Store store;

  add_store_flag(store, app, "a|all", false,
                 "do not ignore entries starting with .");

  add_store_flag(store, app, "A|almost-all", false,
                 "do not list implied . and ..");

  add_store_flag(store, app, "author", false,
                 "with -l, print the author of each file");
  add_store_flag(store, app, "b|escape", false,
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

  add_store_flag(store, app, "B|ignore-backups", false,
                 "do not list implied entries ending with ~");
  add_store_flag(store, app, "c", false,
                 "with -lt: sort by, and show, ctime (time of last"
                 " modification of file status information)"
                 " with -l: show ctime and sort by name"
                 " otherwise: sort by ctime, newest first");
  add_store_flag(store, app, "C", false, "list entries by columns");

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
    std::cout << "flag:" << k_v.first << " value:" << k_v.second.to_str()
              << std::endl;
  }

  return 0;
}
