#ifndef FILE_20A5A886_FEFC_4145_828E_64203B134265_H
#define FILE_20A5A886_FEFC_4145_828E_64203B134265_H
#include <algorithm>
#include <limits>
#include <memory>
#include <set>
#include <fstream>

#include "options_parser/arguments.h"
#include "options_parser/document.h"
#include "options_parser/position.h"
#include "options_parser/matcher.h"
#include "options_parser/taker.h"
#include "options_parser/converter.h"
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
  Arguments *args;
  Maybe<std::string> error;
  Maybe<std::string> error_full;
};

struct Parser {
  template <class Description, class Information>
  Parser(const Description &description, const Information &information);

  Parser() = default;

  Parser(const Parser &) = default;

  bool toggle();
  void disable();
  void enable();

  ParseResult parse(const PositionArguments &s);

  ParseResult parse_string(const std::string &a);

  template <class GetLine>
  size_t parse_lines(const GetLine &get_line, Maybe<std::string> *error,
                     Maybe<std::string> *error_full);

  size_t parse_lines(const std::vector<std::string> &lines,
                     Maybe<std::string> *error, Maybe<std::string> *error_full);

  void parse_file(const std::string &fn, Maybe<std::string> *error,
                  Maybe<std::string> *error_full);

  void add_parser(const Parser &parser, int priority = 0);

  Option *add_option(const Option &o);

  Option *add_option(const Matcher &m, const Taker &t, const Document &d);

  template <class CM = Matcher, class CD = Document>
  Option *add_help(const CM &m = CM{"h|help"},
                   const CD &d = CD{"-h, --help", "show help message"});

  std::string help_message(int level, int width);

 private:
  std::vector<Document> documents(int level);
  std::vector<std::pair<MatchResult, std::shared_ptr<Option>>> match_results(
      const PositionArguments &s) const;

  // To make parser works in value semantic
  struct Holder {
    bool active;
    int help_level;
    // description before options
    property<std::string> description;
    // information after options
    property<std::string> information;
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
inline Parser &parser();

template <class T>
Option *define_flag(Parser &parser, const std::string &flag, T *ptr,
                    const std::string &doc = "");

template <class T>
Option *define_flag(const std::string &flag, T *ptr,
                    const std::string &doc = "");

#define OPTIONS_PARSER_FLAGS_DECLARE(TYPE, NAME) extern TYPE FLAGS_##NAME

#define OPTIONS_PARSER_FLAGS_DEFINE(TYPE, NAME, VALUE, DOC)                  \
  TYPE FLAGS_##NAME = VALUE;                                                 \
  options_parser::Option *FLAGS_OPTIONS__##NAME =                            \
      options_parser::parser().add_option("--" #NAME, &FLAGS_##NAME,         \
                                          {"--" #NAME "=<" #TYPE ">",        \
                                           "Current value: " +               \
                                               options_parser::delay_to_str( \
                                                   &FLAGS_##NAME) +          \
                                               "\n" + DOC})

}  // namespace options_parser
#endif  // FILE_20A5A886_FEFC_4145_828E_64203B134265_H