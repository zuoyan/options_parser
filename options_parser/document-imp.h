#ifndef FILE_B03D9770_5FD4_4607_97CB_E4EA7D3486DB_H
#define FILE_B03D9770_5FD4_4607_97CB_E4EA7D3486DB_H
#include "options_parser/config.h"
#include "options_parser/converter-imp.h"
namespace options_parser {

OPTIONS_PARSER_IMP Document::Document() { message_ = false; }

OPTIONS_PARSER_IMP string Document::prefix() const {
  if (prefix_.empty()) return string();
  return prefix_;
}

OPTIONS_PARSER_IMP string Document::description() const {
  if (description_.empty()) return string();
  return description_;
}

OPTIONS_PARSER_IMP std::vector<string> Document::format(size_t width) const {
  std::vector<string> ret;
  if (message_) {
    if (description_.empty()) return ret;
    // return format_str((string)description_, width);
    return as_formatter((string)description_)(width);
  }
  string p, d;
  if (!prefix_.empty()) p = prefix_;
  if (!description_.empty()) d = description_;
  return formatter_hang(18, " ", "", as_formatter(p), as_formatter(d))(width);
}

}  // namespace options_parser
#endif  // FILE_B03D9770_5FD4_4607_97CB_E4EA7D3486DB_H
