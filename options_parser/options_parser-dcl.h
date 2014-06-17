#ifndef FILE_20A5A886_FEFC_4145_828E_64203B134265_H
#define FILE_20A5A886_FEFC_4145_828E_64203B134265_H
#include <algorithm>
#include <limits>
#include <memory>
#include <set>
#include <fstream>
#include <iostream>

#include "options_parser/arguments-dcl.h"
#include "options_parser/document-dcl.h"
#include "options_parser/position-dcl.h"
#include "options_parser/matcher-dcl.h"
#include "options_parser/taker.h"
#include "options_parser/converter-dcl.h"
#include "options_parser/property.h"
#include "options_parser/expand.h"
#include "options_parser/circumstance.h"

namespace options_parser {

class Option {
 public:
  Matcher match;
  Taker take;
  Document document;
  Circumstance circumstance;
  bool active = true;
  int help_level = 0;
  int priority = 0;

  Option() = default;
  Option(const Option &) = default;

  Option(Matcher m, Taker t, Document d);

  Option *take_at_start();
  Option *hide_from_help();
  Option *set_help_level(int help_level);
  Option *disable();
  Option *set_priority(int priority);
  Option *append_document(Document doc);

  template <class T>
  Option *append_value_document(T *ptr);
};

class ParseResult {
 public:
  Situation situation;
  Maybe<string> error;
  Maybe<string> error_full;

  bool check_print(bool all = true, int code = 1) const;
};

class Parser {
 public:
  template <class Description, class Epilog>
  Parser(const Description &description, const Epilog &epilog);

  Parser();

  Parser(const Parser &) = default;

  void set_description(const property<string> &description);
  Document description() const;

  void set_epilog(const property<string> &epilog);
  Document epilog() const;

  void set_help_level(int help_level);
  int help_level() const;
  void hide_from_help();

  bool toggle();
  void disable();
  void enable();

  ParseResult parse(const Situation &s);

  inline ParseResult parse(int argc, char *argv[], Situation s = Situation()) {
    s.args = ArgvArguments(argc, argv);
    s.position = Position(1, 0);
    return parse(s);
  }

  inline ParseResult parse(const std::vector<string> &argv, size_t off = 1,
                           Situation s = Situation()) {
    s.args = VectorStringArguments(argv);
    s.position = Position(off, 0);
    return parse(s);
  }

  ParseResult parse_string(const string &a, Situation s = Situation());

  template <class GetLine>
  ParseResult parse_lines(const GetLine &get_line, Situation s = Situation());

  ParseResult parse_lines(const std::vector<string> &lines,
                          Situation s = Situation());

  ParseResult parse_file(const string &fn, Situation s = Situation());

  void add_parser(const Parser &parser, int priority = 0);

  std::shared_ptr<Option> add_option(const Option &o);

  std::shared_ptr<Option> add_option(const Matcher &m, const Taker &t,
                                     const Document &d);

  template <class CM, class T, class CD,
            typename std::enable_if<!std::is_function<T>::value, int>::type = 0>
  std::shared_ptr<Option> add_option(const CM &m, T* ptr, const CD &d) {
    auto ret = add_option(m, Taker{ptr}, d);
    ret->append_value_document(ptr);
    return ret;
  }

  template <class CM, class CD,
            typename std::enable_if<
                std::is_constructible<string, const CM &>::value &&
                    std::is_constructible<string, const CD &>::value,
                int>::type = 0>
  std::shared_ptr<Option> add_option(const CM &m, const Taker &t, const CD &d) {
    MatchDescription md(m);
    return add_option(Matcher(md), t, {md.doc, d});
  }

  template <class CM = string, class CD = string>
  std::shared_ptr<Option> add_help(const CM &m = "-h, --help[=level]",
                                   const CD &d = "show help message");

  template <class T, class CO, class CD>
  std::shared_ptr<Option> add_flag(const CO &opts, const CD &doc,
                                   const T &default_value = T()) {
    MatchDescription md(opts);
    auto taker = [md, default_value](const MatchResult &mr) {
      TakeResult tr;
      if (md.num_args > 1) {
        tr.error = "invalid flag " + md.name;
        return tr;
      }
      if (md.num_args == 1) {
        if (md.is_arg_optional) {
          auto v_s = value<T>().optional()(mr.situation);
          tr.situation = v_s.second;
          T *ptr = tr.situation.circumstance.flag<T>(md.name, default_value);
          if (get_value(v_s.first)) {
            *ptr = get_value(get_value(v_s.first));
          } else {
            *ptr = default_value;
          }
        } else {
          auto v_s = value<T>()(mr.situation);
          tr.situation = v_s.second;
          if (is_error(v_s.first)) {
            tr.error = get_error(v_s.first);
          }
          if (!tr.error) {
            tr.situation.circumstance.flag_set(md.name, get_value(v_s.first));
          }
        }
      }
      if (md.num_args == 0) {
        tr.situation = mr.situation;
        tr.situation.circumstance.flag_set(md.name,
                                           default_value);
      }
      return tr;
    };
    return add_option(opts, taker, doc);
  }

