#include <iostream>
#include <cassert>

#include "options_parser/circumstance.h"

#include "test.h"
#include "test_lib.h"

namespace options_parser {

TEST(Basic) {
  Circumstance a;
  a.set("name", string{"value"});
  Circumstance c = a.new_child();
  ASSERT(c.get<string>("name"));
  CHECK_EQ(*c.get<string>("name"), "value");
  c.set("name", string{"c-set-value"});
  CHECK_EQ(*c.get<string>("name"), "c-set-value");
  CHECK_EQ(*a.get<string>("name"), "c-set-value");
  c.local_set("name", string{"c-local"});
  CHECK_EQ(*c.get<string>("name"), "c-local");
  CHECK_EQ(*a.get<string>("name"), "c-set-value");
  c.local_set("name", string{"c-set"});
  CHECK_EQ(*c.get<string>("name"), "c-set");
  CHECK_EQ(*a.get<string>("name"), "c-set-value");
}

}  // namespace options_parser
