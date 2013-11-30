#ifndef FILE_2298A146_867B_44A9_B158_53A19B060797_H
#define FILE_2298A146_867B_44A9_B158_53A19B060797_H
#include "options_parser/maybe.h"
#include "options_parser/arguments.h"
#include "options_parser/state.h"
#include "options_parser/converter.h"

namespace options_parser {

struct Position {
  int index;
  int off;
};

template <class T>
struct PositionValue {
  Maybe<T> value;
  Position start;
  Position end;

  template <class U>
  PositionValue(const PositionValue<U> &o) {
    if (o.value) {
      value = o.value;
    }
  }

  PositionValue(const Maybe<T> &v, const Position &s, const Position &e)
      : value(v), start(s), end(e) {}

  template <class Func>
  auto bind(Func &&func) const
      OPTIONS_PARSER_AUTO_RETURN(PositionValue<decltype(func(*(const T *)0))>(
          value.bind(func), start, end));
};

PositionValue<char> get_char(const Arguments *args, const Position &pos) {
  int c = args->char_at(pos.index, pos.off);
  if (c != -1) {
    Position end{pos.index, pos.off + 1};
    return PositionValue<char>((char)c, pos, end);
  }
  return PositionValue<char>(nothing, pos, pos);
}

PositionValue<std::string> get_arg(const Arguments *args, const Position &pos) {
  if (pos.index >= args->argc()) {
    return PositionValue<std::string>(nothing, pos, pos);
  }
  auto s = args->arg_at(pos.index);
  if (s.size() >= (size_t)pos.off)
    s = s.substr(pos.off);
  else
    s = "";
  Position end{pos.index + 1, 0};
  return PositionValue<std::string>(s, pos, end);
}

PositionValue<std::string> get_match_arg(const Arguments *args,
                                         const Position &pos,
                                         const char prefix = '-') {
  Position arg_pos{pos.index, 0};
  auto r = get_arg(args, arg_pos);
  if (!r.value || !r.value.get()->size() || r.value.get()->at(0) != prefix) {
    return PositionValue<std::string>(nothing, pos, pos);
  }
  if ((int)r.value.get()->size() <= pos.off) {
    return PositionValue<std::string>(nothing, pos, pos);
  }
  int off = pos.off;
  if (off == 0) {
    std::string *p = r.value.mutable_get();
    while (off < (int)p->size() && p->at(off) == prefix) off++;
    *p = p->substr(off);
  } else {
    r.start = pos;
    std::string *p = r.value.mutable_get();
    *p = p->substr(off);
  }
  auto eq = r.value.get()->find('=');
  if (eq < r.value.get()->size()) {
    r.end = pos;
    r.end.off = off + eq + 1;
    *r.value.mutable_get() = r.value.mutable_get()->substr(0, eq);
  }
  return r;
}

struct PositionArguments {
  Position position;
  Arguments *args;
};

template <class T = std::string, class Check = always_true>
state<Either<T>, PositionArguments> value(Check check = Check()) {
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

template <class Check = always_true>
state<Either<std::string>, PositionArguments> match_value(Check check =
                                                              Check()) {
  auto func = [check](const PositionArguments & s)
      ->std::pair<Either<std::string>, PositionArguments> {
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

template <class T, int... I>
auto tuple_value_indices(mpl::vector_c<I...>)
    OPTIONS_PARSER_AUTO_RETURN(gather<PositionArguments>(
        value<typename std::tuple_element<I, T>::type>()...));

template <class T>
auto tuple_value() OPTIONS_PARSER_AUTO_RETURN(tuple_value_indices<T>(
    typename mpl::vector_range<std::tuple_size<T>::value>::type{}));

}  // namespace options_parser
#endif  // FILE_2298A146_867B_44A9_B158_53A19B060797_H
