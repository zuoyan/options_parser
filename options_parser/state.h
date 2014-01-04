#ifndef FILE_B3FD743E_F3A0_4334_A8A5_1711E96AB8C6_H
#define FILE_B3FD743E_F3A0_4334_A8A5_1711E96AB8C6_H
#include <functional>
#include <tuple>

namespace options_parser {

template <class V, class S, class Rebind>
struct state;

template <class T>
struct is_state_impl : std::false_type {};

template <class V, class S, class Rebind>
struct is_state_impl<state<V, S, Rebind>> : std::true_type {};

template <class T>
struct is_state : is_state_impl<typename std::remove_reference<
                      typename std::remove_const<T>::type>::type> {};

template <class V, class S, class Rebind>
struct state {
  typedef V value_type;
  typedef S state_type;

  state() = default;
  state(const state &) = default;
  // state(state&&) = default;

  state &operator=(const state &) = default;
  // state &operator=(state &) = default;
  // state &operator=(state &&) = default;

  template <
      class F,
      typename std::enable_if<
          std::is_convertible<
              decltype(std::declval<F>()(std::declval<S>()).first), V>::value,
          int>::type = 0>
  state(const F &func) {
    static_assert(!is_state<F>::value, "...");
    func_ = func;
  }

  template <
      class U, class R,
      typename std::enable_if<std::is_convertible<U, V>::value, int>::type = 0>
  state(const state<U, S, R> &o)
      : func_(o.func_) {}

  std::pair<V, S> operator()(const S &s) const { return func_(s); }

  std::function<std::pair<V, S>(const S &)> func_;

  template <class T>
  static typename Rebind::template rebind<T, S>::type wrap(const T &v) {
    return [v](const S &s) { return std::make_pair(v, s); };
  }

  template <class F>
  typename Rebind::template rebind<
      typename mpl::is_callable<typename mpl::is_callable<F, V>::result_type,
                                S>::result_type::first_type,
      S>::type
  bind(const F &func) const {
    auto tf = func_;
    return [func, tf](const S &s) {
      auto v_s = tf(s);
      return func(v_s.first)(v_s.second);
    };
  }

  template <class F, typename std::enable_if<mpl::is_callable<F, V, S>::value,
                                             int>::type = 0>
  typename Rebind::template rebind<
      typename mpl::is_callable<F, V, S>::result_type::first_type, S>::type
  bind(const F &func) const {
    auto tf = func_;
    return [func, tf](const S &s) {
      auto v_s = tf(s);
      return func(v_s.first, v_s.second);
    };
  }

  template <class F>
  auto apply(const F &func) const
      -> typename Rebind::template rebind<
            decltype(options_parser::apply(func, std::declval<V>())), S>::type {
    auto tf = func_;
    return [func, tf](const S &s) {
      auto v_s = tf(s);
      return std::make_pair(options_parser::apply(func, v_s.first), v_s.second);
    };
  }

  template <class F>
  typename Rebind::template rebind<Maybe<V>, S>::type check(
      const F &func) {
    auto tf = func_;
    return bind([ func, tf ](const S & s)->std::pair<Maybe<V>, S> {
      auto v_s = tf(s);
      if (options_parser::apply(func, v_s.first)) {
        return std::make_pair(v_s.first, v_s.second);
      }
      return std::make_pair(nothing, s);
    });
  }

  typename Rebind::template rebind<V, S>::type peek() const {
    auto tf = func_;
    return [tf](const S &s) {
      auto r = tf(s);
      r.second = s;
      return r;
    };
  }
};

}  // namespace options_parser
#endif  // FILE_B3FD743E_F3A0_4334_A8A5_1711E96AB8C6_H
