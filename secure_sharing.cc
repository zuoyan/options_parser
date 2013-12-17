/**
 * @file   secure_sharing.cc
 * @author Changsheng Jiang <jiangzuoyan@gmail.com>
 * @date   Sun Dec 15 09:53:37 2013
 *
 * @brief Share information between N nodes, and every K nodes can
 * recover the information, but every (K - 1) nodes can not.
 *
 *
 */
#include "options_parser/options_parser.h"
#include <random>

std::mt19937_64 my_random;

// x^8 + x^4 + x^3 + x + 1.
inline unsigned char f256_add(unsigned char a, unsigned char b) {
  return a ^ b;
}

inline unsigned char f256_sub(unsigned char a, unsigned char b) {
  return a ^ b;
}

// Log table using 0xe5 (229) as the generator
inline unsigned char* f256_logs_table() {
  static unsigned char table[256] = {
      0x00, 0xff, 0xc8, 0x08, 0x91, 0x10, 0xd0, 0x36, 0x5a, 0x3e, 0xd8, 0x43,
      0x99, 0x77, 0xfe, 0x18, 0x23, 0x20, 0x07, 0x70, 0xa1, 0x6c, 0x0c, 0x7f,
      0x62, 0x8b, 0x40, 0x46, 0xc7, 0x4b, 0xe0, 0x0e, 0xeb, 0x16, 0xe8, 0xad,
      0xcf, 0xcd, 0x39, 0x53, 0x6a, 0x27, 0x35, 0x93, 0xd4, 0x4e, 0x48, 0xc3,
      0x2b, 0x79, 0x54, 0x28, 0x09, 0x78, 0x0f, 0x21, 0x90, 0x87, 0x14, 0x2a,
      0xa9, 0x9c, 0xd6, 0x74, 0xb4, 0x7c, 0xde, 0xed, 0xb1, 0x86, 0x76, 0xa4,
      0x98, 0xe2, 0x96, 0x8f, 0x02, 0x32, 0x1c, 0xc1, 0x33, 0xee, 0xef, 0x81,
      0xfd, 0x30, 0x5c, 0x13, 0x9d, 0x29, 0x17, 0xc4, 0x11, 0x44, 0x8c, 0x80,
      0xf3, 0x73, 0x42, 0x1e, 0x1d, 0xb5, 0xf0, 0x12, 0xd1, 0x5b, 0x41, 0xa2,
      0xd7, 0x2c, 0xe9, 0xd5, 0x59, 0xcb, 0x50, 0xa8, 0xdc, 0xfc, 0xf2, 0x56,
      0x72, 0xa6, 0x65, 0x2f, 0x9f, 0x9b, 0x3d, 0xba, 0x7d, 0xc2, 0x45, 0x82,
      0xa7, 0x57, 0xb6, 0xa3, 0x7a, 0x75, 0x4f, 0xae, 0x3f, 0x37, 0x6d, 0x47,
      0x61, 0xbe, 0xab, 0xd3, 0x5f, 0xb0, 0x58, 0xaf, 0xca, 0x5e, 0xfa, 0x85,
      0xe4, 0x4d, 0x8a, 0x05, 0xfb, 0x60, 0xb7, 0x7b, 0xb8, 0x26, 0x4a, 0x67,
      0xc6, 0x1a, 0xf8, 0x69, 0x25, 0xb3, 0xdb, 0xbd, 0x66, 0xdd, 0xf1, 0xd2,
      0xdf, 0x03, 0x8d, 0x34, 0xd9, 0x92, 0x0d, 0x63, 0x55, 0xaa, 0x49, 0xec,
      0xbc, 0x95, 0x3c, 0x84, 0x0b, 0xf5, 0xe6, 0xe7, 0xe5, 0xac, 0x7e, 0x6e,
      0xb9, 0xf9, 0xda, 0x8e, 0x9a, 0xc9, 0x24, 0xe1, 0x0a, 0x15, 0x6b, 0x3a,
      0xa0, 0x51, 0xf4, 0xea, 0xb2, 0x97, 0x9e, 0x5d, 0x22, 0x88, 0x94, 0xce,
      0x19, 0x01, 0x71, 0x4c, 0xa5, 0xe3, 0xc5, 0x31, 0xbb, 0xcc, 0x1f, 0x2d,
      0x3b, 0x52, 0x6f, 0xf6, 0x2e, 0x89, 0xf7, 0xc0, 0x68, 0x1b, 0x64, 0x04,
      0x06, 0xbf, 0x83, 0x38};
  return table;
}