  template <class CD>
  std::shared_ptr<Option> add_flag(const string &opts, const CD &doc) {
    MatchDescription md(opts);
    if (md.num_args == 0) {
      return add_flag(opts, doc, true);
    }
    return add_flag<string>(opts, doc, string());
  }

  std::shared_ptr<Option> add_flag(const string &opts, const Document &doc) {
    MatchDescription md(opts);
    if (md.num_args == 0) {
      return add_flag(opts, doc, true);
    }
    return add_flag<string>(opts, doc, string());
  }

  template <class GetLine>
  std::vector<std::shared_ptr<Option>> add_flags_lines(const GetLine &get_line,
                                                       size_t doc_indent = 8) {
    std::vector<std::shared_ptr<Option>> ret;
    std::string next_line;
    bool is_end = false;
    auto get_flag_line = [&]() {
      Maybe<std::string> may_next;
      std::string line;
      std::swap(line, next_line);
      while (!is_end) {
        may_next = get_line();
        is_end = !may_next;
        if (is_end) break;
        next_line = *may_next.get();
        if (doc_indent && starts_with(next_line, string(doc_indent, ' '))) {
          line += next_line;
          next_line = "";
        } else {
          break;
        }
      }
      return line;
    };
    while (true) {
      string line = get_flag_line();
      size_t off = 0;
      while (off < line.size() && isspace(line[off])) ++off;
      if (off == line.size() && is_end) break;
      if (off == line.size()) continue;
      string doc, rest;
      if (line.find("  ", off) >= line.size()) {
        while (off < line.size()) {
          if (line[off] == '-') {
            while (off < line.size() && !isspace(line[off])) ++off;
            continue;
          }
          if (line[off] == '<') {
            while (off < line.size() && line[off] != '>') ++off;
            if (off < line.size()) ++off;
            continue;
          }
          if (isspace(line[off])) {
            size_t n = off;
            while (n < line.size() && isspace(line[n])) ++n;
            if (n < line.size() && isupper(line[n]) &&
                (n + 1 == line.size() || !islower(line[n]))) {
              off = n;
              while (off < line.size() && !isspace(line[off])) ++off;
              continue;
            }
            off = n;
            break;
          }
        }
        doc = line.substr(0, off);
        rest = line.substr(off);
      } else {
        off = line.find("  ", off);
        doc = line.substr(0, off);
        rest = line.substr(off + 2);
      }
      rest = strip(rest);
      ret.push_back(add_flag(doc, rest));
    }
    return ret;
  }

  std::vector<std::shared_ptr<Option>> add_flags_lines(
      const std::vector<string> &lines, size_t doc_indent = 8);

  std::vector<std::shared_ptr<Option>> add_flags_file(const string &fn,
                                                      size_t doc_indent = 8);

  string help_message(int level, int width) const;
  std::vector<Document> documents(int level) const;

 private:
  std::vector<std::pair<MatchResult, std::shared_ptr<Option>>> match_results(
      const Situation &s) const;

  // To make parser works in value semantic
  struct Holder {
    bool active;
    int help_level;
    // description before options
    Document description;
    // epilog after options
    Document epilog;
    std::vector<std::shared_ptr<Option>> options;
    std::vector<std::pair<int, std::shared_ptr<Holder>>> parsers;
    Holder() {
      active = true;
      help_level = 0;
    }
  };
  std::shared_ptr<Holder> holder_;
};

Value<std::string> config_file();

// A default global parser, to hold options across libraries/objects.
Parser &parser();

template <class T>
std::shared_ptr<Option> define_flag(Parser &parser, const string &flag, T *ptr,
                                    const string &doc = "");

template <class T>
std::shared_ptr<Option> define_flag(const string &flag, T *ptr,
                                    const string &doc = "");

#define OPTIONS_PARSER_FLAGS_DECLARE(TYPE, NAME) extern TYPE FLAGS_##NAME

#define OPTIONS_PARSER_FLAGS_DEFINE(TYPE, NAME, VALUE, DOC)                  \
  TYPE FLAGS_##NAME = VALUE;                                                 \
  auto FLAGS_OPTIONS__##NAME =                                               \
      options_parser::parser().add_option("--" #NAME, &FLAGS_##NAME,         \
                                          {"--" #NAME "=<" #TYPE ">",        \
                                           "Current value: " +               \
                                               options_parser::delay_to_str( \
                                                   &FLAGS_##NAME) +          \
                                               "\n" + DOC})
}  // namespace options_parser
#endif  // FILE_20A5A886_FEFC_4145_828E_64203B134265_H
