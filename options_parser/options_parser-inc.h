#ifndef FILE_E9EBEEAA_F30F_4CB9_8065_201506C55FA4_H
#define FILE_E9EBEEAA_F30F_4CB9_8065_201506C55FA4_H
namespace options_parser {

template <class Description, class Information>
Parser::Parser(const Description &description, const Information &information) {
  holder_ = std::make_shared<Holder>();
  holder_->description = description;
  holder_->information = information;
}

template <class GetLine>
size_t Parser::parse_lines(const GetLine &get_line, Maybe<std::string> *error,
                           Maybe<std::string> *error_full) {
  size_t off = 0;
  while (true) {
    auto maybe_line = get_line();
    if (!maybe_line) break;
    auto l = *maybe_line.get();
    if (!l.size() || l[0] == '#') {
      off += 1;
      continue;
    }
    size_t start = off++;
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
    auto pr = parse_string(l);
    if (pr.error) {
      *error = pr.error;
      *error_full = pr.error_full;
      return start;
    }
  }
  return off;
}

template <class CM, class CD>
Option *Parser::add_help(const CM &m, const CD &d) {
  auto help_take = [this](const MatchResult &) {
    std::cout << help_message(0, 78) << std::endl;
    exit(1);
    TakeResult tr;
    return tr;
  };
  return add_option(m, help_take, d);
}

template <class T>
Option *define_flag(Parser &parser, const std::string &flag, T *ptr,
                    const std::string &doc) {
  return parser.add_option(
      flag, ptr,
      {"--" + flag + "<arg>", "Current value: " + delay_to_str(ptr) +
                                  (doc.size() ? "\n" + doc : "Set " + flag)});
}

template <class T>
Option *define_flag(const std::string &flag, T *ptr, const std::string &doc) {
  return define_flag(parser(), flag, ptr, doc);
}

}  // namespace options_parser
#endif  // FILE_E9EBEEAA_F30F_4CB9_8065_201506C55FA4_H
