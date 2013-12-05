#ifndef FILE_0F882BB8_C84E_465A_BE27_559342975AC1_H
#define FILE_0F882BB8_C84E_465A_BE27_559342975AC1_H
#include "options_parser/config.h"
#include "options_parser/arguments-imp.h"
#include "options_parser/position-imp.h"

namespace options_parser {
inline Matcher::Matcher(Priority priority) {
  match_ = [priority](const PositionArguments &s) {
    MatchResult mr;
    mr.start = s.position;
    mr.end = s.position;
    mr.args = s.args;
    mr.priority = priority;
    return mr;
  };
}

inline Matcher::Matcher(
    const std::vector<string> &opts, Maybe<Priority> exact_priority,
    Maybe<Priority> prefix_priority,
    Maybe<state<Either<string>, PositionArguments>> arg_getter) {
  match_ = [opts, exact_priority, prefix_priority, arg_getter](
      const PositionArguments &s) {
    bool has_raw = false;
    for (const auto &opt : opts) {
      if (opt.size() && opt.at(0) == '-') {
        has_raw = true;
        break;
      }
    }
    std::pair<Either<string>, PositionArguments> m_s;
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

inline MatchResult Matcher::operator()(const PositionArguments &s) const {
  return match_(s);
}

}  // namespace options_parser
#endif  // FILE_0F882BB8_C84E_465A_BE27_559342975AC1_H
