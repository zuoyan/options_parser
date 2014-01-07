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

inline PositionValue<char> get_char(const Arguments &args, const Position &pos);

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

template <class T>
struct Value;

struct ValueRebind {
  template <class T, class S>
  struct rebind {
    static_assert(std::is_same<S, Situation>::value, "must be Situation");
    typedef Value<decltype(get_value(std::declval<T>()))> type;
  };
};

template <class T>
struct Value : state<Either<T>, Situation, ValueRebind> {
  typedef state<Either<T>, Situation, ValueRebind> base;
  typedef T inner_value_type;

  template <class F,
            typename std::enable_if<
                std::is_convertible<decltype(std::declval<F>()(
                                        std::declval<Situation>()).first),
                                    Either<T>>::value,
                int>::type = 0>
  Value(const F &func)
      : base(func) {}

  Value(const Value&) = default;
  Value &operator=(const Value &) = default;

  Value() {
    this->func_ = [](Situation s)->std::pair<Either<T>, Situation> {
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
  }

  template <class Check>
  Value situation_check(const Check &func,
                        const string &message = "situation_check failed") const {
    auto tf = this->func_;
    return Value([ tf, func, message ](Situation s)
                                          ->std::pair<Either<T>, Situation> {
      if (func(s)) {
        return tf(s);
      }
      return std::make_pair(error_message(message), s);
    });
  }

  Value not_option() const {
    return situation_check([](Situation s) {
      if (s.position.off > 0) return true;
      auto v_s = Value<string>()(s);
      if (get_error(v_s.first)) return true;
      auto arg = get_value(v_s.first);
      return !arg.size() || arg[0] != '-';
    });
  }

  Value<Maybe<T>> optional() const {
    auto tf = this->func_;
    auto func = [tf](Situation s) {
      Either<Maybe<T>> v;
      if (s.position.off > 0) {
        auto v_s = tf(s);
        s = v_s.second;
        if (v_s.first.value) {
          v.value = v_s.first.value;
        }
      }
      if (!v.value) {
        v.value = Maybe<T>{};
      }
      return std::make_pair(v, s);
    };
    return func;
  }

  Value<std::vector<T>> many(size_t min = 0, size_t max = -1) const {
    auto tf = this->func_;
    auto func = [ tf, min, max ](const Situation & s)
                                    ->std::pair<Either<std::vector<T>>,
                                                Situation> {
      std::vector<T> vs;
      auto saved = s;
      auto ns = s;
      Maybe<string> error;
      while (vs.size() < max) {
        auto v_s = tf(ns);
        error = get_error(v_s.first);
        if (error) break;
        ns = v_s.second;
        vs.push_back(get_value(v_s.first));
      }
      if (vs.size() < min) {
        return std::make_pair(error_message(*error.get()), saved);
      }
      return std::make_pair(vs, ns);
    };
    return func;
  }

  Value<std::vector<T>> times(size_t n) const { return many(n, n); }
};

template <class T=string>
Value<T> value() {
  return Value<T>();
}

Value<string> match_value(char prefix = '-', bool strip = false);

template <class... States>
struct value_gather_inner_value_type {
  typedef std::tuple<typename States::inner_value_type...> type;
};

struct value_gather_tuple_impl {
  template <int I, int N, class States, class Values,
            typename std::enable_if<(I == N), int>::type = 0>
  static Situation value_gather_tuple_h(const States &states, Values &values,
                                        const Situation &s,
                                        Maybe<string> &error) {
    return s;
  }

  template <int I, int N, class States, class Values,
            typename std::enable_if<(I < N), int>::type = 0>
  static Situation value_gather_tuple_h(const States &states, Values &values,
                                        const Situation &s,
                                        Maybe<string> &error) {
    auto v_s = std::get<I>(states)(s);
    error = get_error(v_s.first);
    if (error) return v_s.second;
    std::get<I>(values) = get_value(v_s.first);
    return value_gather_tuple_h<I + 1, N>(states, values, v_s.second, error);
  }

  template <class... States>
  static Value<typename value_gather_inner_value_type<States...>::type>
  value_gather_tuple(const std::tuple<States...> &states) {
    auto func = [states](const Situation & s)
        ->std::pair<Either<typename value_gather_inner_value_type<States...>::type>,
                    Situation> {
      typename value_gather_inner_value_type<States...>::type value;
      value_gather_tuple_impl self;
      Maybe<string> error;
      auto ns = self.value_gather_tuple_h<0, sizeof...(States)>(states, value,
                                                                s, error);
      if (error) {
        return std::make_pair(error_message(*error.get()), s);
      }
      return std::make_pair(value, ns);
    };
    return func;
  }
};

template <class... State>
auto value_gather_tuple(const std::tuple<State...> &states)
    OPTIONS_PARSER_AUTO_RETURN(
        value_gather_tuple_impl::value_gather_tuple<State...>(states));

template <class... States>
auto value_gather(const States &... states) OPTIONS_PARSER_AUTO_RETURN(
    value_gather_tuple<States...>(std::make_tuple(states...)));

template <class T, int... I>
auto value_tuple_indices(mpl::vector_c<I...>) OPTIONS_PARSER_AUTO_RETURN(
    value_gather(value<typename std::tuple_element<I, T>::type>()...));

template <class T>
auto value_tuple() OPTIONS_PARSER_AUTO_RETURN(value_tuple_indices<T>(
    typename mpl::vector_range<std::tuple_size<T>::value>::type{}));

}  // namespace options_parser
#endif  // FILE_F2D8CD3C_A47C_47F4_B735_63DC6D4098BA_H
