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
state<Either<T>, PositionArguments> value(Check check) {
  auto func = [check](const PositionArguments & s)
      ->std::pair<Either<T>, PositionArguments> {
    if (!check(s)) {
      return std::make_pair(error_message("pre check failed"), s);
    }
    auto r = get_arg(s.args, s.position);
    PositionArguments pa{r.end, s.args};
    if (!r.value) {
      return std::make_pair(error_message("no arguments rest"), pa);
    }
    T val;
    auto ec = from_str(*r.value.get(), &val);
    if (ec) {
      return std::make_pair(error_message(*ec.get()), pa);
    }
    return std::make_pair(val, pa);
  };
  return func;
}

template <class Check>
state<Either<string>, PositionArguments> match_value(Check check) {
  auto func = [check](const PositionArguments & s)
      ->std::pair<Either<string>, PositionArguments> {
    if (!check(s)) {
      return std::make_pair(nothing, s);
    }
    auto r = get_match_arg(s.args, s.position);
    PositionArguments pa{r.end, s.args};
    if (r.value) {
      return std::make_pair(*r.value.get(), pa);
    }
    return std::make_pair(error_message("no arguments rest"), pa);
  };
  return func;
};

}  // namespace options_parser
#endif  // FILE_FD129630_4D45_4D18_A7A8_D51D3445F5A4_H
