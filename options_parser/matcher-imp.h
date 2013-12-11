#ifndef FILE_0F882BB8_C84E_465A_BE27_559342975AC1_H
#define FILE_0F882BB8_C84E_465A_BE27_559342975AC1_H
#include "options_parser/config.h"
#include "options_parser/arguments-imp.h"
#include "options_parser/position-imp.h"

namespace options_parser {
OPTIONS_PARSER_IMP Matcher::Matcher(Priority priority) {
  match_ = [priority](const Situation &s) {
    MatchResult mr;
    mr.start = s.position;
    mr.situation = s;
    mr.priority = priority;
    return mr;
  };
}

OPTIONS_PARSER_IMP Matcher::Matcher(
    const std::vector<string> &opts, Maybe<Priority> exact_priority,
    Maybe<Priority> prefix_priority,
    Maybe<state<Either<string>, Situation>> arg_getter) {
  match_ = [opts, exact_priority, prefix_priority, arg_getter](
      const Situation &s) {
    bool has_raw = false;
    for (const auto &opt : opts) {
      if (opt.size() && opt.at(0) == '-') {
        has_raw = true;
        break;
      }
    }
    std::pair<Either<string>, Situation> m_s;
    if (arg_getter) {
      m_s = (*arg_getter.get())(s);
    } else {
      m_s = match_value(always_true{}, '-', !has_raw)(s);
    }
    MatchResult mr;
    mr.priority = 0;
    mr.start = s.position;
    mr.situation = m_s.second;
    if (get_error(m_s.first)) {
      return mr;
    }
    auto arg = *m_s.first.value.get();
    for (auto const &o : opts) {
      if (o == arg) {
        mr.priority = exact_priority ? *exact_priority.get() : MATCH_EXACT;
        return mr;
      }
    }
    if (!has_raw && arg.size()) {
      for (auto const &o : opts) {
        if (arg.size() < o.size() && o.compare(0, arg.size(), arg) == 0) {
          mr.priority = prefix_priority ? *prefix_priority.get() : MATCH_PREFIX;
          return mr;
        }
      }
    }
    return mr;
  };
}

OPTIONS_PARSER_IMP MatchResult Matcher::operator()(const Situation &s) const {
  return match_(s);
}

}  // namespace options_parser
#endif  // FILE_0F882BB8_C84E_465A_BE27_559342975AC1_H
