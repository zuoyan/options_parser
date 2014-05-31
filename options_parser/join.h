#ifndef FILE_B7B57366_1176_41A3_9BA1_FD9EA1E4A24A_H
#define FILE_B7B57366_1176_41A3_9BA1_FD9EA1E4A24A_H
#include "options_parser/string.h"
#include "options_parser/converter-dcl.h"
#include "options_parser/converter-inc.h"

namespace options_parser {

template <class InputIt, class Convert = to_str_functor>
string join(InputIt first, InputIt last, const string &sep,
            Convert convert = Convert{}) {
  string ret;
  size_t i = 0;
  for (auto it = first; it != last; ++it, ++i) {
    if (i > 0) {
      ret += sep;
    }
    ret += convert(*it);
  }
  return ret;
}

template <class InputRange, class Convert = to_str_functor>
string join(const InputRange &range, const string &sep,
            Convert convert = Convert{}) {
  return join(range.begin(), range.end(), sep, convert);
}

}  // namespace options_parser
#endif // FILE_B7B57366_1176_41A3_9BA1_FD9EA1E4A24A_H
