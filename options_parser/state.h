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

  template <class F, class FR = decltype(get_value(options_parser::apply(
                         std::declval<F>(), get_value(std::declval<V>())))(
                                             std::declval<S>()).first)>
  typename Rebind::template rebind<FR, S>::type bind(const F &func) const {
    auto tf = func_;
    auto lf = [ func, tf ](const S &s) -> std::pair<FR, S> {
      auto v_s = tf(s);
      if (is_error(v_s.first)) {
        return std::make_pair(error_message(get_error(v_s.first)), s);
      }
      auto fr = func(get_value(v_s.first));
      if (is_error(fr)) {
        return std::make_pair(error_message(get_error(fr)), s);
      }
      auto b_s = get_value(fr)(v_s.second);
      if (is_error(b_s.first)) {
        return std::make_pair(b_s.first, s);
      }
      return b_s;
    };
    return lf;
  }

  template <class F>
  auto apply(const F &func) const
      -> typename Rebind::template rebind<
            decltype(options_parser::apply(func, std::declval<V>())), S>::type {
    auto tf = func_;
    return [func, tf](const S &s) {
      auto v_s = tf(s);
      auto r = options_parser::apply(func, v_s.first);
      if (is_error(r)) {
        return std::make_pair(r, s);
      }
      return std::make_pair(r, v_s.second);
    };
  }

  template <class F, class = typename std::enable_if<
                         mpl::is_callable<F, V>::value>::type>
  typename Rebind::template rebind<Maybe<V>, S>::type check(const F &func) {
    return apply([func](const V &v)
                     -> Maybe<V> { return func(v) ? v : nothing; });
  }

  template <class F, class = typename std::enable_if<
                         !mpl::is_callable<F, V>::value>::type>
  typename Rebind::template rebind<V, S>::type check(
      const F &func, const string &message = "check failed") {
    return apply([func, message](const V &v) -> V {
      if (is_error(v)) {
        return v;
      }
      if (func(get_value(v))) {
        return v;
      }
      return error_message(message);
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