// Rev Log table using 0xe5 (229) as the generator
inline unsigned char* f256_exps_table() {
  static unsigned char table[256] = {
      0x01, 0xe5, 0x4c, 0xb5, 0xfb, 0x9f, 0xfc, 0x12, 0x03, 0x34, 0xd4, 0xc4,
      0x16, 0xba, 0x1f, 0x36, 0x05, 0x5c, 0x67, 0x57, 0x3a, 0xd5, 0x21, 0x5a,
      0x0f, 0xe4, 0xa9, 0xf9, 0x4e, 0x64, 0x63, 0xee, 0x11, 0x37, 0xe0, 0x10,
      0xd2, 0xac, 0xa5, 0x29, 0x33, 0x59, 0x3b, 0x30, 0x6d, 0xef, 0xf4, 0x7b,
      0x55, 0xeb, 0x4d, 0x50, 0xb7, 0x2a, 0x07, 0x8d, 0xff, 0x26, 0xd7, 0xf0,
      0xc2, 0x7e, 0x09, 0x8c, 0x1a, 0x6a, 0x62, 0x0b, 0x5d, 0x82, 0x1b, 0x8f,
      0x2e, 0xbe, 0xa6, 0x1d, 0xe7, 0x9d, 0x2d, 0x8a, 0x72, 0xd9, 0xf1, 0x27,
      0x32, 0xbc, 0x77, 0x85, 0x96, 0x70, 0x08, 0x69, 0x56, 0xdf, 0x99, 0x94,
      0xa1, 0x90, 0x18, 0xbb, 0xfa, 0x7a, 0xb0, 0xa7, 0xf8, 0xab, 0x28, 0xd6,
      0x15, 0x8e, 0xcb, 0xf2, 0x13, 0xe6, 0x78, 0x61, 0x3f, 0x89, 0x46, 0x0d,
      0x35, 0x31, 0x88, 0xa3, 0x41, 0x80, 0xca, 0x17, 0x5f, 0x53, 0x83, 0xfe,
      0xc3, 0x9b, 0x45, 0x39, 0xe1, 0xf5, 0x9e, 0x19, 0x5e, 0xb6, 0xcf, 0x4b,
      0x38, 0x04, 0xb9, 0x2b, 0xe2, 0xc1, 0x4a, 0xdd, 0x48, 0x0c, 0xd0, 0x7d,
      0x3d, 0x58, 0xde, 0x7c, 0xd8, 0x14, 0x6b, 0x87, 0x47, 0xe8, 0x79, 0x84,
      0x73, 0x3c, 0xbd, 0x92, 0xc9, 0x23, 0x8b, 0x97, 0x95, 0x44, 0xdc, 0xad,
      0x40, 0x65, 0x86, 0xa2, 0xa4, 0xcc, 0x7f, 0xec, 0xc0, 0xaf, 0x91, 0xfd,
      0xf7, 0x4f, 0x81, 0x2f, 0x5b, 0xea, 0xa8, 0x1c, 0x02, 0xd1, 0x98, 0x71,
      0xed, 0x25, 0xe3, 0x24, 0x06, 0x68, 0xb3, 0x93, 0x2c, 0x6f, 0x3e, 0x6c,
      0x0a, 0xb8, 0xce, 0xae, 0x74, 0xb1, 0x42, 0xb4, 0x1e, 0xd3, 0x49, 0xe9,
      0x9c, 0xc8, 0xc6, 0xc7, 0x22, 0x6e, 0xdb, 0x20, 0xbf, 0x43, 0x51, 0x52,
      0x66, 0xb2, 0x76, 0x60, 0xda, 0xc5, 0xf3, 0xf6, 0xaa, 0xcd, 0x9a, 0xa0,
      0x75, 0x54, 0x0e, 0x01};
  return table;
}

