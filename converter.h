#ifndef FILE_C7E3D7AA_507A_4DB8_9B44_120AB8347570_H
#define FILE_C7E3D7AA_507A_4DB8_9B44_120AB8347570_H
#include <string>
#include <sstream>

#include "maybe.h"

namespace options_parser {

template <class T, class Tag = void>
struct from_stream_impl {
  static bool from_stream(std::istream &is, T *p) {
    is >> *p;
    return !is.fail();
  }
};

template <class T>
bool from_stream(std::istream &is, T *p) {
  return from_stream_impl<T>::from_stream(is, p);
}

template <class T, class Tag = void>
struct to_stream_impl {
  static bool to_stream(std::ostream &os, const T &p) {
    os << p;
    return !os.fail();
  }
};

template <class T>
bool to_stream(std::ostream &os, const T &p) {
  return to_stream_impl<T>::to_stream(os, p);
}

template <class T, class Tag = void>
struct from_str_impl {
  static Maybe<std::string> from_str(const std::string &s, T *p) {
    std::istringstream is(s);
    if (!from_stream(is, p)) {
      return std::string("from_str<") + typeid(T).name() + ">(\"" + s +
             "\") failed";
    }
    if (!is.eof()) {
      std::stringstream t;
      t << is.rdbuf();
      return std::string("from_str<") + typeid(T).name() + ">(\"" + s +
             "\")"
             " rest \"" +
             t.str() + "\"";
    }
    return nothing;
  }
};

Maybe<std::string> from_str(const std::string &s, std::string *p) {
  *p = s;
  return nothing;
}

template <class T>
Maybe<std::string> from_str(const std::string &s, T *p) {
  return from_str_impl<T>::from_str(s, p);
}

template <class T, class Tag = void>
struct to_str_impl {
  static std::string to_str(const T &v) {
    std::ostringstream os;
    to_stream(os, v);
    return os.str();
  }
};

template <class T>
std::string to_str(const T &v) {
  return to_str_impl<T>::to_str(v);
}

std::string to_str(const std::string &v) { return v; }

}  // namespace options_parser
#endif  // FILE_C7E3D7AA_507A_4DB8_9B44_120AB8347570_H
