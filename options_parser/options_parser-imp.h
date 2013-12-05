#ifndef FILE_BE44E7FB_480D_4869_A5D8_0EA9C352CB97_H
#define FILE_BE44E7FB_480D_4869_A5D8_0EA9C352CB97_H
#include "options_parser/config.h"
#include "options_parser/arguments-imp.h"
#include "options_parser/document-imp.h"
#include "options_parser/converter-imp.h"
#include "options_parser/matcher-imp.h"
#include "options_parser/position-imp.h"

namespace options_parser {

inline Option::Option() {
  active = true;
  help_level = 0;
  priority = 0;
}

inline Option::Option(Matcher m, Taker t, Document d)
    : match(m), take(t), document(d) {
  active = true;
  help_level = 0;
  priority = 0;
}

inline Taker bundle(const std::vector<Option> &options) {
  return [options](const MatchResult &mr) {
    TakeResult tr;
    PositionArguments pa{mr.start, mr.args};
    std::vector<std::pair<size_t, MatchResult>> mrs;
    for (size_t i = 0; i < options.size(); ++i) {
      auto &opt = options[i];
      auto mr = opt.match(pa);
      if (mr.priority) {
        mrs.push_back(std::make_pair(i, mr));
      }
    }
    Priority pri = std::numeric_limits<Priority>::min();
    for (const auto &x_mr : mrs) {
      if (pri < x_mr.second.priority) pri = x_mr.second.priority;
    }
    auto last = std::remove_if(mrs.begin(), mrs.end(),
                               [&](const std::pair<size_t, MatchResult> &x_mr) {
      return x_mr.second.priority != pri;
    });
    if (last == mrs.begin()) {
      tr.error = "match none in sub options";
      return tr;
    }
    if (last - mrs.begin() > 1) {
      tr.error = "confused to choose from sub options";
      return tr;
    }
    auto &opt = options[mrs.front().first];
    return opt.take(mrs.front().second);
  };
}

inline bool Parser::toggle() {
  if (!holder_) {
    holder_ = std::make_shared<Holder>();
  }
  holder_->active = !holder_->active;
  return holder_->active;
}

inline void Parser::disable() {
  if (toggle()) toggle();
}

inline void Parser::enable() {
  if (!toggle()) toggle();
}

inline ParseResult Parser::parse(const PositionArguments &s) {
  PositionArguments c = s;
  ParseResult pr;
  while (c.position.index < c.args.argc()) {
    auto mr_opts = match_results(c);
    auto show_position = [](PositionArguments pa, size_t limit = 80) {
      string ret = to_str(pa.position.index);
      if (pa.position.off) {
        ret += "off=" + to_str(pa.position.off) + ", ";
      }
      auto p = pa.position;
      p.off = 0;
      if (p.index < pa.args.argc()) {
        ret += " '" + *get_arg(pa.args, p).value.get() + "'";
      } else {
        ret += " over size=" + to_str(pa.args.argc());
      }
      while (ret.size() + 6 < limit && ++p.index < pa.args.argc()) {
        auto a = *get_arg(pa.args, p).value.get();
        if (ret.size() + a.size() + 2 >= limit) {
          a.resize(limit - 5 - ret.size());
          a += " ...";
        }
        ret += " '" + a + "'";
      }
      return ret;
    };
    if (!mr_opts.size()) {
      pr.position = c.position;
      pr.args = c.args;
      pr.error = "match-none";
      pr.error_full = string("match nothing at ") + show_position(c);
      return pr;
    }
    Priority max_pri = std::numeric_limits<Priority>::min();
    for (auto const &mr_opt : mr_opts) {
      if (max_pri < mr_opt.first.priority) {
        max_pri = mr_opt.first.priority;
      }
    }
    auto last = std::remove_if(
        mr_opts.begin(), mr_opts.end(),
        [&](const std::pair<MatchResult, std::shared_ptr<Option>> &mr_opt) {
          return mr_opt.first.priority != max_pri;
        });
    if (last - mr_opts.begin() >= 2) {
      pr.position = c.position;
      pr.args = c.args;
      std::vector<string> lines;
      for (auto it = mr_opts.begin(); it != last; ++it) {
        auto &doc = it->second->document;
        auto ls = doc.format(78);
        if (lines.size()) lines.push_back("");
        lines.insert(lines.end(), ls.begin(), ls.end());
      }
      pr.error = "match-multiple";
      pr.error_full = string("match multiple at ") + show_position(c) +
                      " with following options:\n" + join(lines, '\n');
      return pr;
    }
    auto const &mr_opt = mr_opts.front();
    // std::cerr << "match start " << mr_opt.first.start.index
    //           << ":" << mr_opt.first.start.off << std::endl;
    // std::cerr << "match end " << mr_opt.first.end.index
    //           << ":" << mr_opt.first.end.off << std::endl;
    auto tr = mr_opt.second->take(mr_opt.first);
    // std::cerr << "tr end " << tr.end.index
    //           << ":" << tr.end.off << std::endl;
    if (tr.error) {
      pr.position = c.position;
      pr.args = c.args;
      pr.error = "take-error";
      pr.error_full = string("process failed: ") + *tr.error.get() + "\n" +
                      "at " + show_position(c) + "\n" + "matched option:\n" +
                      join(mr_opt.second->document.format(78), '\n');
      return pr;
    }
    c.position = tr.end;
    c.args = tr.args;
  }
  pr.position = c.position;
  pr.args = c.args;
  return pr;
}

inline ParseResult Parser::parse_string(const string &a) {
  VectorStringArguments args(expand(a));
  return parse({{0, 0}, args});
}

inline size_t Parser::parse_lines(const std::vector<string> &lines,
                                  Maybe<string> *error,
                                  Maybe<string> *error_full) {
  size_t off = 0;
  return parse_lines([&]()->Maybe<string> {
                       if (off < lines.size()) return lines[off++];
                       return nothing;
                     },
                     error, error_full);
}

inline void Parser::parse_file(const string &fn, Maybe<string> *error,
                               Maybe<string> *error_full) {
  std::ifstream ifs(fn);
  if (!ifs.good()) {
    *error = "open-failed";
    *error_full = "open file '" + fn + "' to read failed";
    return;
  }
  auto get_line = [&]()->Maybe<string> {
    if (!ifs.good()) return nothing;
    string l;
    std::getline(ifs, l);
    return l;
  };
  size_t off = parse_lines(get_line, error, error_full);
  if (*error) {
    auto p = "parse file '" + fn + "' failed at line " + to_str(off);
    if (*error_full) {
      *error_full = p + " " + *error_full->get();
    } else {
      *error_full = p;
    }
  }
}

inline void Parser::add_parser(const Parser &parser, int priority) {
  if (!holder_) {
    holder_ = std::make_shared<Holder>();
  }
  holder_->parsers.push_back(std::make_pair(priority, parser.holder_));
}

inline Option *Parser::add_option(const Option &o) {
  if (!holder_) {
    holder_ = std::make_shared<Holder>();
  }
  holder_->options.push_back(std::make_shared<Option>(o));
  return holder_->options.back().get();
}

inline Option *Parser::add_option(const Matcher &m, const Taker &t,
                                  const Document &d) {
  Option opt;
  opt.match = m;
  opt.take = t;
  opt.document = d;
  opt.help_level = holder_ ? holder_->help_level : 0;
  return add_option(opt);
}

inline string Parser::help_message(int level, int width) {
  auto docs = documents(level);
  std::vector<string> lines;
  for (const auto &doc : docs) {
    auto ls = doc.format(width);
    lines.insert(lines.end(), ls.begin(), ls.end());
  }
  return join(lines, '\n');
}

inline std::vector<Document> Parser::documents(int level) {
  std::vector<Document> docs;
  if (!holder_) return docs;
  if (level < holder_->help_level) return docs;
  if (!holder_->description.empty()) {
    docs.push_back(Document().set_message(holder_->description));
  }
  for (const auto &opt : holder_->options) {
    if (level >= opt->help_level) {
      docs.push_back(opt->document);
    }
  }
  for (const auto &priority_parser : holder_->parsers) {
    Parser p;
    p.holder_ = priority_parser.second;
    auto t = p.documents(level + 1);
    docs.insert(docs.end(), t.begin(), t.end());
  }
  if (!holder_->information.empty()) {
    docs.push_back(Document().set_message(holder_->information));
  }
  return docs;
}

inline std::vector<std::pair<MatchResult, std::shared_ptr<Option>>>
Parser::match_results(const PositionArguments &s) const {
  std::vector<std::pair<MatchResult, std::shared_ptr<Option>>> ret;
  if (!holder_ || !holder_->active) return ret;
  Priority cur_pri = std::numeric_limits<Priority>::min();
  for (auto const &pri_parser : holder_->parsers) {
    if (cur_pri > pri_parser.first) continue;
    Parser p;
    p.holder_ = pri_parser.second;
    auto l = p.match_results(s);
    if (!l.size()) continue;
    if (cur_pri == pri_parser.first) {
      for (auto const &mr_opt : l) {
        ret.push_back(mr_opt);
      }
    } else {
      ret.swap(l);
    }
    cur_pri = pri_parser.first;
  }
  for (auto const &opt : holder_->options) {
    if (!opt->active) continue;
    if (opt->priority < cur_pri) continue;
    auto mr = opt->match(s);
    if (!mr.priority) continue;
    if (opt->priority > cur_pri) {
      ret.clear();
    }
    ret.push_back(std::make_pair(mr, opt));
    cur_pri = opt->priority;
  }
  return ret;
}

// A default global parser, to hold options across libraries/objects.
inline Parser &parser() {
  static Parser p;
  return p;
}

}  // namespace options_parser
#endif  // FILE_BE44E7FB_480D_4869_A5D8_0EA9C352CB97_H
