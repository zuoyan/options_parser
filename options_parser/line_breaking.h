#ifndef FILE_C8B83754_2BB1_4F59_9DE9_E4E510196300_H
#define FILE_C8B83754_2BB1_4F59_9DE9_E4E510196300_H

#include "options_parser/string.h"

#include <vector>
#include <cassert>

namespace options_parser {
namespace line_breaking {

// test linebreaking
struct Block {
  bool drop_at_first;
  bool drop_at_last;
  double width;
  double width_at_first;
  double width_at_last;
  double stretch;
  double stretch_at_first;
  double stretch_at_last;
  double shrink;
  double shrink_at_first;
  double shrink_at_last;
  string value;
  string value_at_last;
  string value_at_first;
  double penalty;

  Block() {
    drop_at_first = false;
    drop_at_last = false;
    width = 0;
    width_at_first = 0;
    width_at_last = 0;
    stretch = 0;
    stretch_at_first = 0;
    stretch_at_last = 0;
    shrink = 0;
    shrink_at_first = 0;
    shrink_at_last = 0;
    penalty = 0;
  }
};

inline void string_to_blocks(const string &s, std::vector<Block> *blocks) {
  for (size_t i = 0; i < s.size(); ++i) {
    if (isspace(s[i])) {
      if (!blocks->size() || blocks->back().value == " ") continue;
    }
    Block b;
    bool c = isspace(s[i]);
    b.drop_at_last = c;
    b.drop_at_first = c;
    b.width = 1;
    b.width_at_first = c ? 0 : b.width;
    b.width_at_last = c ? 0 : b.width;
    b.stretch = c ? 0 : 0;
    b.stretch_at_last = c ? 0 : b.stretch;
    b.stretch_at_first = c ? 0 : b.stretch;
    b.shrink = 0;
    b.shrink_at_last = c ? 0 : b.shrink;
    b.shrink_at_first = c ? 0 : b.shrink;
    b.value.push_back(s[i]);
    b.value_at_first = b.value;
    b.value_at_last = b.value;
    blocks->push_back(b);
  }
}

inline void blocks_fill_penalty_and_hyphen(std::vector<Block> *blocks,
                                           double line_penalty,
                                           double word_penalty,
                                           double hyphen_penaly) {
  for (size_t i = 0; i < blocks->size(); ++i) {
    auto &b = blocks->data()[i];
    b.penalty = line_penalty;
    if (!(isspace(b.value[0]) || ispunct(b.value[0]))) {
      if (b.value == "-") {
        b.penalty += hyphen_penaly;
      }
    }
  }
  for (size_t i = 0; i + 1 < blocks->size(); ++i) {
    auto &c = blocks->data()[i];
    auto &n = blocks->data()[i + 1];
    if (n.value.size() && !isspace(n.value[0])) {
      c.penalty += word_penalty;
      c.value_at_last += '-';
      c.width_at_last += 1;
    }
  }
}

struct LineBreaking {
  LineBreaking(const std::vector<Block> *blocks) : blocks_(blocks) {
    width_partial_sum.resize(blocks->size() + 1);
    stretch_partial_sum.resize(blocks->size() + 1);
    shrink_partial_sum.resize(blocks->size() + 1);
    width_partial_sum.front() = 0;
    stretch_partial_sum.front() = 0;
    shrink_partial_sum.front() = 0;
    for (size_t i = 0; i < blocks->size(); ++i) {
      width_partial_sum[i + 1] = width_partial_sum[i] + blocks->data()[i].width;
      stretch_partial_sum[i + 1] =
          stretch_partial_sum[i] + blocks->data()[i].stretch;
      shrink_partial_sum[i + 1] =
          shrink_partial_sum[i] + blocks->data()[i].shrink;
    }
  }

