#ifndef FILE_BD1BFE84_A637_4106_98AB_29A59177578F_H
#define FILE_BD1BFE84_A637_4106_98AB_29A59177578F_H
#include <cassert>

namespace options_parser {

inline ArgvArguments::ArgvArguments(int argc, char *argv[]) {
  argc_ = argc;
  argv_ = argv;
}

inline int ArgvArguments::argc() const { return argc_; }

inline int ArgvArguments::char_at(int idx, int off) const {
  if (idx >= argc_) return -1;
  if ((size_t)off >= strlen(argv_[idx])) return -1;
  return argv_[idx][off];
}

inline string ArgvArguments::arg_at(int idx) const {
  if (idx >= argc()) return string();
  return argv_[idx];
}

inline VectorStringArguments::VectorStringArguments(
    const std::vector<string> &argv)
    : argv_(argv) {}

inline int VectorStringArguments::argc() const { return argv_.size(); }

inline int VectorStringArguments::char_at(int idx, int off) const {
  if (idx >= argc()) return -1;
  if ((size_t)off >= argv_[idx].size()) return -1;
  return argv_[idx][off];
}

inline string VectorStringArguments::arg_at(int idx) const {
  if (idx >= argc()) return string();
  return argv_[idx];
}

inline FunctionArguments::FunctionArguments(
    std::function<int()> argc, std::function<int(int, int)> char_at,
    std::function<string(int)> arg_at)
    : argc(argc), char_at_(char_at), arg_at_(arg_at) {}

inline int FunctionArguments::char_at(int idx, int off) const {
  if (char_at_) return char_at_(idx, off);
  assert(!arg_at_);
  auto s = arg_at(idx);
  if (s.size() >= off) return -1;
  return s.data()[off];
}

inline string FunctionArguments::arg_at(int idx) const {
  if (arg_at_) return arg_at_(idx);
  assert(char_at_);
  string ret;
  int off = 0;
  while (true) {
    int c = char_at(idx, off++);
    if (c == -1) break;
    ret.push_back(c);
  }
  return ret;
}

}  // namespace options_parser
#endif  // FILE_BD1BFE84_A637_4106_98AB_29A59177578F_H