inline unsigned char f256_mul(unsigned char a, unsigned char b) {
  if (!a || !b) return 0;
  int s = f256_logs_table()[a] + f256_logs_table()[b];
  if (s > 255) s -= 255;
  return f256_exps_table()[s];
}

inline unsigned char f256_inv(unsigned char x) {
  return f256_exps_table()[255 - f256_logs_table()[x]];
}

inline unsigned char f256_div(unsigned char n, unsigned char q) {
  if (!n || !q) return 0;
  int s = 255 + f256_logs_table()[n] - f256_logs_table()[q];
  if (s > 255) s -= 255;
  return f256_exps_table()[s];
}

inline unsigned char f256_pow(unsigned char x, size_t n) {
  if (n == 0) return 1;
  if (x == 0) return 0;
  n *= f256_logs_table()[x];
  n %= 255;
  return f256_exps_table()[n];
}

struct F256 {
  unsigned char value;
  explicit F256(unsigned char v) : value(v) {}
  F256(const F256&) = default;
  F256() : value(0) {}

  F256 operator+(const F256& o) const { return F256(f256_add(value, o.value)); }

  F256 operator-(const F256& o) const { return F256(f256_sub(value, o.value)); }

  F256 operator*(const F256& o) const { return F256(f256_mul(value, o.value)); }

  F256 operator/(const F256& o) const { return F256(f256_div(value, o.value)); }

  F256& operator+=(const F256& o) {
    value = f256_add(value, o.value);
    return *this;
  }

  F256& operator-=(const F256& o) {
    value = f256_sub(value, o.value);
    return *this;
  }

  F256& operator*=(const F256& o) {
    value = f256_mul(value, o.value);
    return *this;
  }

  F256& operator/=(const F256& o) {
    value = f256_div(value, o.value);
    return *this;
  }
};

F256 poly_eval(size_t K, const F256* coefficients, F256 x) {
  F256 y(0);
  for (size_t i = K; i-- > 0;) {
    y = y * x + coefficients[i];
  }
  return y;
}

F256 poly_interp(size_t K, const F256* xs, const F256* ys, F256 x) {
  F256 y(0);
  for (size_t i = 0; i < K; ++i) {
    F256 weight(1);
    for (size_t j = 0; j < K; ++j) {
      if (j == i) continue;
      // weight *= (x - xs[j] / (xs[i] - xs[j])
      F256 xi = xs[i];
      F256 xj = xs[j];
      weight *= (x - xj) / (xi - xj);
    }
    F256 yi = ys[i];
    y += weight * yi;
  }
  return y;
}

void poly_resolve(size_t K, const F256* xs, const F256* ys, F256* P) {
  std::vector<F256> tPi(K + 1);
  F256* Pi = tPi.data() + 1;
  for (size_t i = 0; i < K; ++i) {
    P[i] = F256();
  }
  for (ssize_t i = 0; i < (ssize_t)K; ++i) {
    Pi[0] = F256(1);
    F256 weight(1);
    for (ssize_t j = 0; j < (ssize_t)K; ++j) {
      if (j == i) continue;
      F256 xi = xs[i];
      F256 xj = xs[j];
      weight *= xi - xj;
      for (ssize_t l = j + (j < i); l >= 0; --l) {
        Pi[l] = Pi[l - 1] - Pi[l] * xj;
      }
    }
    F256 yi = ys[i];
    weight = yi / weight;
    for (size_t l = 0; l < K; ++l) {
      P[l] += Pi[l] * weight;
      Pi[l] = F256();
    }
  }
}


// 'mat'is a K x K matrix in row major.
void matrix_mv(size_t K, const F256 *mat, const F256 * x, F256 *y) {
  for (size_t i = 0; i < K; ++i) {
    y[i] = F256();
    for (size_t j = 0; j < K; ++j) {
      y[i] += mat[i * K + j] * x[j];
    }
  }
}

void matrix_multiply(size_t K, const F256 *A, const F256 *B, F256 *C) {
  for (size_t i = 0; i < K; ++i) {
    for (size_t j = 0; j < K; ++j) {
      C[i * K + j] = F256();
      for (size_t k = 0; k < K; ++k) {
        C[i * K + j] += A[i * K + k] * B[k * K + j];
      }
    }
  }
}

