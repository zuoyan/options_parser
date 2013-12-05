#ifndef FILE_B03D9770_5FD4_4607_97CB_E4EA7D3486DB_H
#define FILE_B03D9770_5FD4_4607_97CB_E4EA7D3486DB_H
#include "options_parser/config.h"
#include "options_parser/converter-imp.h"
namespace options_parser {

inline Document::Document() { message_ = false; }

inline string Document::prefix() const {
  if (prefix_.empty()) return string();
  return prefix_;
}

inline string Document::description() const {
  if (description_.empty()) return string();
  return description_;
}

inline std::vector<string> Document::format(size_t width) const {
  std::vector<string> ret;
  if (message_) {
    if (description_.empty()) return ret;
    return format_str((string)description_, width);
  }
  string p, d;
  if (!prefix_.empty()) p = prefix_;
  if (!description_.empty()) d = description_;
  const size_t prefix_width = 18;
  const size_t prefix_sep = 2;
  string left(prefix_width + prefix_sep, ' ');
  string sep(prefix_sep, ' ');
  if (width < prefix_width + prefix_sep + prefix_width) {
    width = prefix_width + prefix_sep + prefix_width;
  }
  auto plines = format_str(p, prefix_width);
  auto dlines = format_str(d, width - prefix_width - prefix_sep);
  if (plines.size() == 0) {
    for (const auto &l : dlines) {
      ret.push_back(p + l);
    }
    return ret;
  }
  if (plines.size() == 1) {
    if (!dlines.size()) return plines;
    ret.push_back(plines.front() + left.substr(plines.front().size()) +
                  dlines.front());
    for (size_t i = 1; i < dlines.size(); ++i) {
      ret.push_back(left + dlines[i]);
    }
    return ret;
  }
  ret = format_str(p, width);
  for (const auto &l : dlines) {
    ret.push_back(left + l);
  }
  return ret;
}

}  // namespace options_parser
#endif  // FILE_B03D9770_5FD4_4607_97CB_E4EA7D3486DB_H