  double line_badness(size_t f, size_t e, double width) {
    assert(f < e);
    assert(e <= blocks_->size());
    double real_width = width_partial_sum[e] - width_partial_sum[f];
    real_width += (*blocks_)[f].width_at_first - (*blocks_)[f].width;
    real_width += (*blocks_)[e - 1].width_at_last - (*blocks_)[e - 1].width;
    double adjust = width - real_width;
    if (adjust == 0) return 0;
    double adjust_ratio = 0;
    if (adjust > 0) {
      double stretch = stretch_partial_sum[e] - stretch_partial_sum[f];
      if (f + 1 == e) {
        stretch = std::min((*blocks_)[f].stretch_at_first,
                           (*blocks_)[f].stretch_at_last);
      } else {
        stretch += (*blocks_)[f].stretch_at_first - (*blocks_)[f].stretch;
        stretch +=
            (*blocks_)[e - 1].stretch_at_last - (*blocks_)[e - 1].stretch;
      }
      adjust_ratio = adjust / (stretch + 1.0);
    } else if (adjust < 0) {
      double shrink = shrink_partial_sum[e] - shrink_partial_sum[f];
      if (f + 1 == e) {
        shrink = std::min((*blocks_)[f].shrink_at_first,
                          (*blocks_)[f].shrink_at_last);
      } else {
        shrink += (*blocks_)[f].shrink_at_first - (*blocks_)[f].shrink;
        shrink += (*blocks_)[e - 1].shrink_at_last - (*blocks_)[e - 1].shrink;
      }
      adjust_ratio = -adjust / (shrink + 0.005);
    }
    return adjust_ratio * adjust_ratio * adjust_ratio + 0.5;
  }

  void break_lines(std::vector<size_t> *lines, size_t width) {
    // break after i, best penaly and last break
    if (!blocks_->size()) return;
    std::vector<std::pair<double, size_t>> penalties(blocks_->size());
    for (size_t i = 0; i < blocks_->size(); ++i) {
      double p = line_badness(0, i + 1, width) + (*blocks_)[i].penalty;
      penalties[i] = std::make_pair(p, i + 1);
    }
    for (size_t i = 0; i < blocks_->size(); ++i) {
      double p = penalties[i].first;
      size_t l = penalties[i].second;
      for (size_t j = 0; j < i; ++j) {
        double t = penalties[j].first + line_badness(j + 1, i + 1, width) +
                   (*blocks_)[i].penalty;
        if (t <= p) {
          p = t;
          l = j + 1;
        }
      }
      penalties[i] = std::make_pair(p, l);
    }
    lines->push_back(blocks_->size());
    while (1) {
      auto l = penalties[lines->back() - 1].second;
      if (l == lines->back()) break;
      lines->push_back(l);
    }
    for (size_t i = 0; i + i < lines->size(); ++i) {
      std::swap(lines->data()[i], lines->data()[lines->size() - 1 - i]);
    }
  }

  const std::vector<Block> *blocks_;
  std::vector<double> width_partial_sum;
  std::vector<double> stretch_partial_sum;
  std::vector<double> shrink_partial_sum;
};

inline std::vector<string> break_string(const string &text, size_t width,
                                        bool keep_first_space = true) {
  std::vector<string> ret;
  std::vector<Block> blocks;
  string_to_blocks(text, &blocks);
  if (keep_first_space) {
    Block b;
    for (size_t i = 0; i < text.size(); ++i) {
      if (isspace(text[i])) {
        b.value += text[i];
      } else {
        break;
      }
    }
    b.width = b.value.size();
    b.width_at_first = b.width;
    b.width_at_last = b.width;
    blocks.push_back(b);
  }
  blocks_fill_penalty_and_hyphen(&blocks, 1000, 100000, 100);
  {
    // fill at tail ...
    Block b;
    b.drop_at_last = true;
    b.drop_at_first = true;
    b.stretch = 1.e20;
    b.stretch_at_last = 1.e20;
    blocks.push_back(b);
  }
  LineBreaking lb(&blocks);
  if (!blocks.size()) return ret;
  std::vector<size_t> lns;
  lb.break_lines(&lns, width);
  assert(lns.back() == blocks.size());
  for (size_t i = 0; i < lns.size(); ++i) {
    size_t s = i ? lns[i - 1] : 0;
    size_t e = lns[i];
    while (s < e) {
      if (blocks[s].drop_at_first) {
        ++s;
      } else {
        break;
      }
    }
    while (s < e) {
      if (blocks[e - 1].drop_at_first) {
        --e;
      } else {
        break;
      }
    }
    string l;
    for (size_t i = s; i < e; ++i) {
      if (i + 1 == e) {
        l += blocks[i].value_at_last;
      } else if (i == s) {
        l += blocks[i].value_at_first;
      } else {
        l +=  blocks[i].value;
      }
    }
    ret.push_back(l);
  }
  return ret;
}

}  // namespace line_breaking
}  // namespace options_parser
#endif // FILE_C8B83754_2BB1_4F59_9DE9_E4E510196300_H
