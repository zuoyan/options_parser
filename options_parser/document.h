#ifndef FILE_9B151F94_CE43_43EA_8BDE_46725FB26379_H
#define FILE_9B151F94_CE43_43EA_8BDE_46725FB26379_H
#include <string>
#include <vector>

#include "options_parser/maybe.h"
#include "options_parser/property.h"
#include "options_parser/converter.h"

namespace options_parser {

struct Document {
  Document() { message_ = false; }

  Document(const Document &) = default;

  template <class P, class D>
  Document(const P &prefix, const D &description) {
    message_ = false;
    set_prefix(prefix);
    set_description(description);
  }

  template <class D>
  Document(const D &description) {
    message_ = false;
    set_description(description);
  }

  std::string prefix() const {
    if (prefix_.empty()) return std::string();
    return prefix_;
  }

  std::string description() const {
    if (description_.empty()) return std::string();
    return description_;
  }

  template <class P>
  Document &set_prefix(const P &p) {
    prefix_ = p;
    return *this;
  }

  template <class D>
  Document &set_description(const D &d) {
    description_ = d;
    return *this;
  }

  template <class D>
  Document &set_message(const D &d) {
    message_ = true;
    return set_description(d);
  }

  static std::vector<std::string> split(const std::string &s, char sep) {
    std::vector<std::string> lines;
    size_t off = 0;
    while (off < s.size()) {
      auto n = s.find('\n', off);
      if (n >= s.size()) break;
      lines.push_back(s.substr(off, n - off));
      off = n + 1;
    }
    if (off < s.size()) lines.push_back(s.substr(off));
    return lines;
  }

  static std::string join(const std::vector<std::string> &vs, char sep) {
    std::string ret;
    for (size_t i = 0; i < vs.size(); ++i) {
      if (i > 0) ret += sep;
      ret += vs[i];
    }
    return ret;
  }

  static std::vector<std::string> split_one_line(const std::string &s,
                                                 size_t width,
                                                 size_t indent = 0) {
    assert(s.find('\n') >= s.size());
    std::vector<std::string> lines;
    if (!s.size()) {
      lines.push_back("");
      return lines;
    }
    size_t off = 0;
    size_t more_indent = 0;
    size_t c_indent = indent + more_indent;
    std::string spaces;
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
      std::string line = s.substr(off, n - off);
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

  static std::vector<std::string> format_str(const std::string &s, int width) {
    std::vector<std::string> ret;
    auto lines = split(s, '\n');
    for (const auto &line : lines) {
      auto ls = split_one_line(line, width, 0);
      for (const auto &l : ls) {
        ret.push_back(l);
      }
    }
    return ret;
  }

  std::vector<std::string> format(size_t width) const {
    std::vector<std::string> ret;
    if (message_) {
      if (description_.empty()) return ret;
      return format_str((std::string)description_, width);
    }
    std::string p, d;
    if (!prefix_.empty()) p = prefix_;
    if (!description_.empty()) d = description_;
    const size_t prefix_width = 18;
    const size_t prefix_sep = 2;
    std::string left(prefix_width + prefix_sep, ' ');
    std::string sep(prefix_sep, ' ');
    if (width < prefix_width + prefix_sep + prefix_width) {
      width = prefix_width + prefix_sep + prefix_width;
    }
    auto plines = format_str(p, prefix_width);
    auto dlines = format_str(d, width - prefix_width - prefix_sep);
    if (plines.size() == 0) {
      for (const auto &l : dlines) {
        ret.push_back(p + l);
      }
      return ret;
    }
    if (plines.size() == 1) {
      if (!dlines.size()) return plines;
      ret.push_back(plines.front() + left.substr(plines.front().size()) +
                    dlines.front());
      for (size_t i = 1; i < dlines.size(); ++i) {
        ret.push_back(left + dlines[i]);
      }
      return ret;
    }
    ret = format_str(p, width);
    for (const auto &l : dlines) {
      ret.push_back(left + l);
    }
    return ret;
  }

  property<std::string> prefix_;
  property<std::string> description_;
  // if message_ is true, we will format this Document without prefix
  bool message_;
};

template <class T>
property<std::string> delay_to_str(const T *ptr) {
  return [ptr]() { return to_str(*ptr); };
}

}  // namespace options_parser
#endif  // FILE_9B151F94_CE43_43EA_8BDE_46725FB26379_H
