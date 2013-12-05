#ifndef FILE_BD1BFE84_A637_4106_98AB_29A59177578F_H
#define FILE_BD1BFE84_A637_4106_98AB_29A59177578F_H
#include "options_parser/config.h"
#include <cassert>

namespace options_parser {

OPTIONS_PARSER_IMP ArgvArguments::ArgvArguments(int argc, char *argv[]) {
  argc_ = argc;
  argv_ = argv;
}

OPTIONS_PARSER_IMP int ArgvArguments::argc() const { return argc_; }

OPTIONS_PARSER_IMP int ArgvArguments::char_at(int idx, int off) const {
  if (idx >= argc_) return -1;
  if ((size_t)off >= strlen(argv_[idx])) return -1;
  return argv_[idx][off];
}

OPTIONS_PARSER_IMP string ArgvArguments::arg_at(int idx) const {
  if (idx >= argc()) return string();
  return argv_[idx];
}

OPTIONS_PARSER_IMP VectorStringArguments::VectorStringArguments(
    const std::vector<string> &argv)
    : argv_(argv) {}

OPTIONS_PARSER_IMP int VectorStringArguments::argc() const {
  return argv_.size();
}

OPTIONS_PARSER_IMP int VectorStringArguments::char_at(int idx, int off) const {
  if (idx >= argc()) return -1;
  if ((size_t)off >= argv_[idx].size()) return -1;
  return argv_[idx][off];
}

OPTIONS_PARSER_IMP string VectorStringArguments::arg_at(int idx) const {
  if (idx >= argc()) return string();
  return argv_[idx];
}

OPTIONS_PARSER_IMP FunctionArguments::FunctionArguments(
    std::function<int()> argc, std::function<int(int, int)> char_at,
    std::function<string(int)> arg_at)
    : argc(argc), char_at_(char_at), arg_at_(arg_at) {}

OPTIONS_PARSER_IMP int FunctionArguments::char_at(int idx, int off) const {
  if (char_at_) return char_at_(idx, off);
  assert(!arg_at_);
  auto s = arg_at(idx);
  if (s.size() >= static_cast<size_t>(off)) return -1;
  return s.data()[off];
}

OPTIONS_PARSER_IMP string FunctionArguments::arg_at(int idx) const {
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