bool matrix_inverse(size_t K, const F256 *A_, F256 *B) {
  for (size_t i = 0; i < K; ++i) {
    for (size_t j = 0; j < K; ++j) {
      B[i * K + j] = F256(i == j);
    }
  }
  std::vector<F256> A(A_, A_ + K * K);
  for (size_t i = 0; i < K; ++i) {
    size_t p = i;
    for (p = i; p < K; ++p) {
      if (A[p * K + i].value) break;
    }
    if (p == K) return false;
    if (p != i) {
      for (size_t j = 0; j < K; ++j) {
        std::swap(A[p * K + j], A[i * K + j]);
        std::swap(B[p * K + j], B[i * K + j]);
      }
    }
    auto a = A[i * K + i];
    assert(a.value != 0);
    for (size_t j = 0; j < K; ++j) {
      B[i * K + j] /= a;
    }
    for (size_t j = i; j < K; ++j) {
      A[i * K + j] /= a;
    }
    for (size_t r = 0; r < K; ++r) {
      if (r == i) continue;
      auto a = A[r * K + i];
      for (size_t j = 0; j < K; ++j) {
        B[r * K + j] -= a * B[i * K + j];
      }
      for (size_t j = i; j < K; ++j) {
        A[r * K + j] -= a * A[i * K + j];
      }
      assert (r >= i || A[r * K + i].value == 0);
    }
  }
  return true;
}

#ifndef NDEBUG
void test() {
  for (int i = 1; i < 256; ++i) {
    auto x = F256(i);
    assert((x * x * x).value == f256_pow(x.value, 3));
    assert((x * x * x * x * x).value == f256_pow(x.value, 5));
  }
  for (int i = 0; i < 256; ++i) {
    std::vector<F256> vs;
    for (int j = 0; j < 256; ++j) {
      F256 ij = F256(i) * F256(j);
      F256 ji = F256(j) * F256(i);
      assert(ji.value == ij.value);
      vs.push_back(ij);
      if (i == 0 || j == 0) {
        assert(ij.value == 0);
        continue;
      }
      assert((ij / F256(j)).value == i);
      assert((ij / F256(i)).value == j);
    }
    if (i == 0) continue;
    std::set<int> ivs;
    for (auto v : vs) {
      ivs.insert(v.value);
    }
    assert(ivs.size() == 256);
  }
  for (size_t c = 0; c < 1000; ++c) {
    std::vector<F256> C(my_random() % 256);
    for (size_t i = 0; i < C.size(); ++i) {
      C[i].value = my_random();
    }
    std::set<int> xset;
    while (xset.size() < C.size()) {
      xset.insert(my_random() % 256);
    }
    std::vector<F256> xs, ys(C.size());
    for (auto x : xset) {
      xs.emplace_back(x);
    }
    for (size_t i = 0; i < xs.size(); ++i) {
      auto x = xs[i];
      auto y = poly_eval(C.size(), C.data(), x);
      ys[i] = y;
      F256 t{};
      for (size_t j = 0; j < C.size(); ++j) {
        t += C[j] * F256(f256_pow(x.value, j));
      }
      assert(t.value == y.value);
    }
    std::vector<F256> Cc(C.size());
    poly_resolve(C.size(), xs.data(), ys.data(), Cc.data());
    for (size_t i = 0; i < C.size(); ++i) {
      assert(C[i].value == Cc[i].value);
    }
  }

  for (size_t c = 0; c < 1000; ++c) {
    size_t K = 1 + my_random() % 256;
    std::vector<F256> A(K * K), B(K * K), C(K * K);
    for (size_t i = 0; i < A.size(); ++i) A[i].value = my_random();
    bool f = matrix_inverse(K, A.data(), B.data());
    if (f) {
      matrix_multiply(K, A.data(), B.data(), C.data());
      for (size_t i = 0; i < K; ++i) {
        for (size_t j = 0; j < K; ++j) {
          if (C[i * K + j].value != (i == j)) {
            for (size_t i = 0; i < K; ++i) {
              std::cerr << "C[" << i << "] ";
              for (size_t j = 0; j < K; ++j) {
                std::cerr << " " << (int)C[i * K + j].value;
              }
              std::cerr << std::endl;
            }
          }
          assert(C[i * K + j].value == (i == j));
        }
      }
    } else {
      std::cerr << "c="<< c << " K=" << K << " random matrix not inversable\n";
    }
  }
}
#endif

