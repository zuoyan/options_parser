#ifndef FILE_02522C8F_5040_40F1_ACBE_AEFC05B4514B_H
#define FILE_02522C8F_5040_40F1_ACBE_AEFC05B4514B_H
#include <algorithm>
#include <limits>
#include <memory>
#include <set>

#include "options_parser/arguments.h"
#include "options_parser/document.h"
#include "options_parser/position.h"
#include "options_parser/matcher.h"
#include "options_parser/taker.h"
#include "options_parser/converter.h"
#include "options_parser/property.h"

namespace options_parser {

struct Option {
  bool active;
  int help_level;
  Matcher match;
  Taker take;
  Document document;
  int priority;

  Option() {
    active = true;
    help_level = 0;
    priority = 0;
  }

  Option(const Option &) = default;

  Option(Matcher m, Taker t, Document d) : match(m), take(t), document(d) {
    active = true;
    help_level = 0;
    priority = 0;
  }
};

Taker bundle(const std::vector<Option> &options) {
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

struct ParseResult {
  Position position;
  Arguments *args;
  Maybe<std::string> error;
  Maybe<std::string> error_full;
};

struct Parser {
  template <class Description, class Information>
  Parser(const Description &description, const Information &information) {
    holder_ = std::make_shared<Holder>();
    holder_->description = description;
    holder_->information = information;
  }

  Parser() = default;

  Parser(const Parser &) = default;

  bool toggle() {
    if (!holder_) {
      holder_ = std::make_shared<Holder>();
    }
    holder_->active = !holder_->active;
    return holder_->active;
  }

  void disable() {
    if (toggle()) toggle();
  }

  void enable() {
    if (!toggle()) toggle();
  }

  ParseResult parse(const PositionArguments &s) {
    PositionArguments c = s;
    ParseResult pr;
    while (c.position.index < c.args->argc()) {
      auto mr_opts = match_results(c);
      auto show_position = [](PositionArguments pa, size_t limit = 80) {
        std::string ret = to_str(pa.position.index);
        if (pa.position.off) {
          ret += "off=" + to_str(pa.position.off) + ", ";
        }
        auto p = pa.position;
        p.off = 0;
        if (p.index < pa.args->argc()) {
          ret += " '" + *get_arg(pa.args, p).value.get() + "'";
        } else {
          ret += " over size=" + to_str(pa.args->argc());
        }
        while (ret.size() + 6 < limit && ++p.index < pa.args->argc()) {
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
        pr.error_full = std::string("match nothing at ") + show_position(c);
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
        std::vector<std::string> lines;
        for (auto it = mr_opts.begin(); it != last; ++it) {
          auto &doc = it->second->document;
          auto ls = doc.format(78);
          if (lines.size()) lines.push_back("");
          lines.insert(lines.end(), ls.begin(), ls.end());
        }
        pr.error = "match-multiple";
        pr.error_full = std::string("match multiple at ") + show_position(c) +
                        " with following options:\n" +
                        Document::join(lines, '\n');
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
        pr.error_full =
            std::string("process failed: ") + *tr.error.get() + "\n" + "at " +
            show_position(c) + "\n" + "matched option:\n" +
            Document::join(mr_opt.second->document.format(78), '\n');
        return pr;
      }
      c.position = tr.end;
      c.args = tr.args;
    }
    pr.position = c.position;
    pr.args = c.args;
    return pr;
  }

  void add_parser(const Parser &parser, Priority priority = 0) {
    if (!holder_) {
      holder_ = std::make_shared<Holder>();
    }
    holder_->parsers.push_back(std::make_pair(priority, parser.holder_));
  }

  Option *add_option(const Option &o) {
    if (!holder_) {
      holder_ = std::make_shared<Holder>();
    }
    holder_->options.push_back(std::make_shared<Option>(o));
    return holder_->options.back().get();
  }

  Option *add_option(const Matcher &m, const Taker &t, const Document &d) {
    Option opt;
    opt.match = m;
    opt.take = t;
    opt.document = d;
    opt.help_level = holder_ ? holder_->help_level : 0;
    return add_option(opt);
  }

  template <class CM = Matcher, class CD = Document>
  Option *add_help(const CM &m = CM{"h|help"},
                   const CD &d = CD{"-h, --help", "show help message"}) {
    auto help_take = [this](const MatchResult &) {
      std::cout << help_message(0, 78) << std::endl;
      exit(1);
      TakeResult tr;
      return tr;
    };
    return add_option(m, help_take, d);
  }

  std::string help_message(int level, int width) {
    auto docs = documents(level);
    std::vector<std::string> lines;
    for (const auto &doc : docs) {
      auto ls = doc.format(width);
      lines.insert(lines.end(), ls.begin(), ls.end());
    }
    return Document::join(lines, '\n');
  }

 private:
  std::vector<Document> documents(int level) {
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

  template <class CM, class CD, class CP>
  typename std::enable_if<std::is_same<CD, Document>::value, Document>::type
  generate_document(const CM &m, const CD &d, CP prefix) {
    d.set_prefix(prefix);
    return d;
  }

  template <class CM, class CD>
  typename std::enable_if<std::is_convertible<CM, std::string>::value &&
                              !std::is_same<CD, Document>::value,
                          Document>::type
  generate_document(const CM &m, const CD &d,
                    const Maybe<std::string> &prefix) {
    Document doc(prefix, d);
    if (!prefix) {
      doc.set_prefix("--" + std::string(m));
    }
    return doc;
  }

  template <class CM, class CD>
  typename std::enable_if<!std::is_convertible<CM, std::string>::value &&
                              !std::is_same<CD, Document>::value,
                          Document>::type
  generate_document(const CM &m, const CD &d,
                    const Maybe<std::string> &prefix) {
    Document doc(prefix, d);
    return doc;
  }

  std::vector<std::pair<MatchResult, std::shared_ptr<Option>>> match_results(
      const PositionArguments &s) const {
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

}  // namespace options_parser
#endif  // FILE_02522C8F_5040_40F1_ACBE_AEFC05B4514B_H
