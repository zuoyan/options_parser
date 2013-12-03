#include <iostream>
#include <errno.h>
#include <unistd.h>

#include "options_parser/options_parser.h"

int main(int argc, char *argv[]) {
  options_parser::Parser cli;

  cli.add_help();
  bool print_args = false;

  cli.add_option("print-args", [&]() { print_args = true; }, {});

  options_parser::Parser get_cli("apt-get Commands:", "");
  cli.add_parser(get_cli);

  std::vector<std::string> result;

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
                         result.push_back("apt-get");
                         result.push_back(c);
                         cli.toggle();
                                                             },
                       {c, c_h.second});
  }

  options_parser::Parser cache_cli("apt-cache Commands:", "");
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
    cache_cli.add_option({c, {}, {}, options_parser::value()}, [&, c]() {
                     result.push_back("apt-cache");
                     result.push_back(c);
                     cli.toggle();
                                                               },
                         {c, c_h.second});
  }

  options_parser::ArgcArgvArguments arguments(argc, argv);
  auto parse_result = cli.parse({{1, 0}, &arguments});
  if (parse_result.error &&
      (!result.size() || *parse_result.error.get() != "match-none")) {
    std::cerr << *parse_result.error.get() << std::endl;
    if (parse_result.error_full) {
      std::cerr << *parse_result.error_full.get() << std::endl;
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
  for (const auto & a : result) {
    result_argv.push_back(a.c_str());
  }
  result_argv.push_back(nullptr);

  execvp(result_argv[0], (char * const *)&result_argv[0]);
  std::cerr << "execvp failed " << strerror(errno) << std::endl;
  return 1;
}
