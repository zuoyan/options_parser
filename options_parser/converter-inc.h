#ifndef FILE_7F2A4BB9_F63E_48A8_8477_01B2AED3C183_H
#define FILE_7F2A4BB9_F63E_48A8_8477_01B2AED3C183_H
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
    if (!is.eof()) {
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

template <class T>
Maybe<string> from_str(const string &s, T *p) {
  return from_str_impl<T>::from_str(s, p);
}

}  // namespace options_parser
#endif  // FILE_7F2A4BB9_F63E_48A8_8477_01B2AED3C183_H
