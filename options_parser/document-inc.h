#ifndef FILE_27FFDE52_CEEE_457F_AF7D_EBBE7B5A6497_H
#define FILE_27FFDE52_CEEE_457F_AF7D_EBBE7B5A6497_H
#include "options_parser/converter-inc.h"
namespace options_parser {

template <class P, class D>
Document::Document(const P &prefix, const D &description) {
  message_ = false;
  set_prefix(prefix);
  set_description(description);
}

template <class D>
Document::Document(const D &description) {
  message_ = false;
  set_description(description);
}

template <class P>
Document &Document::set_prefix(const P &p) {
  prefix_ = p;
  return *this;
}

template <class D>
Document &Document::set_description(const D &d) {
  description_ = d;
  return *this;
}

template <class D>
Document &Document::set_message(const D &d) {
  message_ = true;
  return set_description(d);
}

template <class T>
property<string> delay_to_str(const T *ptr) {
  return [ptr]() { return to_str(*ptr); };
}

}  // namespace options_parser
#endif  // FILE_27FFDE52_CEEE_457F_AF7D_EBBE7B5A6497_H
