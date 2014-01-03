#ifndef FILE_FD129630_4D45_4D18_A7A8_D51D3445F5A4_H
#define FILE_FD129630_4D45_4D18_A7A8_D51D3445F5A4_H
#include "options_parser/arguments-inc.h"
#include "options_parser/converter-inc.h"

namespace options_parser {

template <class T>
template <class U>
PositionValue<T>::PositionValue(const PositionValue<U> &o) {
  if (o.value) {
    value = o.value;
  }
  start = o.start;
  end = o.end;
}

template <class T>
PositionValue<T>::PositionValue(const Maybe<T> &v, const Position &s,
                                const Position &e)
    : value(v), start(s), end(e) {}

template <class T, class Check>
state<Either<T>, Situation> value(Check check) {
  auto func = [check](Situation s)->std::pair<Either<T>, Situation> {
    if (!check(s)) {
      return std::make_pair(error_message("pre check failed"), s);
    }
    auto r = get_arg(s.args, s.position);
    s.position = r.end;
    s.args = s.args;
    if (!r.value) {
      return std::make_pair(error_message("no arguments rest"), s);
    }
    T val;
    auto ec = from_str(*r.value.get(), &val);
    if (ec) {
      return std::make_pair(error_message(*ec.get()), s);
    }
    return std::make_pair(val, s);
  };
  return func;
}

template <class Check>
state<Either<string>, Situation> match_value(Check check, char prefix,
                                             bool strip) {
  auto func =
      [ check, prefix, strip ](Situation s)
                                  ->std::pair<Either<string>, Situation> {
    if (!check(s)) {
      return std::make_pair(nothing, s);
    }
    auto r = get_match_arg(s.args, s.position, prefix, strip);
    s.position = r.end;
    if (r.value) {
      return std::make_pair(*r.value.get(), s);
    }
    return std::make_pair(error_message("no arguments rest"), s);
  };
  return func;
};

}  // namespace options_parser
#endif  // FILE_FD129630_4D45_4D18_A7A8_D51D3445F5A4_H
