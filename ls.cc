#include "options_parser/options_parser_lib.h"
#include "options_parser/any.h"

#include <map>

typedef std::map<std::string, options_parser::Any> Store;

template <class T>
struct assign {
  assign(T* ptr) : ptr(ptr) {}

  void operator()(T v) { *ptr = v; }
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
  return *store[name].get<T>();
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

  add_store_flag(store, app, "block-size", (std::string) "",
                 "scale sizes by SIZE before printing them.  E.g.,"
                 " `--block-size=M' prints sizes in units of"
                 " 1,048,576 bytes.  See SIZE format below.");

  app.parse(argc, argv);

  return 0;
}
