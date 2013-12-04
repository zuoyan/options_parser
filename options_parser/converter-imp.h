#ifndef FILE_D02A7764_8A79_4A54_A05F_E3977CC16DF0_H
#define FILE_D02A7764_8A79_4A54_A05F_E3977CC16DF0_H
namespace options_parser {

inline Maybe<string> from_str(const string &s, string *p) {
  *p = s;
  return nothing;
}

inline string to_str(const string &v) { return v; }

}  // namespace options_parser
#endif  // FILE_D02A7764_8A79_4A54_A05F_E3977CC16DF0_H
