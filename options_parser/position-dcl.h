#ifndef FILE_F2D8CD3C_A47C_47F4_B735_63DC6D4098BA_H
#define FILE_F2D8CD3C_A47C_47F4_B735_63DC6D4098BA_H
#include "options_parser/maybe.h"
#include "options_parser/state.h"
#include "options_parser/arguments-dcl.h"
#include "options_parser/converter-dcl.h"
#include "options_parser/circumstance.h"

namespace options_parser {

struct Position {
  Position(int index, int off) : index(index), off(off) {}
  Position() : index(0), off(0) {}

  int index;
  int off;
};

template <class T>
struct PositionValue {
  Maybe<T> value;
  Position start;
  Position end;

  template <class U>
  PositionValue(const PositionValue<U> &o);

  PositionValue(const Maybe<T> &v, const Position &s, const Position &e);

  template <class Func>
  auto bind(Func &&func) const
      OPTIONS_PARSER_AUTO_RETURN(PositionValue<decltype(func(*(const T *)0))>(
          value.bind(func), start, end));
};

inline PositionValue<char> get_char(const Arguments &args,
                                    const Position &pos);

inline PositionValue<string> get_arg(const Arguments &args,
                                     const Position &pos);

inline PositionValue<string> get_match_arg(const Arguments &args,
                                           const Position &pos,
                                           const char prefix, bool strip);

struct Situation {
  Position position;
  Arguments args;
  Circumstance circumstance;
};

template <class T = string, class Check = always_true>
state<Either<T>, Situation> value(Check check = Check());

template <class Check = always_true>
state<Either<string>, Situation> match_value(Check check = Check(),
                                             char prefix = '-',
                                             bool strip = false);

template <class Check = always_true>
state<Either<Maybe<string>>, Situation> optional_value(Check check = Check()) {
  auto func = [check](Situation s) {
    Either<Maybe<string>> v;
    if (s.position.off > 0 && check(s)) {
      auto v_s = value()(s);
      s = v_s.second;
      if (v_s.first.value) {
        v.value = v_s.first.value;
      }
    }
    if (!v.value) {
      v.value = Maybe<string>{};
    }
    return std::make_pair(v, s);
  };
  return func;
}

template <class T, int... I>
auto tuple_value_indices(mpl::vector_c<I...>) OPTIONS_PARSER_AUTO_RETURN(
    gather<Situation>(value<typename std::tuple_element<I, T>::type>()...));

template <class T>
auto tuple_value() OPTIONS_PARSER_AUTO_RETURN(tuple_value_indices<T>(
    typename mpl::vector_range<std::tuple_size<T>::value>::type{}));

}  // namespace options_parser
#endif  // FILE_F2D8CD3C_A47C_47F4_B735_63DC6D4098BA_H
