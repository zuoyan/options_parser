#include <iostream>
#include <cassert>
#include <cstdlib>

#include "options_parser/options_parser_lib.h"

using namespace options_parser;

formatter columns(const std::vector<formatter>& cols,
                  std::function<string(size_t)>& sep) {
  assert(cols.size());
  if (cols.size() == 1) {
    return cols.front();
  }
  std::vector<string> seps;
  for (size_t i = 0; i + 1 < cols.size(); ++i) {
    seps.push_back(sep(i));
  }
  auto func = [ cols, seps ](size_t width)->std::vector<string> {
    size_t nc = cols.size();
    std::vector<string> ret;
    std::vector<size_t> cws(nc, 1);
    std::vector<std::pair<size_t, size_t>> cwrs(nc, 1);
    std::vector<std::pair<double, size_t>> areas(nc);
    size_t sep_width = 0;
    std::map<size_t, std::map<size_t, size_t>> index_width_to_num_lines;
    auto col_lines = [&](size_t c, size_t w) {
      auto& dc = index_width_to_num_lines[c];
      if (dc.count(w)) {
        return dc[w];
      }
      auto& d = dc[w];
      d = cols[c](w).size();
      areas[c].first += w * d;
      ++areas[c].second;
      return d;
    };
    auto col_area = [&](size_t c) { return areas[c].first / areas[c].second; };
    auto lines = [&]() {
      size_t m = 0;
      for (size_t c = 0; c < nc; ++c) {
        auto l = col_lines(c, cws[c]);
        if (l < m) m = l;
      }
      return m;
    };
    for (const auto & sep : seps) {
      sep_width += sep.size();
    }
    if (width > sep_width + nc) {
      for (size_t i = 0; i < nc; ++i) {
        cws[i] += (width - sep_width - nc) * (i + 1) / nc -
                  (width - sep_width - nc) * i / nc;
      }
      size_t m = lines();
      while (m > 1) {
      }
    }
    std::vector<std::vector<string>> cls;
    for (size_t i = 0; i < cols.size(); ++i) {
      cls.push_back(cols[i](cws[i]));
    }
  };
  return func;
    } formatter join(const formatter & l, const string & sep,
                     const formatter & r) {
  auto func = [l, sep, r](size_t width) {
    size_t lw = width > sep.size() + 1 ? (width - sep.size()) / 2 : 1;
    std::vector<string> ls, rs;
    bool start = 1;
    size_t lwl = 1, lwr = width > sep.size() + 1 ? width - sep.size() - 1 : 2;
    size_t fin_lw = lw;
    while (true) {
      size_t rw = lw + sep.size() < width ? width - lw - sep.size() : 1;
      auto cls = l(lw);
      auto crs = r(rw);
      std::cerr << "join func width=" << width << " lw=" << lw << " rw=" << rw
                << " #l" << cls.size() << " #r" << crs.size() << std::endl;
      if (start) {
        ls = cls;
        rs = crs;
        fin_lw = lw;
        start = false;
      } else {
        size_t m = std::max(ls.size(), rs.size());
        size_t cm = std::max(cls.size(), crs.size());
        if (cm < m || (cm == m && std::abs(lw - rw) < std::abs(lw - rw - 2))) {
          ls = cls;
          rs = crs;
          fin_lw = lw;
        }
      }
      if (cls.size() == crs.size()) break;
      if (cls.size() < crs.size()) {
        auto n = (lw + lwl) / 2;
        if (lw == n) break;
        lwr = lw;
        lw = n;
      } else {
        auto n = (lw + lwr) / 2;
        if (lw == n) break;
        lwl = lw + 1;
        lw = n;
      }
    }
    lw = fin_lw;
    std::vector<string> vs;
    for (auto& l : ls) {
      if (l.size() < lw) {
        l += string(lw - l.size(), ' ');
      }
    }
    for (size_t i = 0; i < ls.size() && i < rs.size(); ++i) {
      vs.push_back(ls[i] + sep + rs[i]);
    }
    for (size_t i = rs.size(); i < ls.size(); ++i) {
      vs.push_back(ls[i] + sep);
    }
    for (size_t i = ls.size(); i < rs.size(); ++i) {
      vs.push_back(string(lw, ' ') + sep + rs[i]);
    }
    return vs;
  };
  return func;
}

int main(int argc, char* argv[]) {
  int width = atoi(argv[1]);
  string sep = "|";
  auto left = as_formatter([argv](size_t width) {
    std::cerr << "format 2 width=" << width << std::endl;
    return as_formatter(argv[2])(width);
  });
  for (int i = 3; i < argc; ++i) {
    auto right = as_formatter([i, argv](size_t width) {
      std::cerr << "format " << i << " width=" << width << std::endl;
      return as_formatter(argv[i])(width);
    });
    left = join(left, sep, right);
  }
  auto vs = left(width);
  for (auto v : vs) {
    std::cerr << "|" << v << "\n";
  }
  return 0;
}
