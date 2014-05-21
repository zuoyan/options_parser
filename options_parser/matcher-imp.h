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

OPTIONS_PARSER_IMP Matcher::Matcher(const MatchFromDescription &mfd,
                                    Maybe<Value<string>> arg_getter) {
  match_ = [mfd, arg_getter](const Situation &s) {
    std::pair<Either<string>, Situation> m_s;
    if (arg_getter) {
      m_s = (*arg_getter.get())(s);
    } else {
      if (mfd.is_raw) {
        m_s = value()(s);
      } else {
        m_s = match_value('-', true)(s);
      }
    }
    MatchResult mr;
    mr.priority = 0;
    mr.start = s.position;
    mr.situation = m_s.second;
    if (get_error(m_s.first)) {
      return mr;
    }
    auto arg = *m_s.first.value.get();
    string first_arg;
    {
      auto t = s;
      t.position.off = 0;
      first_arg = get_value(value()(t).first.value);
    }
    if (mfd.is_raw || (s.position.off == 0 && starts_with(first_arg, "--"))) {
      for (const string &o : mfd.opts) {
        if (o == arg) {
          mr.priority = MATCH_EXACT;
          return mr;
        }
      }
    }
    if (arg.size() && s.position.off == 0 &&
        (mfd.is_raw || starts_with(first_arg, "--"))) {
      for (const string &o : mfd.opts) {
        if (arg.size() < o.size() && o.compare(0, arg.size(), arg) == 0) {
          mr.priority = MATCH_PREFIX;
          return mr;
        }
      }
    }
    if (mfd.is_raw) return mr;
    if (!arg_getter && arg.size() && !starts_with(first_arg, "--")) {
      for (const string &o : mfd.opts) {
        if (o.size() != 1) continue;
        if (arg[0] != o[0]) continue;
        mr.priority = MATCH_EXACT;
        size_t off = s.position.off;
        if (off == 0 && off < first_arg.size() && first_arg[off] == '-') ++off;
        ++off;
        if (off < first_arg.size()) {
          mr.situation = s;
          if (first_arg[off] == '=') {
            mr.situation.position.off = off + 1;
          } else {
            mr.situation.position.off = off;
          }
        }
        return mr;
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
