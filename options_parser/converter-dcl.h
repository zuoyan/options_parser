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
struct to_stream_impl;

template <class T>
bool to_stream(std::ostream &os, const T &p);

template <class T, class Tag = void>
struct from_str_impl;

inline Maybe<string> from_str(const string &s, string *p);

template <class T>
Maybe<string> from_str(const string &s, T *p);

template <class T, class Tag = void>
struct to_str_impl;

template <class T>
string to_str(const T &v);

inline string to_str(const string &v);

}  // namespace options_parser
#endif  // FILE_75F4A863_93A7_4BAE_A77E_5C14D933BBCA_H
