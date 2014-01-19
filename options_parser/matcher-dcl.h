#ifndef FILE_FB6492BE_F7CB_4344_9575_466EAC3B995E_H
#define FILE_FB6492BE_F7CB_4344_9575_466EAC3B995E_H
#include <functional>
#include <algorithm>

#include "options_parser/maybe.h"
#include "options_parser/arguments-dcl.h"
#include "options_parser/position-dcl.h"

namespace options_parser {
typedef int Priority;

constexpr Priority MATCH_NONE = 0;
constexpr Priority MATCH_POSITION = 10;
constexpr Priority MATCH_PREFIX = 1000;
constexpr Priority MATCH_EXACT = 10000;

struct MatchResult {
  Situation situation;
  Position start;
  Priority priority;
};

struct MatchFromDescription {
  string doc;
  string name;
  std::vector<string> opts;
  bool is_arg_optional;
  size_t num_args;
  bool is_raw;

  void init_not_doc() {
    if (doc[0] == '|') {
      is_raw = true;
      opts = split(doc.substr(1), "|");
      if (opts.size()) name = opts.back();
      doc = join(opts, ", ");
      return;
    }
    opts = split(doc, "|");
    if (opts.size()) name = opts.back();
    doc.clear();
    for (auto o : opts) {
      if (doc.size()) doc += ", ";
      doc += (o.size() == 1 ? "-" : "--") + o;
    }
  }

  MatchFromDescription(const string &d) : doc(d) {
    num_args = 0;
    is_arg_optional = false;
    is_raw = false;

    {
      size_t off = 0;
      while (off < d.size() && isspace(d[off])) ++off;
      if (off == doc.size()) return;
      if (doc[off] != '-') {
        init_not_doc();
        return;
      }
    }

    auto desc_split = [](string s) {
      std::vector<string> ret;
      size_t off = 0;
      while (off < s.size()) {
        if (isspace(s[off]) || s[off] == ',') {
          ++off;
          continue;
        }
        if (s[off] == '[') {
          ret.push_back(s.substr(off, 1));
          ++off;
          if (off < s.size() && s[off] == '=') ++off;
          continue;
        }
        if (s[off] == ']') {
          ret.push_back(s.substr(off, 1));
          ++off;
          continue;
        }
        if (s[off] == '-') {
          if (off + 1 < s.size() && s[off + 1] != '-') {
            ret.push_back(s.substr(off, 2));
            off += 2;
            if (off < s.size() && s[off] == '=') {
              ++off;
            }
            continue;
          }
          size_t n = off;
          while (n < s.size() && !isspace(s[n]) && !strchr("[]=,", s[n])) {
            ++n;
          }
          ret.push_back(s.substr(off, n - off));
          off = n;
          if (off < s.size() && s[off] == '=') {
            ++off;
          }
          continue;
        }
        if (s[off] == '<') {
          size_t n = s.find('>', off);
          if (n >= s.size()) {
            n = s.size();
          } else {
            ++n;
          }
          ret.push_back(s.substr(off, n - off));
          off = n;
          continue;
        }
        size_t n = off;
        while (n < s.size() && !isspace(s[n])) ++n;
        ret.push_back(s.substr(off, n - off));
        off = n;
      }
      return ret;
    };

    std::vector<string> vs = desc_split(doc);
    is_arg_optional = std::find(vs.begin(), vs.end(), "[") != vs.end();
    {
      auto it = std::remove_if(vs.begin(), vs.end(),
                               [](string s) { return s == "[" || s == "]"; });
      vs.erase(it, vs.end());
    }

    size_t off = 0;
    while (off < vs.size()) {
      assert(vs[off][0] == '-');
      size_t o = 0;
      while (o < vs[off].size() && vs[off][o] == '-') ++o;
      opts.push_back(vs[off].substr(o));
      size_t n = off + 1;
      while (n < vs.size() && vs[n][0] != '-') ++n;
      size_t na = n - off - 1;
      if (num_args < na) num_args = na;
      off = n;
    }
    if (opts.size()) name = opts.back();
  }
};

struct Matcher {
  Matcher() = default;
  Matcher(const Matcher &) = default;

  Matcher(Priority priority);

  template <class F, decltype(*(MatchResult *)0 =
                                  std::declval<F>()(std::declval<Situation>()),
                              0) = 0>
  Matcher(const F &func) {
    match_ = func;
  }

  template <class F, decltype(*(int *)0 = get_value(std::declval<F>()(
                                  std::declval<Situation>()).first),
                              0) = 0>
  Matcher(const F &func) {
    match_ = [func](const Situation &s) {
      auto v_s = func(s);
      MatchResult mr;
      mr.start = s.position;
      mr.situation = v_s.second;
      mr.priority = 0;
      if (!get_error(v_s.first)) {
        mr.priority = get_value(v_s.first);
      }
      return mr;
    };
  }

  template <
      class F,
      decltype(*(int *)0 = std::declval<F>()(std::declval<string>()), 0) = 0>
  Matcher(const F &func) {
    match_ = [func](const Situation &s) {
      auto a_s = value()(s);
      MatchResult mr;
      mr.start = s.position;
      mr.situation = a_s.second;
      mr.priority = 0;
      if (!get_error(a_s.first)) {
        string arg = get_value(a_s.first);
        mr.priority = func(arg);
      }
      return mr;
    };
  }

  template <class S, decltype(*(string *)0 = std::declval<S>(), 0) = 0>
  Matcher(S &&s, Maybe<Value<string>> arg_getter = nothing) {
    MatchFromDescription from_desc(s);
    new (this) Matcher(from_desc, arg_getter);
  }

  Matcher(const MatchFromDescription &mfd,
          Maybe<Value<string>> arg_getter = nothing);

  MatchResult operator()(const Situation &s) const;

  std::function<MatchResult(const Situation &)> match_;
};

}  // namespace options_parser
#endif  // FILE_FB6492BE_F7CB_4344_9575_466EAC3B995E_H
