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

  template <class CM, class CD,
            typename std::enable_if<
                std::is_constructible<string, const CM &>::value &&
                    std::is_constructible<string, const CD &>::value,
                int>::type = 0>
  std::shared_ptr<Option> add_option(const CM &m, const Taker &t, const CD &d) {
    MatchFromDescription mfd(m);
    return add_option(Matcher(mfd), t, {mfd.doc, d});
  }

  template <class CM = string, class CD = string>
  std::shared_ptr<Option> add_help(const CM &m = "-h, --help",
                                   const CD &d = "show help message");

  template <class T, class CO, class CD>
  std::shared_ptr<Option> add_flag(const CO &opts, const CD &doc,
                                   const T &default_value = T()) {
    MatchFromDescription mfd(opts);
    auto taker = [mfd, default_value](const MatchResult &mr) {
      TakeResult tr;
      if (mfd.num_args > 1) {
        tr.error = "invalid flag " + mfd.name;
        return tr;
      }
      if (mfd.num_args == 1) {
        if (mfd.is_arg_optional) {
          auto v_s = optional_value<T>()(mr.situation);
          tr.situation = v_s.second;
          T *ptr = tr.situation.circumstance.flag<T>(mfd.name);
          if (get_value(v_s.first)) {
            *ptr = get_value(get_value(v_s.first));
          } else {
            *ptr = default_value;
          }
        } else {
          auto v_s = value<T>()(mr.situation);
          tr.situation = v_s.second;
          tr.error = get_error(v_s.first);
          if (!tr.error) {
            T *ptr = tr.situation.circumstance.flag<T>(mfd.name);
            *ptr = get_value(v_s.first);
          }
        }
      }
      if (mfd.num_args == 0) {
        tr.situation = mr.situation;
        T *ptr = tr.situation.circumstance.flag<T>(mfd.name);
        *ptr = default_value;
      }
      return tr;
    };
    return add_option(opts, taker, doc);
  }

  template <class CD>
  std::shared_ptr<Option> add_flag(const string &opts, const CD &doc) {
    return add_flag(opts, doc, true);
  }

  std::shared_ptr<Option> add_flag(const string &opts, const Document &doc) {
    return add_flag(opts, doc, true);
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
