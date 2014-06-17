#ifndef FILE_FDF3ED4F_018D_4887_A1C6_305E80B2932A_H
#define FILE_FDF3ED4F_018D_4887_A1C6_305E80B2932A_H
#include <functional>
#include <memory>
#include <type_traits>

#include "options_parser/maybe.h"

namespace options_parser {

template <class T>
class property {
  std::function<T(void)> func_;
  std::shared_ptr<T> value_;
 public:
  property() = default;

  template <class U, typename std::enable_if<std::is_constructible<T, U>::value,
                                             int>::type = 0>
  property(const U &v)
      : value_(new T(v)) {}

  template <class F,
            typename std::enable_if<
                std::is_convertible<typename mpl::is_callable<F>::result_type,
                                    T>::value,
                int>::type = 0>
  property(const F &func)
      : func_(func) {}

  property(const property &) = default;

  bool empty() const { return !value_ && !func_; }

  operator T() const {
    assert(!empty());
    if (func_) return func_();
    return *value_.get();
  }

  template <class U>
  property &operator=(const U &u) {
    if ((intptr_t)&u != (intptr_t) this) {
      this->~property();
      new (this) property(u);
    }
    return *this;
  }

  template <class U>
  property &operator=(const Maybe<U> &v) {
    this->~property();
    if (v) {
      new (this) property(*v.get());
    } else {
      new (this) property();
    }
    return *this;
  }

  friend property operator+(const property &l, const property &r) {
    if (r.empty()) return l;
    if (l.empty()) return r;
    return [l, r]() { return (T)l + (T)r; };
  }
};

}  // namespace options_parser
#endif  // FILE_FDF3ED4F_018D_4887_A1C6_305E80B2932A_H
