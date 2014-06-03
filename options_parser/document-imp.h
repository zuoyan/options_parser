#ifndef FILE_B03D9770_5FD4_4607_97CB_E4EA7D3486DB_H
#define FILE_B03D9770_5FD4_4607_97CB_E4EA7D3486DB_H
#include "options_parser/config.h"
#include "options_parser/converter-imp.h"
namespace options_parser {

OPTIONS_PARSER_IMP Document::Document() {}

OPTIONS_PARSER_IMP std::vector<string> Document::format(size_t width) const {
  if (format_) {
    return format_(width);
  }
  return hang(18, " ", "", left_, right_)(width);
}

OPTIONS_PARSER_IMP void Document::append(const Formatter &more) {
  if (format_) {
    format_ = vcat(format_, indent(18, more));
  } else {
    right_ = vcat(right_, more);
  }
}

OPTIONS_PARSER_IMP void Document::append_message(const Formatter &more) {
  format_ = vcat(format_, indent(18, more));
}

}  // namespace options_parser
#endif  // FILE_B03D9770_5FD4_4607_97CB_E4EA7D3486DB_H
