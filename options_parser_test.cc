#include <iostream>
#include <cassert>

#include "options_parser/options_parser_lib.h"

using namespace options_parser;

OPTIONS_PARSER_FLAGS_DEFINE(int, flag_int, 0, "A flag of int type");

int main(int argc, char *argv[]) {
  Parser app(
      std::string("Test options parser.\n") +
      "Usage: " + argv[0] +
          " [<options>]\n"
          "            sub [<sub options>] [--]\n"
          "Sed ut perspiciatis, unde omnis iste natus error sit voluptatem "
          "accusantium doloremque laudantium, totam rem aperiam eaque ipsa, "
          "quae ab illo inventore veritatis et quasi architecto beatae vitae "
          "dicta sunt, explicabo. Nemo enim ipsam voluptatem, quia voluptas "
          "sit, aspernatur aut odit aut fugit, sed quia consequuntur magni "
          "dolores eos, qui ratione voluptatem sequi nesciunt, neque porro "
          "quisquam est, qui dolorem ipsum, quia dolor sit amet consectetur "
          "adipisci[ng] velit, sed quia non numquam [do] eius modi tempora "
          "inci[di]dunt, ut labore et dolore magnam aliquam quaerat "
          "voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem "
          "ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi "
          "consequatur? Quis autem vel eum iure reprehenderit, qui in ea "
          "voluptate velit esse, quam nihil molestiae consequatur, vel illum, "
          "qui dolorem eum fugiat, quo voluptas nulla pariatur?\n\n",
      "This's options parse following a state monad design.\n"
      "At vero eos et accusamus et iusto odio dignissimos ducimus, qui "
      "blanditiis praesentium voluptatum deleniti atque corrupti, quos "
      "dolores et quas molestias excepturi sint, obcaecati cupiditate "
      "non provident, similique sunt in culpa, qui officia deserunt "
      "mollitia animi, id est laborum et dolorum fuga. Et harum quidem "
      "rerum facilis est et expedita distinctio. Nam libero tempore, "
      "cum soluta nobis est eligendi optio, cumque nihil impedit, quo "
      "minus id, quod maxime placeat, facere possimus, omnis voluptas "
      "assumenda est, omnis dolor repellendus. Temporibus autem "
      "quibusdam et aut officiis debitis aut rerum necessitatibus saepe "
      "eveniet, ut et voluptates repudiandae sint et molestiae non "
      "recusandae. Itaque earum rerum hic tenetur a sapiente delectus, "
      "ut aut reiciendis voluptatibus maiores alias consequatur aut "
      "perferendis doloribus asperiores repellat…");
  app.add_parser(options_parser::parser());

  app.add_option("--config-file", [&](const std::string &fn) {
      options_parser::Maybe<std::string> error, error_full;
      app.parse_file(fn, &error, &error_full);
      return error_full ? *error_full.get() : "";
                                },
                 {"--config-file FILE", "parse options from FILE"});

  Parser sub("\nsub command and options\n",
             "Using -- to toggle sub option off.\n\n");

  app.add_parser(sub, 1);
  sub.toggle();

  Matcher m(value().apply([](std::string a) { return a == "sub" ? 1 : 0; }));

  app.add_option(
      value().apply([](std::string a) { return a == "sub" ? 1 : 0; }),
      [&]() { sub.enable(); }, {"sub", "toggle sub command on"});

  sub.add_option(value().apply([](std::string a) { return a == "--" ? 1 : 0; }),
                 [&]() { sub.disable(); },
                 {"--", "close sub command"})->help_level = 10000;
  sub.add_help();

  sub.add_option("--sub-ab <str> <str>", [](std::string a, std::string b) {
                                           std::cout << "sub-ab got arg " << a
                                                     << " and " << b
                                                     << std::endl;
                                         },
                 "print and eat the following two arguments");

  sub.add_option(
      "-a <str>",
      [](std::string a) { std::cout << "a in sub got arg " << a << std::endl; },
      "print and eat the following argument");

  int int_value = 13;
  bool flag = false;

  app.add_help();

  app.add_option(
      "--int <int>", &int_value,
      "Current: " + delay_to_str(&int_value) + "\nSet integer value.");

  app.add_option("--no-flag, --flag, --flag-on, --flag-off",
                 bundle({{"--flag", [&]() { flag = true; }, {}},
                         {"--no-flag", [&]() { flag = false; }, {}},
                         {"--flag-on", [&]() { flag = true; }, {}},
                         {"--flag-off", [&]() { flag = false; }, {}}}),
                 "turn flag on or off");

  app.add_option("|no-flag|flag|flag-on|flag-off",
                 bundle({{"|flag", [&]() { flag = true; }, {}},
                         {"|no-flag", [&]() { flag = true; }, {}},
                         {"|flag-on", [&]() { flag = true; }, {}},
                         {"|flag-off", [&]() { flag = true; }, {}}}),
                 {"flag, no-flag, flag-on, flag-off", [&]() {
                   return std::string("Current: ") + to_str(flag) +
                          "\nturn flag on or off";
                 }});

  app.add_option("--func <str> <str>", [](std::string a, std::string b) {
                                         std::cout << "func got arg " << a
                                                   << " and " << b << std::endl;
                                       },
                 "Take and print the two arguments");

  app.add_option("--func-si <str> <int>", [](std::string a, int b) {
                                            std::cout << "func-si got arg " << a
                                                      << " and " << b
                                                      << std::endl;
                                          },
                 "Take and print the two arguments");

  app.add_option("--value-ab A B",
                 gather(value(), value())
                     .apply(check_invoke([](std::string a, std::string b) {
                        std::cerr << "value-ab a " << a << std::endl;
                        std::cerr << "value-ab b " << b << std::endl;
                      })),
                 "take and print the two arguments");

  app.add_option("--one-int <integer>", value<int>().apply([](int v) {
                                          std::cerr << "one-int " << v
                                                    << std::endl;
                                        }),
                 "one integer");

  app.add_option("--all-int <integer>...",
                 many(value<int>(), 1, 4).apply([](std::vector<int> vs) {
                   for (size_t i = 0; i < vs.size(); ++i) {
                     std::cerr << "all-int " << i << " value " << vs[i]
                               << std::endl;
                   }
                 }),
                 "all following integers");

  auto parse_result = app.parse(argc, argv);

  if (parse_result.error) {
    std::cerr << *parse_result.error.get() << std::endl;
    if (parse_result.error_full) {
      std::cerr << *parse_result.error_full.get() << std::endl;
    }
    return 1;
  }

  std::cerr << "int_value " << int_value << std::endl;
  std::cerr << "flag " << flag << std::endl;
  std::cerr << "flag_int " << FLAGS_flag_int << std::endl;
}