void sharing_seg(size_t K, const char* plain, const F256* codes,
                std::vector<std::string>& parts) {
  size_t N = parts.size();
  const F256* coefficients = (const F256*)plain;
  for (size_t i = 0; i < N; ++i) {
    char c = poly_eval(K, coefficients, codes[i]).value;
    parts[i].push_back(c);
  }
}

void sharing_message(size_t K, size_t m, const char* plain, const F256* codes,
                     std::vector<std::string>& parts) {
  std::vector<F256> M(K * K), Minv(K * K);
  while (1) {
    for (size_t i = 0; i < M.size(); ++i) {
      M[i].value = my_random();
    }
    if (matrix_inverse(K, M.data(), Minv.data())) break;
  }
  for (size_t i = 0; i < K; ++i) {
    sharing_seg(K, (char*)&M[i * K], codes, parts);
  }
  std::vector<char> C(K);
  for (size_t i = 0; i < K; ++i) {
    C[i] = my_random();
  }
  sharing_seg(K, C.data(), codes, parts);
  std::vector<char> E(K);
  size_t off = 0;
  while (off + K <= m) {
    for (size_t i = 0; i < K; ++i) {
      C[i] ^= plain[off + i];
    }
    matrix_mv(K, M.data(), (const F256*)C.data(), (F256*)E.data());
    sharing_seg(K, E.data(), codes, parts);
    C.swap(E);
    off += K;
  }
  for (size_t i = 0; i + off < m; ++i) {
    C[i] ^= plain[off + i];
  }
  for (size_t i = m - off; i + 1 < K; ++i) {
    C[i] = my_random();
  }
  C.back() ^= (m - off);
  matrix_mv(K, M.data(), (const F256*)C.data(), (F256*)E.data());
  sharing_seg(K, E.data(), codes, parts);
}

void recover_seg(size_t K, const F256* codes, const F256* seg, F256* plain) {
  poly_resolve(K, codes, seg, plain);
}

void recover_message(size_t K, const F256* codes,
                     const std::vector<std::string>& parts,
                     std::string& plain) {
  assert(parts.size() >= K);
  size_t pl = parts.front().size();
  assert(pl >= K + 2);
  for (size_t i = 0; i < parts.size(); ++i) {
    if (parts[i].size() != pl) {
      std::cerr << "parts with different length ...\n";
      abort();
    }
  }
  std::vector<F256> seg(K);
  std::vector<F256> C(K), E(K);
  std::vector<F256> M(K * K), Minv(K * K);
  for (size_t i = 0; i < K; ++i) {
    for (size_t j = 0; j < K; ++j) {
      seg[j] = F256(parts[j][i]);
    }
    recover_seg(K, codes, seg.data(), &M[i * K]);
  }
  for (size_t j = 0; j < K; ++j) {
    seg[j] = F256(parts[j][K]);
  }
  recover_seg(K, codes, seg.data(), C.data());
  if (!matrix_inverse(K, M.data(), Minv.data())) {
    std::cerr << "recover message got a degenerated matrix ...\n";
    return;
  }
  for (size_t i = K + 1; i < pl; ++i) {
    for (size_t j = 0; j < K; ++j) {
      seg[j] = F256(parts[j][i]);
    }
    recover_seg(K, codes, seg.data(), E.data());
    matrix_mv(K, Minv.data(), E.data(), seg.data());
    for (size_t j = 0; j < K; ++j) {
      seg[j] += C[j];
    }
    C.swap(E);
    size_t L = K;
    if (i + 1 == pl) L = seg.back().value;
    assert(L <= K);
    for (size_t j = 0; j < L; ++j) {
      plain.push_back(seg[j].value);
    }
  }
}

