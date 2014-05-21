#ifndef FILE_7D6FBDCB_5360_4543_AFB4_327EDD39CD6A_H
#define FILE_7D6FBDCB_5360_4543_AFB4_327EDD39CD6A_H
#include <cstring>
#include <string>
#include <vector>
#include <functional>
#include <memory>

#include "options_parser/string.h"

namespace options_parser {

struct Arguments {
  Arguments() = default;
  Arguments(const Arguments &) = default;
  Arguments(Arguments &&o) { std::swap(inst_, o.inst_); }
  Arguments &operator=(const Arguments &o) = default;
  Arguments &operator=(Arguments &&o) = default;

  template <class T>
  Arguments(const T &v)
      : inst_(std::make_shared<Holder<T>>(v)) {}

  struct Interface {
    virtual int argc() const = 0;
    virtual int char_at(int idx, int off) const = 0;
    virtual string arg_at(int idx) const = 0;
  };

  template <class T>
  struct Holder : public Interface {
    Holder(const T &v) : inst_(v) {}

    virtual int argc() const { return inst_.argc(); }

    virtual int char_at(int idx, int off) const {
      return inst_.char_at(idx, off);
    }

    virtual string arg_at(int idx) const { return inst_.arg_at(idx); }

    T inst_;
  };

  int argc() const { return inst_->argc(); }

  int char_at(int idx, int off) const { return inst_->char_at(idx, off); }

  string arg_at(int idx) const { return inst_->arg_at(idx); }

 private:
  std::shared_ptr<Interface> inst_;
};

struct ArgvArguments {
  ArgvArguments(int argc, char *argv[]);

  int argc() const;
  int char_at(int idx, int off) const;
  string arg_at(int idx) const;

  int argc_;
  char **argv_;
};

struct VectorStringArguments {
  VectorStringArguments(const std::vector<string> &argv);

  int argc() const;

  int char_at(int idx, int off) const;
  string arg_at(int idx) const;

  std::vector<string> argv_;
};

struct FunctionArguments {
  FunctionArguments(
      std::function<int()> argc, std::function<int(int, int)> char_at,
      std::function<string(int)> arg_at = std::function<string(int)>{});

  std::function<int()> argc;
  std::function<int(int, int)> char_at_;
  std::function<string(int)> arg_at_;

  int char_at(int idx, int off) const;
  string arg_at(int idx) const;
};

}  // namespace options_parser
#endif  // FILE_7D6FBDCB_5360_4543_AFB4_327EDD39CD6A_H
