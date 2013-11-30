#ifndef FILE_024ED828_8C97_4719_8FDB_B71DEB72390D_H
#define FILE_024ED828_8C97_4719_8FDB_B71DEB72390D_H
#include <type_traits>
#include <cassert>

#include "options_parser/cow.h"
#include "options_parser/mpl.h"

#define OPTIONS_PARSER_AUTO_RETURN(...) \
  ->decltype(__VA_ARGS__) { return __VA_ARGS__; }

namespace options_parser {

struct not_used {};

template <class ...A, class T=not_used>
void show_type(T _=T()) {
  static_assert(mpl::amount(std::is_same<A, T>::value...), "...");
}

struct Nothing {};
constexpr Nothing nothing{};

struct void_ {
  void_() = default;

  void_(const void_ &) = default;

  void_(Nothing) {}

  operator Nothing() const { return nothing; }
};

template <class F>
struct VoidWrap {
  F func;

  template <class... Args>
  struct result_type_impl {
    typedef decltype(std::declval<F>()(std::declval<Args>()...)) origin_type;
    static constexpr bool origin_void = std::is_void<origin_type>::value;
    typedef typename std::conditional<origin_void, void_, origin_type>::type
        type;
  };

  template <class... Args>
  typename std::enable_if<result_type_impl<Args &&...>::origin_void,
                          typename result_type_impl<Args &&...>::type>::type
  call(Args &&... args) const {
    func(std::forward<Args>(args)...);
    return void_{};
  }

  template <class... Args>
  typename std::enable_if<!result_type_impl<Args &&...>::origin_void,
                          typename result_type_impl<Args &&...>::type>::type
  call(Args &&... args) const {
    return func(std::forward<Args>(args)...);
  }

  template <class... Args>
  typename result_type_impl<Args &&...>::type operator()(Args &&... args)
      const {
    return call(std::forward<Args>(args)...);
  }
};

template <class F>
VoidWrap<F> void_wrap(const F &f) {
  return VoidWrap<F>{f};
}

template <class F, class V, class Tag = void>
struct apply_impl;

template <class F, class V>
struct apply_impl<
    F, V, typename std::enable_if<mpl::is_callable<F, V>::value>::type> {
  static auto apply(const F &f, const V &v) OPTIONS_PARSER_AUTO_RETURN(f(v));
};

template <class F, class V>
struct apply_impl<
    F, V, typename std::enable_if<!mpl::is_callable<F, V>::value>::type> {
  static auto apply(const F &f, const V &v)
      OPTIONS_PARSER_AUTO_RETURN(v.apply(f));
};

template <class F, class V>
auto apply(const F &f, const V &v)
    OPTIONS_PARSER_AUTO_RETURN(apply_impl<F, V>::apply(f, v));

template <class T>
struct Maybe;

struct always_true {
  template <class... U>
  constexpr bool operator()(U &&... u) const {
    return true;
  }
};

template <class T>
Maybe<T> maybe(const T &v) {
  return Maybe<T>(v);
}

template <class T>
struct Maybe {
  cow<T> value;

  explicit operator bool() const { return value.get(); }

  template <class U>
  Maybe(const U &value)
      : value(value) {}

  Maybe(Nothing) {}

  Maybe() {}

  Maybe(const Maybe<T> &o) : value(o.value) {}

  template <class U>
  Maybe &operator=(const U &v) {
    if ((intptr_t) & v != (intptr_t) this) {
      this->~Maybe();
      new (this) Maybe(v);
    }
    return *this;
  }

  const T *get() const { return value.get(); }

  T *mutable_get() { return value.mutable_get(); }

  explicit operator T() const {
    auto ptr = get();
    assert(ptr);
    return *ptr;
  }

  template <class Func>
  auto bind(Func &&func) const
      OPTIONS_PARSER_AUTO_RETURN(value ? maybe(func(*value.get())) : nothing);

  template <class Func>
  auto apply(Func &&func) const
      OPTIONS_PARSER_AUTO_RETURN(value ? maybe(void_wrap(func)(*value.get()))
                                       : nothing);

  template <class U>
  static Maybe<U> wrap(const U &v) {
    return Maybe<U>(v);
  }
};

template <class T = std::string>
struct Error {
  T message;

