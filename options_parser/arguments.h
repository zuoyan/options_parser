#ifndef FILE_E40896CE_3D98_4B9B_8D42_12E756B9F29A_H
#define FILE_E40896CE_3D98_4B9B_8D42_12E756B9F29A_H
#include <cstring>
#include <string>
#include <vector>

namespace options_parser {

struct Arguments {
  inline virtual ~Arguments() {}
  virtual int argc() const = 0;
  virtual int char_at(int idx, int off) const = 0;
  virtual std::string arg_at(int idx) const = 0;
};

struct ArgcArgvArguments : Arguments {
  ArgcArgvArguments(int argc, char *argv[]) {
    argc_ = argc;
    argv_ = argv;
  }

  virtual int argc() const { return argc_; }

  virtual int char_at(int idx, int off) const {
    if (idx >= argc_) return -1;
    if ((size_t)off >= strlen(argv_[idx])) return -1;
    return argv_[idx][off];
  }

  virtual std::string arg_at(int idx) const {
    if (idx >= argc()) return std::string();
    return argv_[idx];
  }

  int argc_;
  char **argv_;
};

struct VectorArgvArguments : Arguments {
  VectorArgvArguments(const std::vector<std::string> *argv) { argv_ = argv; }

  virtual int argc() const { return argv_->size(); }

  virtual int char_at(int idx, int off) const {
    if (idx >= argc()) return -1;
    if ((size_t)off >= (*argv_)[idx].size()) return -1;
    return (*argv_)[idx][off];
  }

  virtual std::string arg_at(int idx) const {
    if (idx >= argc()) return std::string();
    return (*argv_)[idx];
  }

  const std::vector<std::string> *argv_;
};
}  // namespace options_parser
#endif  // FILE_E40896CE_3D98_4B9B_8D42_12E756B9F29A_H
