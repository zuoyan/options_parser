#ifndef FILE_7F2A4BB9_F63E_48A8_8477_01B2AED3C183_H
#define FILE_7F2A4BB9_F63E_48A8_8477_01B2AED3C183_H
#include <cstring>

namespace options_parser {

template <class T, class Tag>
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

template <class T, class Tag>
struct from_str_impl {
  static Maybe<string> from_str(const string &s, T *p) {
    std::istringstream is(s);
    if (!from_stream(is, p)) {
      return string("from_str<") + typeid(T).name() + ">(\"" + s + "\") failed";
    }
    if (is.peek() != EOF) {
      std::stringstream t;
      t << is.rdbuf();
      return string("from_str<") + typeid(T).name() + ">(\"" + s +
             "\")"
             " rest \"" +
             t.str() + "\"";
    }
    return nothing;
  }
};

template <class Tag>
struct from_str_impl<bool, Tag> {
  static Maybe<string> from_str(const string &s, bool *p) {
    if (strcasecmp(s.c_str(), "true") == 0 || strcmp(s.c_str(), "1") == 0 ||
        strcasecmp(s.c_str(), "t") == 0) {
      *p = true;
      return nothing;
    }
    if (strcasecmp(s.c_str(), "false") == 0 || strcmp(s.c_str(), "0") == 0 ||
        strcasecmp(s.c_str(), "f") == 0) {
      *p = false;
      return nothing;
    }
    return "from_str<b>(\"" + s + "\") failed";
  }
};

template <class T>
Maybe<string> from_str(const string &s, T *p) {
  static_assert(!std::is_same<T, string>::value, "should not a string ...");
  return from_str_impl<T>::from_str(s, p);
}

}  // namespace options_parser
#endif  // FILE_7F2A4BB9_F63E_48A8_8477_01B2AED3C183_H
