#ifndef FILE_20A5A886_FEFC_4145_828E_64203B134265_H
#define FILE_20A5A886_FEFC_4145_828E_64203B134265_H
#include <algorithm>
#include <limits>
#include <memory>
#include <set>
#include <fstream>

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

struct Option {
  bool active;
  int help_level;
  Matcher match;
  Taker take;
  Document document;
  int priority;

  Option();
  Option(const Option &) = default;

  Option(Matcher m, Taker t, Document d);
};

Taker bundle(const std::vector<Option> &options);

struct ParseResult {
  Situation situation;
  Maybe<string> error;
  Maybe<string> error_full;
};

struct Parser {
  template <class Description, class Epilog>
  Parser(const Description &description, const Epilog &epilog);

  Parser();

  Parser(const Parser &) = default;

  // description is print before options
  void set_description(const property<string> &description);

  // epilog is print after options
  void set_epilog(const property<string> &epilog);

  bool toggle();
  void disable();
  void enable();

  ParseResult parse(const Situation &s);

  inline ParseResult parse(int argc, char *argv[]) {
    Situation s;
    s.args = ArgvArguments(argc, argv);
    s.position = Position(1, 0);
    if (holder_) {
      holder_->circumstance.check_init();
      s.circumstance = holder_->circumstance;
    }
    return parse(s);
  }

  inline ParseResult parse(const std::vector<string> argv, size_t off = 1) {
    Situation s;
    s.args = VectorStringArguments(argv);
    s.position = Position(off, 0);
    if (holder_) {
      holder_->circumstance.check_init();
      s.circumstance = holder_->circumstance;
    }
    return parse(s);
  }

  ParseResult parse_string(const string &a);

  template <class GetLine>
  size_t parse_lines(const GetLine &get_line, Maybe<string> *error,
                     Maybe<string> *error_full);

  size_t parse_lines(const std::vector<string> &lines,
                     Maybe<string> *error, Maybe<string> *error_full);

  void parse_file(const string &fn, Maybe<string> *error,
                  Maybe<string> *error_full);

  void add_parser(const Parser &parser, int priority = 0);

  std::shared_ptr<Option> add_option(const Option &o);

  std::shared_ptr<Option> add_option(const Matcher &m, const Taker &t,
                                     const Document &d);

  template <
      class CM, class CD,
      typename std::enable_if<std::is_constructible<string, CM>::value &&
                                  std::is_constructible<string, CD>::value,
                              int>::type = 0>
  std::shared_ptr<Option> add_option(const CM &m, const Taker &t, const CD &d) {
    std::vector<string> opts = split(m, "|");
    bool is_raw = (opts.front().size() && opts.front()[0] == '-');
    std::string pattern;
    if (opts.back().find("=") < opts.back().size()) {
      auto &b = opts.back();
      pattern = b.substr(b.find('='), 0);
      b = b.substr(0, b.find('='));
    }
    auto opts_prefix = opts;
    if (!is_raw) {
      for (auto & o : opts_prefix) {
        o = (o.size() == 1 ? "-" : "--") + o;
      }
    }
    return add_option(Matcher(opts), t, {join(opts_prefix, ", ") + pattern, d});
  }

  template <class CM = Matcher, class CD = Document>
  std::shared_ptr<Option> add_help(const CM &m = CM{"h|help"},
                                   const CD &d = "show help message");

  template <class T, class CD>
  std::shared_ptr<Option> add_flag(const string &opts, const CD &doc) {
    std::string flag_name = opts;
    if (flag_name.find('|') < flag_name.size()) {
      flag_name = flag_name.substr(flag_name.rfind('|'));
    }
    if (flag_name.find('=') < flag_name.size()) {
      flag_name = flag_name.substr(0, flag_name.find('='));
    }
    auto taker = [flag_name](const MatchResult &mr) {
      TakeResult tr;
      auto v_s = value<T>()(mr.situation);
      tr.error = get_error(v_s.first);
      tr.situation = v_s.second;
      if (!tr.error) {
        T *ptr = tr.situation.circumstance.flag<T>(flag_name);
        *ptr = get_value(v_s.first);
      }
      return tr;
    };
    return this->add_option<string, CD>(opts, taker, doc);
  }

  template <class T, class CD>
  std::shared_ptr<Option> add_optional_flag(const string &opts,
                                            const T &default_value,
                                            const CD &doc) {
    std::string flag_name = opts;
    if (flag_name.find('|') < flag_name.size()) {
      flag_name = flag_name.substr(flag_name.rfind('|'));
    }
    if (flag_name.find('=') < flag_name.size()) {
      flag_name = flag_name.substr(0, flag_name.find('='));
    }
    auto taker = [flag_name, default_value](const MatchResult &mr) {
      TakeResult tr;
      auto v_s = optional_value<T>()(mr.situation);
      tr.situation = v_s.second;
      T *ptr =
      tr.situation.circumstance.flag<T>(flag_name);
      if (get_value(v_s.first)) {
        *ptr = get_value(get_value(v_s.first));
      } else {
        *ptr = default_value;
      }
      return tr;
    };
    return this->add_option<string, CD>(opts, taker, doc);
  }

  std::shared_ptr<Option> add_flag(const string &names, const Document &doc) {
    return add_optional_flag(names, true, doc);
  }

  string help_message(int level, int width);

  void set_circumstance(const Circumstance &circumstance) {
    holder_->circumstance = circumstance;
  }

 private:
  std::vector<Document> documents(int level);
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
    // The outest circumstance(the one in parser who called parse) is passed to
    // every matcher and taker.
    Circumstance circumstance;
    Holder() {
      active = true;
      help_level = 0;
    }
  };
  std::shared_ptr<Holder> holder_;
};

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
