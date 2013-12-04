#ifndef FILE_F7270B23_B818_49F3_B123_1716E9FECE6C_H
#define FILE_F7270B23_B818_49F3_B123_1716E9FECE6C_H

#ifndef OPTIONS_PARSER_NO_WORDEXP
#include <wordexp.h>
#endif

#include <vector>
#include <string>

namespace options_parser {

#ifndef OPTIONS_PARSER_NO_WORDEXP
inline std::vector<string> wordexp(const string& input) {
  std::vector<string> ret;
  wordexp_t p;
  int r = wordexp(input.c_str(), &p, 0);
  if (r) return ret;
  for (size_t i = 0; i < p.we_wordc; ++i) {
    ret.push_back(p.we_wordv[i]);
  }
  wordfree(&p);
  return ret;
}
#endif

inline std::vector<string> split_space_separated(
    const string& input) {
  std::vector<string> ret;
  size_t off = 0;
  char quote = 0;
  string word;
  bool pre_quote = false;
  while (off < input.size()) {
    char c = input[off];
    if (quote == c) {
      quote = 0;
      off += 1;
      continue;
    }
    if (quote == '\'') {
      word.push_back(c);
      off += 1;
      continue;
    }
    if (c == '\\' && off + 1 < input.size()) {
      char n = input[off + 1];
      switch (n) {
        case 'n':
          word.push_back('\n');
          off += 2;
          continue;
          break;
        case 'r':
          word.push_back('\n');
          off += 2;
          continue;
          break;
        case 't':
          word.push_back('\t');
          off += 2;
          continue;
          break;
        case ' ':
          word.push_back(' ');
          off += 2;
          continue;
          break;
        default:
          word.push_back(n);
          off += 2;
          continue;
      }
    }
    if (quote) {
      word.push_back(c);
      off += 1;
      continue;
    }
    if (isspace(c)) {
      if (pre_quote || word.size()) {
        ret.push_back(word);
        pre_quote = false;
      }
      word.clear();
      off += 1;
      continue;
    }
    if (c == '"' || c == '\'') {
      quote = c;
      pre_quote = true;
      off += 1;
      continue;
    }
    word.push_back(c);
    off += 1;
  }
  if (word.size() || (!quote && pre_quote)) {
    ret.push_back(word);
  }
  return ret;
}

inline std::vector<string> expand(const string& input) {
#ifndef OPTIONS_PARSER_NO_WORDEXP
  return wordexp(input);
#endif
  return split_space_separated(input);
}

}  // namespace options_parser
#endif // FILE_F7270B23_B818_49F3_B123_1716E9FECE6C_H
