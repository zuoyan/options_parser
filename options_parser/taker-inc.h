#ifndef FILE_AEEDB153_B259_46B0_B7FE_44CC5E6C29E1_H
#define FILE_AEEDB153_B259_46B0_B7FE_44CC5E6C29E1_H
#include "options_parser/matcher-inc.h"
#include "options_parser/converter-inc.h"

namespace options_parser {
template <class T>
inline Maybe<string> Taker::to_error(const Maybe<T> &v) {
  if (!v) return string("empty result");
  return to_error(*v.get());
}

template <class T>
inline Maybe<string> Taker::to_error(const std::vector<T> &vs) {
  for (const auto & v : vs) {
    auto e = to_error(v);
    if (e) return e;
  }
  return nothing;
}

template <class T>
inline Maybe<string> Taker::to_error(const Either<T> &ve) {
  auto e = get_error(ve);
  if (e) return e;
  return to_error(get_value(ve));
}

}  // namespace options_parser
#endif  // FILE_AEEDB153_B259_46B0_B7FE_44CC5E6C29E1_H
