#ifndef FILE_FB6492BE_F7CB_4344_9575_466EAC3B995E_H
#define FILE_FB6492BE_F7CB_4344_9575_466EAC3B995E_H
#include <functional>

#include "options_parser/maybe.h"
#include "options_parser/arguments-dcl.h"
#include "options_parser/position.h"

namespace options_parser {
typedef int Priority;

constexpr Priority MATCH_NONE = 0;
constexpr Priority MATCH_POSITION = 10;
constexpr Priority MATCH_PREFIX = 1000;
constexpr Priority MATCH_EXACT = 10000;

struct MatchResult {
  Arguments args;
  Position start;
  Position end;
  Priority priority;
};

struct Matcher {
  Matcher() = default;
  Matcher(const Matcher &) = default;

  Matcher(Priority priority);

  template <class F, decltype(*(MatchResult *)0 = std::declval<F>()(
                                  std::declval<PositionArguments>()),
                              0) = 0>
  Matcher(const F &func) {
    match_ = func;
  }

  template <class F, decltype(*(int *)0 = get_value(std::declval<F>()(
                                  std::declval<PositionArguments>()).first),
                              0) = 0>
  Matcher(const F &func) {
    match_ = [func](const PositionArguments &s) {
      auto v_s = func(s);
      MatchResult mr;
      mr.start = s.position;
      mr.args = v_s.second.args;
      mr.end = v_s.second.position;
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
          Maybe<state<Either<string>, PositionArguments>> arg_getter = nothing);

  template <class S, decltype(*(string *)0 = std::declval<S>(), 0) = 0>
  Matcher(S &&s, Maybe<Priority> exact_priority = nothing,
          Maybe<Priority> prefix_priority = nothing,
          Maybe<state<Either<string>, PositionArguments>> arg_getter =
              nothing) {
    string opt_str = s;
    std::vector<string> opts;
    {
      size_t off = 0;
      while (off < opt_str.size()) {
        size_t n = opt_str.find('|', off);
        if (n >= opt_str.size()) n = opt_str.size();
        opts.push_back(opt_str.substr(off, n - off));
        off = n + 1;
      }
    }
    new (this) Matcher(opts, exact_priority, prefix_priority, arg_getter);
  }

  MatchResult operator()(const PositionArguments &s) const;

  std::function<MatchResult(const PositionArguments &)> match_;
};

}  // namespace options_parser
#endif  // FILE_FB6492BE_F7CB_4344_9575_466EAC3B995E_H
