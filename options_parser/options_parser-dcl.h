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
  Position position;
  Arguments args;
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

  ParseResult parse(const PositionArguments &s);

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

  std::shared_ptr<Option> add_option(const Matcher &m, const Taker &t, const Document &d);

  template <class CM = Matcher, class CD = Document>
  std::shared_ptr<Option> add_help(const CM &m = CM{"h|help"},
                                   const CD &d =
                                       CD{"-h, --help", "show help message"});

  string help_message(int level, int width);

 private:
  std::vector<Document> documents(int level);
  std::vector<std::pair<MatchResult, std::shared_ptr<Option>>> match_results(
      const PositionArguments &s) const;

  // To make parser works in value semantic
  struct Holder {
    bool active;
    int help_level;
    // description before options
    property<string> description;
    // epilog after options
    property<string> epilog;
    std::vector<std::shared_ptr<Option>> options;
    std::vector<std::pair<int, std::shared_ptr<Holder>>> parsers;

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
