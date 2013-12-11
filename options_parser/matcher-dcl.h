#ifndef FILE_FB6492BE_F7CB_4344_9575_466EAC3B995E_H
#define FILE_FB6492BE_F7CB_4344_9575_466EAC3B995E_H
#include <functional>

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

struct MatchFromDoc {
  string doc;
  string name;
  std::vector<string> opts;
  bool is_optional;
  size_t num_args;

  MatchFromDoc(const string &d, bool strip = true) : doc(d) {
    num_args = 0;
    is_optional = false;
    bool with_prefix = false;
    if (d.find('|') < d.size() || d.find_first_of(" ,<>=") >= d.size()) {
      opts = split(d, "|");
      for (string &o : opts) {
        size_t off = 0;
        while (off < o.size() && o[off] == '-') ++off;
        if (off > 0) {
          with_prefix = true;
        }
      }
      if (with_prefix) {
        doc = join(opts, ", ");
        if (opts.size()) {
          name = opts.back();
          size_t off = 0;
          while (off < name.size() && name[off] == '-') ++off;
          name = name.substr(off);
        }
      } else {
        doc.clear();
        for (const string &o : opts) {
          if (doc.size()) doc += ", ";
          doc += (o.size() == 1 ? "-" : "--") + o;
        }
        if (opts.size()) name = opts.back();
      }
      return;
    }
    size_t off = 0;
    auto count_args = [&](size_t &off) {
      while (off < d.size() && isspace(d[off])) ++off;
      bool eq = off < d.size() && d[off] == '[';
      if (eq) ++off;
      size_t n = 0;
      while (off < d.size()) {
        if (d[off] == '=') {
          ++off;
          continue;
        }
        if (isspace(d[off])) {
          ++off;
          continue;
        }
        if (d[off] == '<') {
          while (off < d.size() && d[off] != '>') {
            ++off;
          }
          ++n;
        }
        if (d[off] == ',') {
          ++off;
          break;
        }
        if (d[off] == '-') {
          break;
        }
        while (off < d.size() && !isspace(d[off])) {
          ++off;
        }
        ++n;
      }
      if (eq && off < d.size() && d[off] == ']') ++off;
      if (eq) is_optional = true;
      return n;
    };
    while (off < d.size()) {
      if (off < d.size() && isspace(d[off])) {
        ++off;
        continue;
      }
      assert(d[off] == '-');
      size_t n = off;
      while (n < d.size() && d[n] == '-') ++n;
      size_t t = n + 1;
      if (n - off > 1) {
        while (t < d.size() && !isspace(d[t]) && d[t] != '=') ++t;
      }
      opts.emplace_back(d.data() + n, d.data() + t);
      off = t;
      auto na = count_args(off);
      if (na < num_args) num_args = na;
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

  Matcher(const std::vector<string> &opts,
          Maybe<Priority> exact_priority = nothing,
          Maybe<Priority> prefix_priority = nothing,
          Maybe<state<Either<string>, Situation>> arg_getter = nothing);

  template <class S, decltype(*(string *)0 = std::declval<S>(), 0) = 0>
  Matcher(S &&s, Maybe<Priority> exact_priority = nothing,
          Maybe<Priority> prefix_priority = nothing,
          Maybe<state<Either<string>, Situation>> arg_getter =
              nothing) {
    MatchFromDoc from_doc(s);
    new (this)
        Matcher(from_doc.opts, exact_priority, prefix_priority, arg_getter);
  }

  MatchResult operator()(const Situation &s) const;

  std::function<MatchResult(const Situation &)> match_;
};

}  // namespace options_parser
#endif  // FILE_FB6492BE_F7CB_4344_9575_466EAC3B995E_H