  Error() = default;

  Error(const Error &) = default;

  Error(const T &m) : message(m) {}

  operator T() const { return message; }
};

Error<> error_message(const std::string &m) { return Error<>(m); }

template <class Value, class Other = std::string>
struct Either {
  Maybe<Value> value;
  Maybe<Other> other;

  Either() = default;

  Either(const Either &) = default;

  Either(const Error<Other> &e) { other = e.message; }

  template <class U>
  Either(const U &v)
      : value(v) {}

  template <class Func>
  auto bind(Func &&func)
      const -> Either<decltype(func(std::declval<Value>())), Other> {
    if (other) return Error<Other>(*other.get());
    if (value) return func(*this->value.get());
    return Error<Other>();
  }

  template <class Func>
  auto apply(Func &&func)
      const -> Either<decltype(void_wrap(func)(std::declval<Value>())), Other> {
    if (other) return Error<Other>(*other.get());
    if (value) return void_wrap(func)(*this->value.get());
    return Error<Other>();
  }

  template <class U>
  static Either<U> wrap(const U &v) {
    return Either<U>(v);
  }
};

template <class T>
T get_value(const T &v) {
  return v;
}

template <class T>
T get_value(const Maybe<T> &v) {
  assert(v);
  return *v.get();
}

template <class T>
T get_value(const Either<T> &ve) {
  assert(!ve.other);
  return *ve.value.get();
}

Nothing get_value(Nothing) { return nothing; }

template <class T>
Maybe<std::string> get_error(const T &v) {
  return nothing;
}

template <class T>
Maybe<std::string> get_error(const Maybe<T> &v) {
  if (v) return nothing;
  return std::string("empty");
}

template <class T>
Maybe<std::string> get_error(const Either<T> &ve) {
  return ve.other;
}

Maybe<std::string> get_error(Nothing) { return nothing; }

template <class F>
struct invoker {
  invoker(const F &f) : func(f) {}

  F func;

  template <class V>
  struct result_type_impl;

  template <class... Args>
  struct result_type_impl<std::tuple<Args...>> {
    typedef decltype(std::declval<F>()(std::declval<Args>()...)) type;
  };

  template <class V, int... I>
  typename result_type_impl<V>::type call_indices(
      const V &v, const mpl::vector_c<I...>) const {
    return this->func(std::get<I>(v)...);
  }

  template <class V>
  typename result_type_impl<V>::type operator()(const V &v) const {
    return this->call_indices(
        v, typename mpl::vector_range<std::tuple_size<V>::value>::type{});
  }
};

template <class F>
invoker<F> invoke(const F &f) {
  return invoker<F>(f);
}

template <class F>
struct check_invoker {
  check_invoker(const F &f) : func(f) {}

  F func;

  template <class V>
  struct result_type_impl;

  template <class... Args>
  struct result_type_impl<std::tuple<Args...>> {
    typedef decltype(
        void_wrap(std::declval<F>())(get_value(std::declval<Args>())...)) type;
  };

  Maybe<std::string> check_all() const { return nothing; }

  template <class H, class... R>
  Maybe<std::string> check_all(const H &h, const R &... r) const {
    auto e = get_error(h);
    if (e) return e;
    return check_all(r...);
  }

  template <class V, int... I>
  Maybe<std::string> check_indices(const V &v, mpl::vector_c<I...>) const {
    return check_all(std::get<I>(v)...);
  }

  template <class V, int... I>
  Either<typename result_type_impl<V>::type> call_indices(
      const V &v, mpl::vector_c<I...> indices) const {
    auto e = this->check_indices(v, indices);
    if (e) {
      return error_message(*e.get());
    }
    return void_wrap(this->func)(get_value(std::get<I>(v))...);
  }

  template <class V>
  Either<typename result_type_impl<V>::type> operator()(const V &v) const {
    return this->call_indices(
        v, typename mpl::vector_range<std::tuple_size<V>::value>::type{});
  }
};

template <class F>
check_invoker<F> check_invoke(const F &f) {
  return check_invoker<F>(f);
}

}  // namespace options_parser
#endif  // FILE_024ED828_8C97_4719_8FDB_B71DEB72390D_H
