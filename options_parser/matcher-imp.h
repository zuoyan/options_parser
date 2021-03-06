#ifndef FILE_0F882BB8_C84E_465A_BE27_559342975AC1_H
#define FILE_0F882BB8_C84E_465A_BE27_559342975AC1_H
#include "options_parser/config.h"
#include "options_parser/arguments-imp.h"
#include "options_parser/position-imp.h"

namespace options_parser {

OPTIONS_PARSER_IMP MatchDescription::MatchDescription(const string &d)
    : doc(d) {
  num_args = 0;
  is_arg_optional = false;
  size_t off = 0;
  while (off < d.size() && isspace(d[off])) ++off;
  if (off == doc.size()) return;
  prefix = d[off];
  auto desc_split = [](string s, string prefix) {
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
      if (s[off] == prefix.front()) {
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

  std::vector<string> vs = desc_split(doc, prefix);
  is_arg_optional = std::find(vs.begin(), vs.end(), "[") != vs.end();
  {
    auto it = std::remove_if(vs.begin(), vs.end(),
                             [](string s) { return s == "[" || s == "]"; });
    vs.erase(it, vs.end());
  }
  off = 0;
  while (off < vs.size()) {
    assert(starts_with(vs[off], prefix));
    opts.push_back(vs[off]);
    size_t n = off + 1;
    while (n < vs.size() && !starts_with(vs[n], prefix)) ++n;
    size_t na = n - off - 1;
    if (num_args < na) num_args = na;
    off = n;
  }
  if (opts.size()) {
    name = opts.back();
  }
}

OPTIONS_PARSER_IMP Matcher::Matcher(Priority priority) {
  match_ = [priority](const Situation &s) {
    MatchResult mr;
    mr.start = s.position;
    mr.situation = s;
    mr.priority = priority;
    return mr;
  };
}

OPTIONS_PARSER_IMP Matcher::Matcher(const MatchDescription &md) {
  match_ = [md](const Situation &s) {
    MatchResult mr;
    mr.priority = 0;
    mr.start = s.position;
    mr.situation = s;
    auto first_s = s;
    first_s.position.off = 0;
    auto first_value_s = value()(first_s);
    if (is_error(first_value_s.first)) {
      return mr;
    }
    string first_arg = get_value(first_value_s.first);
    if (static_cast<size_t>(s.position.off) >= first_arg.size() ||
        !starts_with(first_arg, md.prefix)) {
      return mr;
    }
    auto arg = first_arg.substr(s.position.off);
    if (arg.find('=') != string::npos) {
      auto p = arg.find('=');
      arg = arg.substr(0, p);
      mr.situation.position.off += arg.size() + 1;
    } else {
      mr.situation.position.off = 0;
      mr.situation.position.index++;
    }
    if (s.position.off > 0) {
      arg += md.prefix;
    }
    for (const string &o : md.opts) {
      if (arg == o) {
        mr.priority = MATCH_EXACT;
        return mr;
      }
    }
    for (const string &o : md.opts) {
      if (starts_with(o, arg)) {
        mr.priority = MATCH_PREFIX;
        return mr;
      }
    }
    // check one
    mr.situation.position = s.position;
    if (s.position.off > 0) {
      arg = md.prefix + first_arg.substr(s.position.off, 1);
      mr.situation.position.off++;
    } else {
      arg = first_arg.substr(0, md.prefix.size() + 1);
      mr.situation.position.off = md.prefix.size() + 1;
    }
    if (static_cast<size_t>(mr.situation.position.off) < first_arg.size() &&
        first_arg[mr.situation.position.off] == '=') {
      mr.situation.position.off++;
    } else if (static_cast<size_t>(mr.situation.position.off) >=
               first_arg.size()) {
      mr.situation.position.off = 0;
      mr.situation.position.index++;
    }
    for (const string &o : md.opts) {
      if (arg == o) {
        mr.priority = MATCH_EXACT;
        return mr;
      }
    }
    mr.situation.position = s.position;
    return mr;
  };
}

OPTIONS_PARSER_IMP MatchResult Matcher::operator()(const Situation &s) const {
  return match_(s);
}

}  // namespace options_parser
#endif  // FILE_0F882BB8_C84E_465A_BE27_559342975AC1_H
