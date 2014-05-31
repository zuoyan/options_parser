#ifndef FILE_266AC82D_20F2_4BAA_9935_1880DD37A105_H
#define FILE_266AC82D_20F2_4BAA_9935_1880DD37A105_H
#include "options_parser/config.h"
#include "options_parser/matcher-imp.h"
#include "options_parser/converter-imp.h"

namespace options_parser {

OPTIONS_PARSER_IMP TakeResult Taker::operator()(const MatchResult &mr) const {
  return take_(mr);
}

OPTIONS_PARSER_IMP Taker::operator bool() const { return bool(take_); }

OPTIONS_PARSER_IMP Maybe<string> Taker::to_error(Nothing) { return nothing; }

OPTIONS_PARSER_IMP Maybe<string> Taker::to_error(void_) { return nothing; }

OPTIONS_PARSER_IMP Maybe<string> Taker::to_error(int c) {
  if (c > 0) return nothing;
  return "error code: " + std::to_string(c);
}

OPTIONS_PARSER_IMP Maybe<string> Taker::to_error(bool f) {
  if (f) return nothing;
  return string("failed");
}

OPTIONS_PARSER_IMP Maybe<string> Taker::to_error(const string &e) {
  if (!e.size()) return nothing;
  return e;
}

}  // namespace options_parser
#endif  // FILE_266AC82D_20F2_4BAA_9935_1880DD37A105_H
