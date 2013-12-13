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
    size_t off = 0;
    while (off < d.size() && isspace(d[off])) ++off;
    if (off == doc.size()) return;
    if (doc[off] != '-') {
      init_not_doc();
      return;
    }

    auto count_args = [&](size_t &off) {
      while (off < d.size() && isspace(d[off])) ++off;
      bool sq = off < d.size() && d[off] == '[';
      if (sq) ++off;
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
          if (off < d.size()) ++off;
          ++n;
          continue;
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
      if (sq && off < d.size() && d[off] == ']') ++off;
      if (sq) is_arg_optional = true;
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
        while (t < d.size() && !isspace(d[t]) && d[t] != '=' && d[t] != '[') {
          ++t;
        }
      }
      opts.emplace_back(d.data() + n, d.data() + t);
      off = t;
      size_t na = count_args(off);
      if (na > num_args) num_args = na;
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

  template <class S, decltype(*(string *)0 = std::declval<S>(), 0) = 0>
  Matcher(S &&s, Maybe<state<Either<string>, Situation>> arg_getter = nothing) {
    MatchFromDescription from_desc(s);
    new (this) Matcher(from_desc, arg_getter);
  }

  Matcher(const MatchFromDescription &mfd,
          Maybe<state<Either<string>, Situation>> arg_getter = nothing);

  MatchResult operator()(const Situation &s) const;

  std::function<MatchResult(const Situation &)> match_;
};

}  // namespace options_parser
#endif  // FILE_FB6492BE_F7CB_4344_9575_466EAC3B995E_H
