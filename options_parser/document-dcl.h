#ifndef FILE_29B677D6_8334_48D6_A77C_1B719C1198AD_H
#define FILE_29B677D6_8334_48D6_A77C_1B719C1198AD_H
#include <string>
#include <vector>

#include "options_parser/maybe.h"
#include "options_parser/property.h"
#include "options_parser/string.h"
#include "options_parser/converter-dcl.h"
#include "options_parser/line_breaking.h"

namespace options_parser {

typedef std::function<std::vector<string>(size_t width)> formatter;

inline formatter formatter_hang(size_t first_width, string hsep, string vsep,
                                const formatter &first,
                                const formatter &second) {
  auto func = [first_width, hsep, vsep, first, second](size_t width) {
    auto f = first(width);
    if (width <= first_width + hsep.size() || f.size() != 1 ||
        f.front().size() > first_width) {
      if (vsep.size()) f.push_back(vsep);
      auto s = second(width);
      auto left = string(first_width, ' ') + hsep;
      for (const auto &l : s) {
        f.push_back(left + l);
      }
      return f;
    }
    auto s = second(width - first_width - hsep.size());
    if (s.size()) {
      s.front() = f.front() + string(first_width - f.front().size(), ' ') +
                  hsep + s.front();
      auto left = string(first_width + hsep.size(), ' ');
      for (size_t i = 1; i < s.size(); ++i) {
        s[i] = left + s[i];
      }
      return s;
    }
    return f;
  };
  return func;
}

inline formatter formatter_left(const string &left, const formatter &f) {
  auto func = [left, f](size_t width) {
    auto vs = f(left.size() >= width ? 1 : width - left.size());
    for (auto &v : vs) {
      v = left + v;
    }
    return vs;
  };
  return func;
}

inline formatter formatter_indent(const size_t indent, const formatter &f) {
  return formatter_left(string(indent, ' '), f);
}

inline formatter as_formatter(const property<string> &text) {
  auto func = [text](size_t width) {
    std::vector<string> ret;
    if (!text.empty()) {
      auto ls = split((string)text, '\n');
      for (const auto &l : ls) {
        auto vs = line_breaking::break_string(l, width);
        for (const auto &v : vs) {
          ret.push_back(v);
        }
      }
    }
    return ret;
  };
  return func;
}

template <class F>
inline typename std::enable_if<mpl::is_callable<F, int>::value,
                               formatter>::type
as_formatter(const F &f) {
  return f;
}

inline formatter vcat(const std::vector<formatter> &fs) {
  auto func = [fs](size_t width) {
    std::vector<string> vs;
    for (const auto& f : fs) {
      auto ls = f(width);
      for (const auto &l : ls) {
        vs.push_back(l);
      }
    }
    return vs;
  };
  return func;
}

template <class... M>
inline formatter vcat(std::vector<formatter> fs, const formatter &a,
                      const M &... more) {
  fs.push_back(a);
  return vcat(fs, more...);
}

template <class H, class...M>
inline formatter vcat(const H &a, const M &... more) {
  return vcat(std::vector<formatter>{}, as_formatter(a), as_formatter(more)...);
}

struct Document {
  Document();

  Document(const Document &) = default;

  template <class P, class D>
  Document(const P &prefix, const D &description);

  template <class D>
  Document(const D &description);

  template <class D>
  void set_message(const D &d);

  std::vector<string> format(size_t width) const;

  formatter prefix_;
  formatter description_;
  // if message_ is true, we will format this Document without prefix
  bool message_;
};

template <class T>
property<string> delay_to_str(const T *ptr);
}  // namespace options_parser
#endif  // FILE_29B677D6_8334_48D6_A77C_1B719C1198AD_H
