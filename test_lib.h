#ifndef FILE_A93FFB28_73C5_446E_8B92_F57759FA7E26_H
#define FILE_A93FFB28_73C5_446E_8B92_F57759FA7E26_H
#include <vector>
#include <string>
#include <ostream>
#include <utility>

namespace test {

template <class T>
struct Container {
  typedef T value_type;

  std::vector<T> values;

  template <class... H>
  Container(H&&... h)
      : values(std::forward<H>(h)...) {}

  Container(const std::initializer_list<T>& vs) : values(vs) {}
  Container(const Container&) = default;
  Container(Container&&) = default;
  Container(Container&) = default;

  Container& operator=(const Container&) = default;

  template <class R>
  friend bool operator==(const Container& l, const R& r) {
    size_t i = 0;
    for (const auto& v : r) {
      if (i >= l.values.size()) return false;
      if (l.values[i] != v) return false;
      ++i;
    }
    if (i != l.values.size()) return false;
    return true;
  }

  template <class L>
  friend bool operator==(const L& l, const Container& r) {
    return r == l;
  }

  friend bool operator==(const Container& l, const Container& r) {
    return l == r.values;
  }

  friend std::ostream& operator<<(std::ostream& os, const Container& vs) {
    os << "[";
    for (size_t i = 0; i < vs.values.size(); ++i) {
      if (i > 0) os << ", ";
      os << vs.values[i];
    }
    return os << "]";
  }
};

template <class VS>
Container<typename VS::value_type> to_container(const VS& vs) {
  return Container<typename VS::value_type>{vs};
}

}  // namespace test
#endif // FILE_A93FFB28_73C5_446E_8B92_F57759FA7E26_H
