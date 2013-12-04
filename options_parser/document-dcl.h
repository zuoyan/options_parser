#ifndef FILE_29B677D6_8334_48D6_A77C_1B719C1198AD_H
#define FILE_29B677D6_8334_48D6_A77C_1B719C1198AD_H
#include <string>
#include <vector>

#include "options_parser/maybe.h"
#include "options_parser/property.h"
#include "options_parser/string.h"
#include "options_parser/converter-dcl.h"

namespace options_parser {

struct Document {
  Document();
  Document(const Document &) = default;
  template <class P, class D>
  Document(const P &prefix, const D &description);
  template <class D>
  Document(const D &description);

  string prefix() const;
  string description() const;
  template <class P>
  Document &set_prefix(const P &p);
  template <class D>
  Document &set_description(const D &d);
  template <class D>
  Document &set_message(const D &d);

  std::vector<string> format(size_t width) const;

  property<string> prefix_;
  property<string> description_;
  // if message_ is true, we will format this Document without prefix
  bool message_;
};

template <class T>
property<string> delay_to_str(const T *ptr);

}  // namespace options_parser
#endif  // FILE_29B677D6_8334_48D6_A77C_1B719C1198AD_H