void split_file(
    size_t K, const std::string& in_file,
    const std::vector<std::pair<F256, std::string>>& out_code_files) {
  std::vector<F256> codes(out_code_files.size());
  for (size_t i = 0; i < out_code_files.size(); ++i) {
    codes[i] = out_code_files[i].first;
  }
  std::string plain;
  {
    std::ifstream ifs(in_file);
    plain.assign(std::istreambuf_iterator<char>(ifs),
                 std::istreambuf_iterator<char>());
  }
  std::vector<std::string> parts(codes.size());
  sharing_message(K, plain.size(), plain.data(), codes.data(), parts);
  for (size_t i = 0; i < parts.size(); ++i) {
    std::ofstream ofs(out_code_files[i].second);
    ofs.write(parts[i].data(), parts[i].size());
  }
}

void combine_file(
    size_t K, const std::string& out_file,
    const std::vector<std::pair<F256, std::string>>& in_code_files) {
  std::vector<F256> codes(in_code_files.size());
  for (size_t i = 0; i < in_code_files.size(); ++i) {
    codes[i] = in_code_files[i].first;
  }
  std::vector<std::string> parts(codes.size());
  for (size_t i = 0; i < parts.size(); ++i) {
    std::ifstream ifs(in_code_files[i].second);
    parts[i].assign(std::istreambuf_iterator<char>(ifs),
                    std::istreambuf_iterator<char>());
  }
  std::string plain;
  recover_message(K, codes.data(), parts, plain);
  std::ofstream ofs(out_file);
  ofs.write(plain.data(), plain.size());
}

// out.1 => 1:out.1
// out.[1-3] => 1:out.1 2:out.2 3:out.3
// out.[1,3-5] => out.1 out.3 out.4 out.5
std::vector<std::pair<F256, std::string>> expand_code_file(
    const std::string& pattern) {
  std::vector<std::pair<F256, std::string>> ret;
  auto lb = pattern.find('[');
  auto rb = pattern.find(']', lb);
  if (lb >= pattern.size() || rb >= pattern.size()) {
    auto d = pattern.find_first_of("123456789");
    if (d >= pattern.size()) return ret;
    d = pattern.find_last_of("123456789");
    auto b = pattern.find_last_not_of("123456789", d);
    if (b >= pattern.size()) b = -1;
    auto c = atoi(pattern.data() + b + 1);
    ret.emplace_back(F256(c), pattern);
    return ret;
  }
  auto tokens = options_parser::split(pattern.substr(lb + 1, rb - lb - 1), ",");
  auto prefix = pattern.substr(0, lb);
  auto postfix = pattern.substr(rb + 1);
  for (auto& t : tokens) {
    t = options_parser::strip(t);
    if (!t.size()) continue;
    if (t.find('-') >= t.size()) {
      char *e;
      int c = strtol(t.data(), &e, 10);
      if (*e || c < 0 || c >= 256) {
        ret.clear();
        return ret;
      }
      ret.emplace_back(F256(c), prefix + t + postfix);
      continue;
    }
    char *e;
    size_t first = strtol(t.data(), &e, 10);
    if (*e != '-') {
      ret.clear();
      return ret;
    }
    size_t last = strtol(t.data() + t.find('-') + 1, &e, 10);
    if (*e || last >= 256 || first > last) {
      ret.clear();
      return ret;
    }
    for (auto i = first; i <= last; ++i) {
      auto k = options_parser::to_str(i);
      while (k.size() < t.find('-')) k = "0" + k;
      ret.emplace_back(F256(i), prefix + k + postfix);
    }
  }
  return ret;
}

