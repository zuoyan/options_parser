#ifndef FILE_BE44E7FB_480D_4869_A5D8_0EA9C352CB97_H
#define FILE_BE44E7FB_480D_4869_A5D8_0EA9C352CB97_H
#include <limits>

#include "options_parser/config.h"
#include "options_parser/arguments-imp.h"
#include "options_parser/document-imp.h"
#include "options_parser/converter-imp.h"
#include "options_parser/matcher-imp.h"
#include "options_parser/position-imp.h"
#include "options_parser/join.h"

namespace options_parser {

OPTIONS_PARSER_IMP Option *Option::take_at_start() {
  auto t = take;
  take = [t](MatchResult mr) {
    mr.situation.position = mr.start;
    return t(mr);
  };
  return this;
}

OPTIONS_PARSER_IMP Option *Option::hide_from_help() {
  help_level = std::numeric_limits<int>::max();
  return this;
}

OPTIONS_PARSER_IMP Option *Option::set_help_level(int level) {
  help_level = level;
  return this;
}

OPTIONS_PARSER_IMP Option *Option::disable() {
  active = false;
  return this;
}

OPTIONS_PARSER_IMP Option *Option::set_priority(int p) {
  priority = p;
  return this;
}

OPTIONS_PARSER_IMP Option *Option::append_document(Document doc) {
  if (doc.format_) {
    document.append_message(doc.format_);
  } else {
    document.append(doc.right_);
  }
  return this;
}

OPTIONS_PARSER_IMP Option::Option(Matcher m, Taker t, Document d)
    : match(m), take(t), document(d) {
  if (take) {
    active = false;
  }
}

OPTIONS_PARSER_IMP bool ParseResult::check_print(bool all, int code) const {
  if (error) {
    string es = *error.get();
    if (es != "match-none" || all) {
      std::cerr << "parsing arguments got error: " << es << std::endl;
      if (error_full) {
        std::cerr << *error_full.get() << std::endl;
      }
      if (code != -1) exit(code);
    }
    return false;
  }
  return true;
}

OPTIONS_PARSER_IMP Parser::Parser() { holder_ = std::make_shared<Holder>(); }

OPTIONS_PARSER_IMP void Parser::set_description(
    const property<string> &description) {
  holder_->description = description;
}

OPTIONS_PARSER_IMP Document Parser::description() const {
  return holder_->description;
}

OPTIONS_PARSER_IMP void Parser::set_epilog(const property<string> &epilog) {
  holder_->epilog = epilog;
}

OPTIONS_PARSER_IMP Document Parser::epilog() const { return holder_->epilog; }

OPTIONS_PARSER_IMP void Parser::set_help_level(int help_level) {
  holder_->help_level = help_level;
}

OPTIONS_PARSER_IMP int Parser::help_level() const {
  return holder_->help_level;
}

OPTIONS_PARSER_IMP void Parser::hide_from_help() {
  holder_->help_level = std::numeric_limits<int>::max();
}

OPTIONS_PARSER_IMP bool Parser::toggle() {
  holder_->active = !holder_->active;
  return holder_->active;
}

OPTIONS_PARSER_IMP void Parser::disable() { holder_->active = false; }

OPTIONS_PARSER_IMP void Parser::enable() { holder_->active = true; }

OPTIONS_PARSER_IMP std::string show_position(const Situation &s,
                                             size_t limit = 80) {
  string ret = "index=" + to_str(s.position.index);
  if (s.position.off) {
    ret += " off=" + to_str(s.position.off) + ", ";
  }
  int index = s.position.index;
  if (index < s.args.argc()) {
    ret += " argv='" + s.args.arg_at(index) + "'";
  } else {
    if (index == s.args.argc()) {
      ret += " (end of argv)";
    } else {
      ret += " argv-size=" + to_str(s.args.argc());
    }
  }
  while (ret.size() + 6 < limit && ++index < s.args.argc()) {
    auto a = s.args.arg_at(index);
    if (ret.size() + a.size() + 2 >= limit) {
      a.resize(limit - 5 - ret.size());
      a += " ...";
    }
    ret += " '" + a + "'";
  }
  return ret;
}

OPTIONS_PARSER_IMP ParseResult Parser::parse(const Situation &s) {
  Situation c = s;
  if (!c.parser) c.parser = this;
  ParseResult pr;

  while (c.position.index < c.args.argc()) {
    auto mr_opts = match_results(c);
    if (!mr_opts.size()) {
      pr.situation = c;
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
      pr.situation = c;
      std::vector<string> lines;
      for (auto it = mr_opts.begin(); it != last; ++it) {
        auto &doc = it->second->document;
        auto ls = doc.format(78);
        if (lines.size()) lines.push_back("");
        lines.insert(lines.end(), ls.begin(), ls.end());
      }
      pr.error = "match-multiple";
      pr.error_full = string("match multiple at ") + show_position(c) +
                      " with following options:\n" + join(lines, "\n");
      return pr;
    }
    auto const &mr_opt = mr_opts.front();
    // std::cerr << "match end at " << show_position(mr_opt.first.situation)
    //           << std::endl;
    auto tr = mr_opt.second->take(mr_opt.first);
    // std::cerr << "tr end at " << show_position(tr.situation) << std::endl;
    if (tr.error) {
      pr.situation = c;
      pr.error = "take-error";
      pr.error_full = *tr.error.get() + "\ncurrent argument " +
                      show_position(c) + "\nwith matched option:\n" +
                      join(mr_opt.second->document.format(78), "\n");
      return pr;
    }
    c = tr.situation;
    c.option = nullptr;
    c.circumstance = c.circumstance.parent();
  }
  pr.situation = c;
  return pr;
}

OPTIONS_PARSER_IMP ParseResult
Parser::parse_string(const string &a, Situation s) {
  VectorStringArguments args(expand(a));
  s.args = args;
  s.position.off = 0;
  s.position.index = 0;
  return parse(s);
}

OPTIONS_PARSER_IMP ParseResult
Parser::parse_lines(const std::vector<string> &lines, Situation s) {
  size_t off = 0;
  return parse_lines([&]() -> Maybe<string> {
                       if (off < lines.size()) return lines[off++];
                       return nothing;
                     },
                     s);
}

OPTIONS_PARSER_IMP Value<string> config_file(const string &filename) {
  return [filename](Situation s) -> std::pair<Either<string>, Situation> {
    auto cli = s.parser;
    assert(cli);
    auto pr = cli->parse_file(filename, s);
    string error;
    if (pr.error) {
      error = get_value(pr.error_full ? pr.error_full : pr.error);
    }
    if (error.size()) {
      return std::make_pair(
          error_message("when parsing config file '" + filename + "':" + error),
          s);
    }
    return std::make_pair(filename, s);
  };
}

OPTIONS_PARSER_IMP ParseResult
Parser::parse_file(const string &fn, Situation s) {
  std::ifstream ifs(fn);
  ParseResult pr;
  pr.situation = s;
  if (!ifs.good()) {
    pr.error = "open-failed";
    pr.error_full = "open file '" + fn + "' to read failed: " + strerror(errno);
    return pr;
  }
  auto get_line = [&]() -> Maybe<string> {
    if (!ifs.good()) return nothing;
    string l;
    std::getline(ifs, l);
    return l;
  };
  pr = parse_lines(get_line, pr.situation);
  if (pr.error) {
    auto p = "parse file '" + fn + "' failed at line " +
             to_str(pr.situation.position.index) + " field " +
             to_str(pr.situation.position.off);
    if (pr.error_full) {
      pr.error_full = p + "\n" + *pr.error_full.get();
    } else {
      pr.error_full = p;
    }
  }
  return pr;
}

OPTIONS_PARSER_IMP void Parser::add_parser(const Parser &parser, int priority) {
  holder_->parsers.push_back(std::make_pair(priority, parser.holder_));
}

OPTIONS_PARSER_IMP std::shared_ptr<Option> Parser::add_option(const Option &o) {
  holder_->options.push_back(std::make_shared<Option>(o));
  return holder_->options.back();
}

OPTIONS_PARSER_IMP std::shared_ptr<Option> Parser::add_option(
    const Matcher &m, const Taker &t, const Document &d) {
  Option opt;
  opt.match = m;
  opt.take = t;
  opt.document = d;
  opt.help_level = holder_ ? holder_->help_level : 0;
  return add_option(opt);
}

OPTIONS_PARSER_IMP std::vector<std::shared_ptr<Option>> Parser::add_flags_file(
    const string &fn, size_t doc_indent) {
  std::ifstream ifs(fn);
  auto get_line = [&]() -> Maybe<string> {
    string line;
    std::getline(ifs, line);
    if (!ifs.fail()) {
      return line;
    }
    return nothing;
  };
  return add_flags_lines(get_line, doc_indent);
}

OPTIONS_PARSER_IMP std::vector<std::shared_ptr<Option>> Parser::add_flags_lines(
    const std::vector<string> &lines, size_t doc_indent) {
  size_t off = 0;
  auto get_line = [&]() -> Maybe<string> {
    if (off < lines.size()) return lines[off++];
    return nothing;
  };
  return add_flags_lines(get_line, doc_indent);
}

OPTIONS_PARSER_IMP string Parser::help_message(int level, int width) const {
  auto docs = documents(level);
  std::vector<string> lines;
  for (const auto &doc : docs) {
    auto ls = doc.format(width);
    lines.insert(lines.end(), ls.begin(), ls.end());
  }
  return join(lines, "\n");
}

OPTIONS_PARSER_IMP std::vector<Document> Parser::documents(int level) const {
  std::vector<Document> docs;
  if (!holder_) return docs;
  if (level < holder_->help_level) return docs;
  docs.push_back(holder_->description);
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
  docs.push_back(holder_->epilog);
  return docs;
}

OPTIONS_PARSER_IMP std::vector<std::pair<MatchResult, std::shared_ptr<Option>>>
Parser::match_results(const Situation &s) const {
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
    auto os = s;
    os.option = opt.get();
    os.circumstance = os.circumstance.new_child();
    auto mr = opt->match(os);
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
OPTIONS_PARSER_IMP Parser &parser() {
  static Parser p;
  return p;
}

}  // namespace options_parser
#endif  // FILE_BE44E7FB_480D_4869_A5D8_0EA9C352CB97_H
