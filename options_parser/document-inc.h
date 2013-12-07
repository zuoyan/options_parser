#ifndef FILE_27FFDE52_CEEE_457F_AF7D_EBBE7B5A6497_H
#define FILE_27FFDE52_CEEE_457F_AF7D_EBBE7B5A6497_H
#include "options_parser/converter-inc.h"
namespace options_parser {

template <class P, class D>
Document::Document(const P &prefix, const D &description) {
  message_ = false;
  prefix_ = as_formatter(prefix);
  description_ = as_formatter(description);
}

template <class D>
Document::Document(const D &description) {
  message_ = false;
  description_ = as_formatter(description);
}

template <class D>
void Document::set_message(const D &d) {
  message_ = true;
  description_ = as_formatter(d);
}

template <class T>
property<string> delay_to_str(const T *ptr) {
  return [ptr]() { return to_str(*ptr); };
}

}  // namespace options_parser
#endif  // FILE_27FFDE52_CEEE_457F_AF7D_EBBE7B5A6497_H
