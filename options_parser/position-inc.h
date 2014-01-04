#ifndef FILE_FD129630_4D45_4D18_A7A8_D51D3445F5A4_H
#define FILE_FD129630_4D45_4D18_A7A8_D51D3445F5A4_H
#include "options_parser/arguments-inc.h"
#include "options_parser/converter-inc.h"

namespace options_parser {

template <class T>
template <class U>
PositionValue<T>::PositionValue(const PositionValue<U> &o) {
  if (o.value) {
    value = o.value;
  }
  start = o.start;
  end = o.end;
}

template <class T>
PositionValue<T>::PositionValue(const Maybe<T> &v, const Position &s,
                                const Position &e)
    : value(v), start(s), end(e) {}

}  // namespace options_parser
#endif  // FILE_FD129630_4D45_4D18_A7A8_D51D3445F5A4_H
