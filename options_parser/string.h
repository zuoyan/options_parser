#ifndef FILE_B3B012C5_486F_4FF9_8AF3_C94136A260CE_H
#define FILE_B3B012C5_486F_4FF9_8AF3_C94136A260CE_H
#include <cassert>
#include <string>
#include <vector>

namespace options_parser {
typedef std::string string;

inline bool starts_with(const string &s, const string &prefix) {
  return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

inline std::vector<string> split(const string &s, const string &sep,
                                 size_t limit = -1) {
  std::vector<string> fields;
  size_t off = 0;
  while (off < s.size() && fields.size() < limit) {
    auto n = s.find(sep, off);
    if (n >= s.size()) break;
    fields.push_back(s.substr(off, n - off));
    off = n + sep.size();
  }
  if (off < s.size()) fields.push_back(s.substr(off));
  return fields;
}

inline string strip(const string &s) {
  size_t l = 0, r = s.size();
  while (l < r && isspace(s[l])) {
    ++l;
  }
  while (l < r && isspace(s[r - 1])) {
    --r;
  }
  return s.substr(l, r - l);
}

inline string replace(const string &o, const string &p, const string &n) {
  size_t off = 0;
  string ret;
  while (off < o.size()) {
    size_t next = o.find(p, off);
    if (next == string::npos) next = o.size();
    ret += o.substr(off, next - off);
    if (next < o.size()) ret += n;
    off = next + p.size();
  }
  return ret;
}

}  // namespace options_parser
#endif  // FILE_B3B012C5_486F_4FF9_8AF3_C94136A260CE_H
