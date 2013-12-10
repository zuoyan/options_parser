#ifndef FILE_27FFDE52_CEEE_457F_AF7D_EBBE7B5A6497_H
#define FILE_27FFDE52_CEEE_457F_AF7D_EBBE7B5A6497_H
#include "options_parser/converter-inc.h"
namespace options_parser {

template <class P, class D>
Document::Document(const P &prefix, const D &description) {
  auto p = indent(1, Formatter(prefix));
  auto d = Formatter(description);
  format_ = hang(18, " ", "", p, d);
}

template <class D>
Document::Document(const D &description) {
  new (this) Document("", description);
}

template <class D>
void Document::set_message(const D &d) {
  format_ = Formatter(d);
}

template <class T>
property<string> delay_to_str(const T *ptr) {
  return [ptr]() { return to_str(*ptr); };
}

}  // namespace options_parser
#endif  // FILE_27FFDE52_CEEE_457F_AF7D_EBBE7B5A6497_H
