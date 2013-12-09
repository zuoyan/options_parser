#include <iostream>
#include <errno.h>
#include <unistd.h>

#include "options_parser/options_parser.h"

int main(int argc, char *argv[]) {
  options_parser::Parser cli;

  cli.add_help();
  bool print_args = false;

  std::vector<std::shared_ptr<options_parser::Option>> common_options;
  common_options.push_back(cli.add_option(
      "print-args", [&]() { print_args = true; }, {}));
  std::string result_command;
  std::vector<std::string> result;

  common_options.push_back(
      cli.add_option("c", [&](std::string a) {
          result.push_back("-c");
          result.push_back(a);
        },
        {"-c=?", "Read this configuration file."}));

  common_options.push_back(cli.add_option(
      "o", [&](std::string a) {
        result.push_back("-o");
        result.push_back(a);
      },
      {"-o=?", "Set an arbitrary configuration option, eg -o dir::cache=/tmp"}));

  options_parser::Parser get_cli("apt-get Commands:", "\n");
  cli.add_parser(get_cli);

  std::vector<std::pair<std::string, std::string>> get_commands{
      {"update", "Retrieve new lists of packages"},
      {"upgrade", "Perform an upgrade"},
      {"install", "Install new packages (pkg is libc6 not libc6.deb)"},
      {"remove", "Remove packages"},
      {"autoremove", "Remove automatically all unused packages"},
      {"purge", "Remove packages and config files"},
      {"source", "Download source archives"},
      {"build-dep", "Configure build-dependencies for source packages"},
      {"dist-upgrade", "Distribution upgrade, see apt-get(8)"},
      {"dselect-upgrade", "Follow dselect selections"},
      {"clean", "Erase downloaded archive files"},
      {"autoclean", "Erase old downloaded archive files"},
      {"check", "Verify that there are no broken dependencies"},
      {"changelog", "Download and display the changelog for the given package"},
      {"download", "Download the binary package into the current directory"}};

  for (auto c_h : get_commands) {
    auto c = c_h.first;
    get_cli.add_option({c, {}, {}, options_parser::value()}, [&, c]() {
                         result_command = "apt-get";
                         result.push_back(c);
                         cli.toggle();
                         get_cli.toggle();
                                                             },
                       {c, c_h.second});
  }

  get_cli.add_option("q", [&]() {
      result.push_back("-q");
    }, {"-q", "Loggable output - no progress indicator"});

  get_cli.add_option("qq", [&]() {
      result.push_back("-qq");
    }, {"-qq", "No output except for errors"});

  get_cli.add_option("d", [&]() {
      result.push_back("-d");
    }, {"-d", "Download only - do NOT install or unpack archives"});

  get_cli.add_option("s", [&]() {
      result.push_back("-s");
    }, {"-s", "No-act. Perform ordering simulation"});

  get_cli.add_option("y", [&]() {
      result.push_back("-y");
    }, {"-y", "Assume Yes to all queries and do not prompt"});

  get_cli.add_option("f", [&]() {
      result.push_back("-f");
    }, {"-f", "Attempt to correct a system with broken dependencies in place"});

  get_cli.add_option("m", [&]() {
      result.push_back("-m");
    }, {"-m", "Attempt to continue if archives are unlocatable"});

  get_cli.add_option("u", [&]() {
      result.push_back("-u");
    }, {"-u", "Show a list of upgraded packages as well"});

  get_cli.add_option("b", [&]() {
      result.push_back("-b");
    }, {"-b", "Build the source package after fetching it"});

  get_cli.add_option("V", [&]() {
      result.push_back("-V");
    }, {"-V", "Show verbose version numbers"});

  options_parser::Parser cache_cli("apt-cache Commands:", "\n");
  cli.add_parser(cache_cli);

  std::vector<std::pair<std::string, std::string>> cache_commands{
      {"gencaches", "Build both the package and source cache"},
      {"showpkg", "Show some general information for a single package"},
      {"showsrc", "Show source records"},
      {"stats", "Show some basic statistics"},
      {"dump", "Show the entire file in a terse form"},
      {"dumpavail", "Print an available file to stdout"},
      {"unmet", "Show unmet dependencies"},
      {"search", "Search the package list for a regex pattern"},
      {"show", "Show a readable record for the package"},
      {"depends", "Show raw dependency information for a package"},
      {"rdepends", "Show reverse dependency information for a package"},
      {"pkgnames", "List the names of all packages in the system"},
      {"dotty", "Generate package graphs for GraphViz"},
      {"xvcg", "Generate package graphs for xvcg"},
      {"policy", "Show policy settings"}};

  for (auto c_h : cache_commands) {
    auto c = c_h.first;
    cache_cli.add_option({c, {}, {}, options_parser::value()},
                         [&, c]() {
                           result_command = "apt-cache";
                           result.push_back(c);
                           cli.toggle();
                           cache_cli.toggle();
                         },
                         {c, c_h.second});
  }

  cache_cli.add_option("p", [&](std::string a) {
      result.push_back("-p");
      result.push_back(a);
    }, {"-p=?", "The package cache."});

  cache_cli.add_option("s", [&](std::string a) {
      result.push_back("-s");
      result.push_back(a);
    }, {"-s=?", "The source cache."});

  cache_cli.add_option("q", [&]() {
      result.push_back("-q");
    }, {"-q", "Disable progress indicator."});

  cache_cli.add_option("i", [&]() {
      result.push_back("-i");
    }, {"-i", "Show only important deps for the unmet command."});

  for (auto o : common_options) {
    get_cli.add_option(*o);
    cache_cli.add_option(*o);
  }

  options_parser::ArgvArguments arguments(argc, argv);
  auto parse_result = cli.parse({{1, 0}, arguments});

  if (!result_command.size()) {
    std::vector<std::string> ar, gr, cr;
    std::string ac, gc, cc;
    ar.swap(result);
    ac.swap(result_command);

    auto less = [](options_parser::Position a,  options_parser::Position b) {
      return a.index < b.index || (a.index == b.index && a.off < b.off);
    };

    result.clear();
    auto gpr = get_cli.parse({{1, 0}, arguments});
    if (less(parse_result.position, gpr.position)) {
      parse_result = gpr;
      ar.swap(result);
      ac.swap(result_command);
    }

    result.clear();
    auto cpr = cache_cli.parse({{1, 0}, arguments});
    if (less(parse_result.position, cpr.position)) {
      parse_result = cpr;
      ar.swap(result);
      ac.swap(result_command);
    }

    std::swap(ar, result);
    std::swap(ac, result_command);
  }

  if (parse_result.error && *parse_result.error.get() != "match-none") {
    std::cerr << *parse_result.error.get() << std::endl;
    if (parse_result.error_full) {
      std::cerr << *parse_result.error_full.get() << std::endl;
    }
    return 1;
  }

  if (!result_command.size()) {
    std::cerr << "give me a command ..." << std::endl;
    if (parse_result.error) {
      std::cerr << *parse_result.error.get() << std::endl;
      if (parse_result.error_full) {
        std::cerr << *parse_result.error_full.get() << std::endl;
      }
    }
    return 1;
  }

  options_parser::many(options_parser::value().apply([&](std::string arg) {
    result.push_back(arg);
  }))({parse_result.position, parse_result.args});

  if (print_args) {
    for (auto c : result) {
      std::cout << c << std::endl;
    }
    return 0;
  }

  std::vector<const char*> result_argv;
  result_argv.push_back(result_command.c_str());
  for (const auto & a : result) {
    result_argv.push_back(a.c_str());
  }
  result_argv.push_back(nullptr);

  execvp(result_argv[0], (char * const *)&result_argv[0]);
  std::cerr << "execvp failed " << strerror(errno) << std::endl;
  return 1;
}
