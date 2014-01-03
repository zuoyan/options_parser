#ifndef FILE_C8B83754_2BB1_4F59_9DE9_E4E510196300_H
#define FILE_C8B83754_2BB1_4F59_9DE9_E4E510196300_H

#include "options_parser/string.h"

#include <vector>
#include <cassert>

namespace options_parser {
namespace line_breaking {

inline std::vector<string> break_string(const string &text, size_t width,
                                        bool indent_as_first_space = true) {
  std::vector<string> ret;
  size_t indent = 0;
  if (indent_as_first_space) {
    while (indent < text.size() && isspace(text[indent])) ++indent;
  }
  string indent_str(indent, ' ');
  auto tokens = split(text, " ");
  string line;
  for (const string &token : tokens) {
    if (!token.size()) continue;
    if (line.size() && line.size() + token.size() + 1 + indent > width) {
      ret.push_back(indent_str + line);
      line.clear();
    }
    if (line.size())
      line += " " + token;
    else
      line = token;
  }
  if (line.size()) ret.push_back(indent_str + line);
  return ret;
}

}  // namespace line_breaking
}  // namespace options_parser
#endif  // FILE_C8B83754_2BB1_4F59_9DE9_E4E510196300_H
