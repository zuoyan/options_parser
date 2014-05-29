#ifndef FILE_E9EBEEAA_F30F_4CB9_8065_201506C55FA4_H
#define FILE_E9EBEEAA_F30F_4CB9_8065_201506C55FA4_H
#include <iostream>

#include "options_parser/arguments-inc.h"
#include "options_parser/document-inc.h"
#include "options_parser/converter-inc.h"
#include "options_parser/matcher-inc.h"
#include "options_parser/position-inc.h"

namespace options_parser {

template <class Description, class Epilog>
Parser::Parser(const Description &description, const Epilog &epilog) {
  holder_ = std::make_shared<Holder>();
  holder_->description.set_message(description);
  holder_->epilog.set_message(epilog);
}

template <class GetLine>
ParseResult Parser::parse_lines(const GetLine &get_line, Situation s) {
  size_t line_index = 0;
  ParseResult pr;
  pr.situation = s;
  while (true) {
    auto maybe_line = get_line();
    if (!maybe_line) break;
    auto l = *maybe_line.get();
    if (!l.size() || l[0] == '#') {
      line_index += 1;
      continue;
    }
    if (l.back() == '\\') {
      l = l.substr(0, l.size() - 1);
      while (true) {
        auto ml = get_line();
        if (!ml) break;
        auto n = *ml.get();
        if (!n.size()) break;
        if (n.back() != '\\') {
          l += n;
          break;
        }
        l += n.substr(0, n.size() - 1);
      }
    }
    pr = parse_string(l, pr.situation);
    if (pr.error) {
      break;
    }
    ++line_index;
  }
  pr.situation.position.off = pr.situation.position.index;
  pr.situation.position.index = line_index;
  return pr;
}

template <class CM, class CD>
std::shared_ptr<Option> Parser::add_help(const CM &m, const CD &d) {
  auto help_take = [](const MatchResult &mr) {
    TakeResult tr;
    auto v_s = value<int>().optional()(mr.situation);
    tr.error = get_error(v_s.first);
    if (tr.error) {
      std::cerr << "error from help: " << *tr.error.get() << "..." << std::endl;
      exit(1);
      return tr;
    }
    tr.situation = v_s.second;
    tr.error = "help";
    auto self = tr.situation.circumstance.get<Parser>();
    if (!self) {
      std::cerr << "fix me: parser not find in circumstance ..." << std::endl;
      exit(1);
      return tr;
    }
    int level = 0;
    if (get_value(v_s.first)) {
      level = *get_value(v_s.first).get();
    }
    std::cout << self->help_message(level, 78) << std::endl;
    exit(0);
    return tr;
  };
  return add_option(m, help_take, d);
}

template <class T>
std::shared_ptr<Option> define_flag(Parser &parser, const string &flag, T *ptr,
                                    const string &doc) {
  return parser.add_option(
      flag, ptr,
      {"--" + flag + "<arg>", "Current value: " + delay_to_str(ptr) +
                                  (doc.size() ? "\n" + doc : "Set " + flag)});
}

template <class T>
std::shared_ptr<Option> define_flag(const string &flag, T *ptr,
                                    const string &doc) {
  return define_flag(parser(), flag, ptr, doc);
}

}  // namespace options_parser
#endif  // FILE_E9EBEEAA_F30F_4CB9_8065_201506C55FA4_H
