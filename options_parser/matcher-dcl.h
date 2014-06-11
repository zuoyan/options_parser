#ifndef FILE_FB6492BE_F7CB_4344_9575_466EAC3B995E_H
#define FILE_FB6492BE_F7CB_4344_9575_466EAC3B995E_H
#include <functional>
#include <algorithm>

#include "options_parser/maybe.h"
#include "options_parser/join.h"
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

// Description string for match
//
// The first non space character is the prefix(usually '-'), we'll try to parse
// it like a long and short options descriptions, for example, see 'ls --help'.
//
// Example:
//
// '-a, --author AUTHOR' will matches '-a=name', '-a name', '-aname', '--author
//  name', '--author=name', and even '--autho name'(prefix match).
//
// If we have added -a, -u, -t, -h, -o, -r, then -author will match all of them.
//
// '/a, /author AUTHOR' will matches '/a=name', '/a name', '/author=name'.
struct MatchDescription {
  string prefix;
  string doc;
  std::vector<string> opts;
  string name;
  bool is_arg_optional;
  size_t num_args;

  MatchDescription(const string &d);
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

  template <class F,
            decltype(*(int *)0 = get_value(
                         std::declval<F>()(std::declval<Situation>()).first),
                     0) = 0>
  Matcher(const F &func) {
    match_ = [func](const Situation &s) {
      auto v_s = func(s);
      MatchResult mr;
      mr.start = s.position;
      mr.situation = v_s.second;
      mr.priority = 0;
      if (!is_error(v_s.first)) {
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
      if (!is_error(a_s.first)) {
        string arg = get_value(a_s.first);
        mr.priority = func(arg);
      }
      return mr;
    };
  }

  template <class S, decltype(*(string *)0 = std::declval<S>(), 0) = 0>
  Matcher(S &&s) {
    MatchDescription d(s);
    new (this) Matcher(d);
  }

  Matcher(const MatchDescription &md);

  MatchResult operator()(const Situation &s) const;

  std::function<MatchResult(const Situation &)> match_;
};

}  // namespace options_parser
#endif  // FILE_FB6492BE_F7CB_4344_9575_466EAC3B995E_H
