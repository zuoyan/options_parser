#include <iostream>
#include <cassert>

#include "test.h"
#include "test_lib.h"

TEST(SimpleTest) {
  int a = 13;
  int b = 13;
  EXPECT_EQ(a, b);
}
