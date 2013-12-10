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

struct Formatter {
  Formatter() = default;

  Formatter(const Formatter &) = default;

  std::vector<string> operator()(size_t width) const {
    if (format_) return format_(width);
    return std::vector<string>{};
  }

  inline Formatter(const property<string> &text) {
    format_ = [text](size_t width) {
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
  }

  template <class F, typename std::enable_if<mpl::is_callable<F, int>::value,
                                             int>::type = 0>
  Formatter(const F &f) {
    format_ = f;
  }

  std::function<std::vector<string>(size_t width)> format_;
};

inline Formatter hang(size_t first_width, string hsep, string vsep,
                      const Formatter &first, const Formatter &second) {
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
  return Formatter(func);
}

inline Formatter every_left(const string &left, const Formatter &f) {
  auto func = [left, f](size_t width) {
    auto vs = f(left.size() >= width ? 1 : width - left.size());
    for (auto &v : vs) {
      v = left + v;
    }
    return vs;
  };
  return Formatter(func);
}

inline Formatter indent(const size_t indent, const Formatter &f) {
  return every_left(string(indent, ' '), f);
}

inline Formatter vcat(const std::vector<Formatter> &fs) {
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
  return Formatter(func);
}

template <class... M>
inline Formatter vcat(std::vector<Formatter> fs, const Formatter &a,
                      const M &... more) {
  fs.push_back(a);
  return vcat(fs, more...);
}

template <class H, class...M>
inline Formatter vcat(const H &a, const M &... more) {
  return vcat(std::vector<Formatter>{}, Formatter(a), Formatter(more)...);
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

  Formatter format_;
};

template <class T>
property<string> delay_to_str(const T *ptr);
}  // namespace options_parser
#endif  // FILE_29B677D6_8334_48D6_A77C_1B719C1198AD_H