int main(int argc, char *argv[]) {
  {
    // seed by default
    my_random.seed(std::random_device{}());
  }
  options_parser::Parser app(
      "Split a message, and deliver to N nodes. Every K nodes"
      " can recover the origin message, but every (K - 1) nodes can not."
      " Or recover a message from K nodes after splitting.\n"
      "Usage:\n"
      "  secure_sharing -K NUM --split in-file out-file-with-codes\n"
      "  secure_sharing -K NUM --combine out-file in-file-with-codes\n"
      "\n",
      "Examples:\n"
      "\n"
      "  To split file 'secure.doc' to 10 files, and every 3 files can recover"
      " the 'secure.doc'. Note that, '[1-10]' is a pattern also used by shell,"
      " so you'd better quote it:\n"
      "    secure_sharing -K 3 --split secure.doc 'secure.part.[1-10]'\n"
      "\n"
      "  Combine file parts to recover the orignal:\n"
      "    secure_sharing -K 3 --combine secure.doc 'secure.part[1-3]'\n"
      "\n"
      "Please note that this program does *NOT* encrypt any thing, it is"
      " possible to recover the orginal file if it's entropy is very small."
      " You are oblidged to encrypt it before passing to this tool.\n"
      "Wrote by jiangzuoyan@gmail.com, use it at you own risk.");
  app.add_help();
  size_t K = 0;

#ifndef NDEBUG
  // &test doesn't work with g++-4.6
  app.add_option("--test", []() {test();}, "test first");
#endif

  app.add_option("--seed NUM", [](size_t a) {
      my_random.seed(a);
    }, "seed the random");

  app.add_option("-K, --min-recover-nodes", [&](size_t k)->std::string {
                                              if (k == 0) {
                                                return "must be positive";
                                              }
                                              K = k;
                                              return "";
                                            },
                 "the minimal nodes required to recover");

  auto get_arg = [&](const options_parser::MatchResult& mr,
                     std::string& file_name,
                     std::vector<std::pair<F256, std::string>>& code_files) {
    options_parser::TakeResult tr;
    auto f_s = options_parser::value()(mr.situation);
    tr.error = get_error(f_s.first);
    tr.situation = f_s.second;
    if (tr.error) {
      return tr;
    }
    file_name = get_value(f_s.first);
    while (1) {
      auto c_s = options_parser::value<size_t>()(tr.situation);
      if (get_error(c_s.first)) {
        auto f_s = options_parser::value()(tr.situation);
        if (get_error(f_s.first)) break;
        auto cfs = expand_code_file(get_value(f_s.first));
        if (!cfs.size()) break;
        for (const auto& cf : cfs) {
          code_files.push_back(cf);
        }
        tr.situation = f_s.second;
        continue;
      }
      size_t c = get_value(c_s.first);
      if (c >= 256) {
        tr.error = "invalid code, must be 0-255";
        return tr;
      }
      auto f_s = options_parser::value()(c_s.second);
      if (get_error(f_s.first)) break;
      tr.situation = f_s.second;
      code_files.emplace_back(F256(c), get_value(f_s.first));
    }
    std::set<int> codes;
    for (const auto& cf: code_files) {
      codes.insert(cf.first.value);
    }
    if (codes.size() != code_files.size()) {
      tr.error = "some codes duplicate";
    }
    if (codes.size() < K) {
      tr.error = "expect at least " + options_parser::to_str(K) +
                 " files with different codes, but given # " +
                 options_parser::to_str(codes.size());
    }
    return tr;
  };

  app.add_option(
      "--split IN-FILE [CODE OUT-FILE].../[OUT-FILENAME-WITH-CODE]...",
      [&](const options_parser::MatchResult& mr) {
        std::string in_file;
        std::vector<std::pair<F256, std::string>> out_code_files;
        auto tr = get_arg(mr, in_file, out_code_files);
        if (tr.error) return tr;
        split_file(K, in_file, out_code_files);
        return tr;
      },
      "split IN-FILE and store every parts to OUT-FILE...");

  app.add_option(
      "--combine OUT-FILE [CODE IN-FILE].../[IN-FILENAME-WITH-CODE]...",
      [&](const options_parser::MatchResult& mr) {
        std::string out_file;
        std::vector<std::pair<F256, std::string>> in_code_files;
        auto tr = get_arg(mr, out_file, in_code_files);
        if (tr.error) return tr;
        combine_file(K, out_file, in_code_files);
        return tr;
      },
      "recover from IN-FILE... to OUT-FILE");

  auto parse_result = app.parse(argc, argv);

  if (parse_result.error) {
    std::cerr << "parse-error: " << *parse_result.error.get() << std::endl;
    if (parse_result.error_full) {
      std::cerr << *parse_result.error_full.get() << std::endl;
    }
    return 1;
  }

  return 0;
}
