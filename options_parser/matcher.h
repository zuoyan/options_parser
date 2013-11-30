#ifndef FILE_E7F07257_683B_4FA3_B31F_CAF05225457F_H
#define FILE_E7F07257_683B_4FA3_B31F_CAF05225457F_H
#include <functional>

#include "options_parser/maybe.h"
#include "options_parser/arguments.h"
#include "options_parser/position.h"

namespace options_parser {
typedef int Priority;

constexpr Priority MATCH_NONE = 0;
constexpr Priority MATCH_POSITION = 10;
constexpr Priority MATCH_PREFIX = 1000;
constexpr Priority MATCH_EXACT = 10000;

struct MatchResult {
  Arguments *args;
  Position start;
  Position end;
  Priority priority;
};

struct Matcher {
  Matcher() {}

  Matcher(const Matcher &) = default;

  Matcher(Priority priority) {
    match_ = [priority](const PositionArguments &s) {
      MatchResult mr;
      mr.start = s.position;
      mr.end = s.position;
      mr.args = s.args;
      mr.priority = priority;
      return mr;
    };
  }

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

  Matcher(const std::vector<std::string> &opts,
          Maybe<Priority> exact_priority = nothing,
          Maybe<Priority> prefix_priority = nothing,
          Maybe<state<Either<std::string>, PositionArguments>> arg_getter =
              nothing) {
    match_ = [opts, exact_priority, prefix_priority, arg_getter](
        const PositionArguments &s) {
      bool has_raw = false;
      for (const auto &opt : opts) {
        if (opt.size() && opt.at(0) == '-') {
          has_raw = true;
          break;
        }
      }
      std::pair<Either<std::string>, PositionArguments> m_s;
      if (arg_getter) {
        m_s = (*arg_getter.get())(s);
      } else {
        if (has_raw)
          m_s = value()(s);
        else
          m_s = match_value()(s);
      }
      MatchResult mr;
      mr.priority = 0;
      mr.start = s.position;
      mr.args = m_s.second.args;
      mr.end = m_s.second.position;
      if (get_error(m_s.first)) {
        return mr;
      }
      auto arg = *m_s.first.value.get();
      for (auto const &o : opts) {
        if (o == arg) {
          mr.priority = exact_priority ? (Priority)exact_priority : MATCH_EXACT;
          return mr;
        }
      }
      if (!has_raw && arg.size()) {
        for (auto const &o : opts) {
          if (arg.size() < o.size() && o.compare(0, arg.size(), arg) == 0) {
            mr.priority =
                prefix_priority ? (Priority)prefix_priority : MATCH_PREFIX;
            return mr;
          }
        }
      }
      return mr;
    };
  }

  template <class S, decltype(*(std::string *)0 = std::declval<S>(), 0) = 0>
  Matcher(S &&s, Maybe<Priority> exact_priority = nothing,
          Maybe<Priority> prefix_priority = nothing,
          Maybe<state<Either<std::string>, PositionArguments>> arg_getter =
              nothing) {
    std::string opt_str = s;
    std::vector<std::string> opts;
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

  MatchResult operator()(const PositionArguments &s) const { return match_(s); }

  std::function<MatchResult(const PositionArguments &)> match_;
};

}  // namespace options_parser
#endif  // FILE_E7F07257_683B_4FA3_B31F_CAF05225457F_H
