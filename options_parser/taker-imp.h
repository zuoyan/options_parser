#ifndef FILE_266AC82D_20F2_4BAA_9935_1880DD37A105_H
#define FILE_266AC82D_20F2_4BAA_9935_1880DD37A105_H
#include "options_parser/matcher-imp.h"
#include "options_parser/converter-imp.h"

namespace options_parser {

inline TakeResult Taker::operator()(const MatchResult &mr) const {
  return take_(mr);
}

inline Maybe<string> Taker::to_error(Nothing) { return nothing; }

inline Maybe<string> Taker::to_error(void_) { return nothing; }

inline Maybe<string> Taker::to_error(int c) {
  if (c > 0) return nothing;
  return "error code: " + std::to_string(c);
}

inline Maybe<string> Taker::to_error(bool f) {
  if (f) return nothing;
  return string("failed");
}

inline Maybe<string> Taker::to_error(const string &e) {
  if (!e.size()) return nothing;
  return e;
}

}  // namespace options_parser
#endif  // FILE_266AC82D_20F2_4BAA_9935_1880DD37A105_H
