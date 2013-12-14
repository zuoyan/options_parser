#ifndef FILE_75F4A863_93A7_4BAE_A77E_5C14D933BBCA_H
#define FILE_75F4A863_93A7_4BAE_A77E_5C14D933BBCA_H
#include <string>
#include <sstream>

#include "options_parser/maybe.h"

namespace options_parser {

template <class T, class Tag = void>
struct from_stream_impl;

template <class T>
bool from_stream(std::istream &is, T *p);

template <class T, class Tag = void>
struct from_str_impl;

Maybe<string> from_str(const string &s, string *p);

template <class T>
Maybe<string> from_str(const string &s, T *p);

template <class T>
struct has_operator_to_stream {
  template <class U>
  static auto deduce(U *ptr) -> decltype(*(std::ostream *)0 << *ptr);
  template <class U>
  static char deduce(...);

  static constexpr bool value =
      !std::is_same<decltype(deduce<T>((T *)0)), char>::value;
};

template <class T, class Tag=void>
struct to_stream_impl;

template <class T>
struct to_stream_impl<
    T, typename std::enable_if<has_operator_to_stream<T>::value>::type> {
  static bool to_stream(std::ostream &os, const T &p) {
    os << p;
    return !os.fail();
  }
};

template <class T>
auto to_stream(std::ostream &os, const T &p)
    OPTIONS_PARSER_AUTO_RETURN(to_stream_impl<T>::to_stream(os, p));

template <class T>
struct has_to_stream {
  template <class U>
  static auto deduce(U *ptr)
      -> decltype(to_stream_impl<U>::to_stream(*(std::ostream *)0, *ptr));

  template <class U>
  static char deduce(...);

  static constexpr bool value =
      std::is_same<decltype(deduce<T>((T *)0)), bool>::value;
};

static_assert(has_to_stream<int>::value, "...");
static_assert(!has_to_stream<has_to_stream<int>>::value, "...");

template <class T, class Tag = void>
struct to_str_impl;

template <class T>
struct to_str_impl<T, typename std::enable_if<has_to_stream<T>::value>::type> {
  static string to_str(const T &v) {
    std::ostringstream os;
    to_stream(os, v);
    return os.str();
  }
};

template <class T>
auto to_str(const T &v) OPTIONS_PARSER_AUTO_RETURN(to_str_impl<T>::to_str(v));

inline string to_str(const string &v);

template <class T>
struct has_to_str {
  template <class U>
  static auto deduce(U *)
      -> decltype(options_parser::to_str(std::declval<U>()));

  template <class U>
  static char deduce(...);

  static constexpr bool value =
      !std::is_same<decltype(deduce<T>((T *)0)), char>::value;
};

static_assert(has_to_str<int>::value, "...");
static_assert(!has_to_str<has_to_str<int>>::value, "...");

}  // namespace options_parser
#endif  // FILE_75F4A863_93A7_4BAE_A77E_5C14D933BBCA_H
