#ifndef FILE_86B02CB4_542A_4720_B778_ECBACC21EAB0_H
#define FILE_86B02CB4_542A_4720_B778_ECBACC21EAB0_H
#include "options_parser/arguments-inc.h"
#include "options_parser/converter-inc.h"

namespace options_parser {

inline PositionValue<char> get_char(const Arguments &args,
                                    const Position &pos) {
  int c = args.char_at(pos.index, pos.off);
  if (c != -1) {
    Position end{pos.index, pos.off + 1};
    return PositionValue<char>((char)c, pos, end);
  }
  return PositionValue<char>(nothing, pos, pos);
}

inline PositionValue<string> get_arg(const Arguments &args,
                                          const Position &pos) {
  if (pos.index >= args.argc()) {
    return PositionValue<string>(nothing, pos, pos);
  }
  auto s = args.arg_at(pos.index);
  if (s.size() >= (size_t)pos.off)
    s = s.substr(pos.off);
  else
    s = "";
  Position end{pos.index + 1, 0};
  return PositionValue<string>(s, pos, end);
}

inline PositionValue<string> get_match_arg(const Arguments &args,
                                           const Position &pos,
                                           const char prefix) {
  Position arg_pos{pos.index, 0};
  auto r = get_arg(args, arg_pos);
  if (!r.value || !r.value.get()->size() || r.value.get()->at(0) != prefix) {
    return PositionValue<string>(nothing, pos, pos);
  }
  if ((int)r.value.get()->size() <= pos.off) {
    return PositionValue<string>(nothing, pos, pos);
  }
  int off = pos.off;
  if (off == 0) {
    string *p = r.value.mutable_get();
    while (off < (int)p->size() && p->at(off) == prefix) off++;
    *p = p->substr(off);
  } else {
    r.start = pos;
    string *p = r.value.mutable_get();
    *p = p->substr(off);
  }
  auto eq = r.value.get()->find('=');
  if (eq < r.value.get()->size()) {
    r.end = pos;
    r.end.off = off + eq + 1;
    *r.value.mutable_get() = r.value.mutable_get()->substr(0, eq);
  }
  return r;
}

}  // namespace options_parser
#endif  // FILE_86B02CB4_542A_4720_B778_ECBACC21EAB0_H
