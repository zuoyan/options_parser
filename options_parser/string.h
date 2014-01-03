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

inline string join(const std::vector<string> &vs, const string &sep) {
  string ret;
  for (size_t i = 0; i < vs.size(); ++i) {
    if (i > 0) ret += sep;
    ret += vs[i];
  }
  return ret;
}

inline std::vector<string> split_one_line(const string &s, size_t width,
                                          size_t indent = 0) {
  assert(s.find('\n') >= s.size());
  std::vector<string> lines;
  if (!s.size()) {
    lines.push_back("");
    return lines;
  }
  size_t off = 0;
  size_t more_indent = 0;
  size_t c_indent = indent + more_indent;
  string spaces;
  spaces.assign(width, ' ');
  while (s.size() > off + c_indent + width) {
    size_t n = off;
    while (1) {
      auto p = s.find_first_of(" \t", n + 1);
      if (p >= s.size()) break;
      if (s[p] == '\t') {
        more_indent = 2;
      }
      if (p - off + c_indent > width) {
        break;
      }
      n = p;
    }
    bool space_break = true;
    if (n == off) {
      space_break = false;
      n = off + (width - c_indent);
    }
    string line = s.substr(off, n - off);
    while (line.find('\t') < line.size()) {
      line[line.find('\t')] = ' ';
    }
    line = spaces.substr(0, c_indent) + line;
    lines.push_back(line);
    c_indent = indent + more_indent;
    off = n + space_break;
  }
  if (off < s.size())
    lines.push_back(spaces.substr(0, c_indent) + s.substr(off));
  return lines;
}

inline std::vector<string> format_str(const string &s, int width) {
  std::vector<string> ret;
  auto lines = split(s, "\n");
  for (const auto &line : lines) {
    auto ls = split_one_line(line, width, 0);
    for (const auto &l : ls) {
      ret.push_back(l);
    }
  }
  return ret;
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
    if (next > o.size()) next = o.size();
    ret += o.substr(off, next - off);
    if (next < o.size()) ret += n;
    off = next + p.size();
  }
  return ret;
}

}  // namespace options_parser
#endif  // FILE_B3B012C5_486F_4FF9_8AF3_C94136A260CE_H
