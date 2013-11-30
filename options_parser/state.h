#ifndef FILE_B3FD743E_F3A0_4334_A8A5_1711E96AB8C6_H
#define FILE_B3FD743E_F3A0_4334_A8A5_1711E96AB8C6_H
#include <functional>
#include <tuple>

namespace options_parser {

template <class V, class S>
struct state;

template <class T>
struct is_state_impl : std::false_type {};

template <class V, class S>
struct is_state_impl<state<V, S>> : std::true_type {};

template <class T>
struct is_state : is_state_impl<typename std::remove_reference<
                      typename std::remove_const<T>::type>::type> {};

template <class V, class S>
struct state {
  static_assert(!is_state<V>::value, "...");
  static_assert(!is_state<S>::value, "...");

  state() = default;
  state(const state&) = default;
  state(state&&) = default;

  state &operator=(const state &) = default;
  state &operator=(state &) = default;
  state &operator=(state &&) = default;

  template <class F,
            typename std::enable_if<
                std::is_convertible<
                    decltype(std::declval<F>()(std::declval<S>())), V>::value,
                int>::type = 0>
  state(const F &func) {
    static_assert(!is_state<F>::value, "...");
    func_ = func;
  }

  template <class U, typename std::enable_if<std::is_convertible<U, V>::value,
                                             int>::type = 0>
  state(const state<U, S> &o)
      : func_(o.func_) {}

  std::pair<V, S> operator()(const S &s) const { return func_(s); }

  std::function<std::pair<V, S>(const S &)> func_;

  template <class T>
  static state<T, S> wrap(const T &v) {
    return [v](const S &s) { return std::make_pair(v, s); };
  }

  template <class F, typename std::enable_if<mpl::is_callable<F, V>::value,
                                             int>::type = 0>
  state<typename mpl::is_callable<typename mpl::is_callable<F, V>::result_type,
                                  S>::result_type::first_type,
        S>
  bind(const F &func) const {
    auto tf = func_;
    return [func, tf](const S &s) {
      auto v_s = tf(s);
      return func(v_s.first)(v_s.second);
    };
  }

  template <class F, typename std::enable_if<mpl::is_callable<F, V, S>::value,
                                             int>::type = 0>
  state<typename mpl::is_callable<F, V, S>::result_type::first_type, S> bind(
      const F &func) const {
    auto tf = func_;
    return [func, tf](const S &s) {
      auto v_s = tf(s);
      return func(v_s.first, v_s.second);
    };
  }

  template <class F>
  auto apply(const F &func) const
      -> state<decltype(options_parser::apply(func, std::declval<V>())), S> {
    auto tf = func_;
    return [func, tf](const S &s) {
      auto v_s = tf(s);
      return std::make_pair(options_parser::apply(func, v_s.first), v_s.second);
    };
  }

  template <class F>
  state<Maybe<V>, S> check(const F &func) {
    auto tf = func_;
    return bind([ func, tf ](const S & s)->std::pair<Maybe<V>, S> {
      auto v_s = tf(s);
      if (func(v_s.first)) return std::make_pair(v_s.first, v_s.second);
      return std::make_pair(nothing, s);
    });
  }
};

template <class S>
struct state_value_type;

template <class V, class S>
struct state_value_type<state<V, S>> {
  typedef V type;
};

template <class S>
struct state_state_type;

template <class V, class S>
struct state_state_type<state<V, S>> {
  typedef S type;
};

template <class... States>
struct gather_value_type {
  typedef std::tuple<decltype(get_value(
      std::declval<typename state_value_type<States>::type>()))...> type;
};

template <class S>
struct gather_tuple_impl {
  template <int I, int N, class States, class Values,
            typename std::enable_if<(I == N), int>::type = 0>
  static S gather_tuple_h(const States &states, Values &values, const S &s,
                          Maybe<std::string> &error) {
    return s;
  }

  template <int I, int N, class States, class Values,
            typename std::enable_if<(I < N), int>::type = 0>
  static S gather_tuple_h(const States &states, Values &values, const S &s,
                          Maybe<std::string> &error) {
    auto v_s = std::get<I>(states)(s);
    error = get_error(v_s.first);
    if (error) return v_s.second;
    std::get<I>(values) = get_value(v_s.first);
    return gather_tuple_h<I + 1, N>(states, values, v_s.second, error);
  }

    template <class... States>
    static state<Either<typename gather_value_type<States...>::type>, S>
    gather_tuple(const std::tuple<States...> &states) {
      // return state<Either<typename gather_value_type<States...>::type>, S>{};
      auto func = [states](const S & s)
          ->std::pair<Either<typename gather_value_type<States...>::type>, S> {
        typename gather_value_type<States...>::type value;
        gather_tuple_impl self;
        Maybe<std::string> error;
        auto ns =
        self.gather_tuple_h<0, sizeof...(States)>(states, value, s, error);
        if (error) {
          return std::make_pair(error_message(*error.get()), s);
        }
        return std::make_pair(value, ns);
      };
      return func;
    }
};

template <class S, class... States>
auto gather_tuple(const std::tuple<States...> &states)
    OPTIONS_PARSER_AUTO_RETURN(gather_tuple_impl<S>::gather_tuple(states));

template <class H, class... States>
auto gather_tuple(const std::tuple<H, States...> &states)
    OPTIONS_PARSER_AUTO_RETURN(
        gather_tuple<typename state_state_type<H>::type>(states));

template <class S, class... States>
auto gather(const States &... states)
    OPTIONS_PARSER_AUTO_RETURN(gather_tuple<S>(std::make_tuple(states...)));

template <class H, class... States>
auto gather(const H &h, const States &... states) OPTIONS_PARSER_AUTO_RETURN(
    gather<typename state_state_type<H>::type>(h, states...));

template <class V, class S>
state<Either<std::vector<decltype(get_value(std::declval<V>()))>>, S> many(
    const state<V, S> &one, size_t min = 0, size_t max = -1) {
  auto func =
      [ one, min, max ](const S & s)
                           ->std::pair<Either<std::vector<decltype(
                                           get_value(std::declval<V>()))>>,
                                       S> {
    std::vector<decltype(get_value(std::declval<V>()))> vs;
    auto saved = s;
    auto ns = s;
    Maybe<std::string> error;
    while (vs.size() < max) {
      auto v_s = one(ns);
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

template <class V, class S>
auto times(const state<V, S> &one, size_t n)
    OPTIONS_PARSER_AUTO_RETURN(many(one, n, n));

}  // namespace options_parser
#endif  // FILE_B3FD743E_F3A0_4334_A8A5_1711E96AB8C6_H
