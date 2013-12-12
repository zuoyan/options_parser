#include "options_parser/options_parser_lib.h"
#include "options_parser/any.h"

#include <map>

int main(int argc, char* argv[]) {
  options_parser::Parser app("ls", "try ls command");
  options_parser::Circumstance circumstance;
  circumstance.init();
  app.set_circumstance(circumstance);

  app.add_parser(options_parser::parser());

  app.add_flag("a|all", "do not ignore entries starting with .");

  app.add_flag("A|almost-all", "do not list implied . and ..");

  app.add_flag("author", "with -l, print the author of each file");
  app.add_flag("b|escape", "print C-style escapes for nongraphic characters");

  app.add_flag("--block-size=SIZE",
               "scale sizes by SIZE before printing them.  E.g.,"
               " `--block-size=M' prints sizes in units of"
               " 1,048,576 bytes.  See SIZE format below.");

  app.add_flag("B|ignore-backups", "do not list implied entries ending with ~");
  app.add_flag("c",
               "with -lt: sort by, and show, ctime (time of last"
               " modification of file status information)"
               " with -l: show ctime and sort by name"
               " otherwise: sort by ctime, newest first");
  app.add_flag("C", "list entries by columns");

  app.add_flag("--color=[WHEN]",
               "colorize the output.  WHEN defaults to 'always'"
               " or can be 'never' or 'auto'.  More info below",
               std::string("always"));

  app.add_flag("d|directory",
               "list directory entries instead of contents,"
               " and do not dereference symbolic links");

  app.add_flag("D|dired",
               "generate output designed for Emacs' dired mode");

  app.add_option("-f", [&]() {
                         *circumstance.flag<bool>("all") = false;
                         *circumstance.flag<bool>("sort") = false;
                         *circumstance.flag<bool>("list") = false;
                         *circumstance.flag<bool>("size") = 0;
                         *circumstance.flag<std::string>("color") =
                             std::string("never");
                       },
                 "do not sort, enable -aU, disable -ls --color");

  app.add_flag("F|classify",
               "append indicator (one of */=>@|) to entries");

  app.add_flag("file-type", "likewise, except do not append '*'");

  app.add_flag<std::string>("--format=WORD",
                             "across -x, commas -m, horizontal -x, long -l,"
                             " single-column -1, verbose -l, vertical -C");

  app.add_flag("full-time", "like -l --time-style=full-iso");

  app.add_flag("g", "like -l, but do not list owner");

  app.add_flag("group-directories-first",
               "group directories before files."
               " augment with a --sort option, but any"
               " use of --sort=none (-U) disables grouping");

  app.add_flag("G|no-group",
               "in a long listing, don't print group names");

  app.add_flag("h|human-readable",
               "with -l, print sizes in human readable format"
               " (e.g., 1K 234M 2G)");

  app.add_flag("si", "likewise, but use powers of 1000 not 1024");

  app.add_flag("H|dereference-command-line",
               "follow symbolic links listed on the command line");

  app.add_flag("dereference-command-line-symlink-to-dir",
               "follow each command line symbolic link"
               " that points to a directory");

  app.add_flag<std::string>("--hide=PATTERN",
                            "do not list implied entries matching shell PATTERN"
                            " (overridden by -a or -A)");

  app.add_flag<std::string>("--indicator-style=WORD",
                             "append indicator with style WORD to entry names:"
                             " none (default), slash (-p),"
                            " file-type (--file-type), classify (-F)");

  app.add_flag("i|inode", "print the index number of each file");

  app.add_flag<std::string>(
      "-I, --ignore=PATTERN",
      "do not list implied entries matching shell PATTERN");

  app.add_option(
      "-k|--kibibytes",
      [&]() { *circumstance.flag<size_t>("block-size") = (size_t)1024; },
      "use 1024-byte blocks");

  app.add_flag("l", "use a long listing format");

  app.add_flag("L|dereference",
               "when showing file information for a symbolic"
               " link, show information for the file the link"
               " references rather than for the link itself");
  app.add_flag("m", "fill width with a comma separated list of entries");
  app.add_flag("n|numeric-uid-gid",
               "like -l, but list numeric user and group IDs");

  app.add_flag("N|literal", "print raw entry names (don't treat e.g. control"
               " characters specially)");
  app.add_flag("o", "like -l, but do not list group information");
  app.add_flag("-p, --indicator-style=slash",
               "append / indicator to directories");

  app.add_flag("q|hide-control-chars", "print ? instead of non graphic characters");

  app.add_flag("show-control-chars", "show non graphic characters as-is (default"
               " unless program is `ls' and output is a terminal)");

  app.add_flag("Q|quote-name", "enclose entry names in double quotes");
  app.add_flag<std::string>("--quoting-style=WORD",
                            "use quoting style WORD for entry names:"
                            " literal, locale, shell, shell-always, c, escape");

  app.add_flag("r|reverse", "reverse order while sorting");
  app.add_flag("R|recursive", "list subdirectories recursively");
  app.add_flag("s|size", "print the allocated size of each file, in blocks");
  app.add_flag("S", "sort by file size");
  app.add_flag<std::string>("--sort=WORD",
                            "sort by WORD instead of name: none -U,"
                            " extension -X, size -S, time -t, version -v");
  app.add_flag<std::string>("--time=WORD",
                            "with -l, show time as WORD instead of modification"
                            " time: atime -u, access -u, use -u, ctime -c,"
                            " or status -c; use specified time as sort key"
                            " if --sort=time");
  app.add_flag<std::string>("--time-style=STYLE",
                            "with -l, show times using style STYLE:"
                            " full-iso, long-iso, iso, locale, +FORMAT."
                            " FORMAT is interpreted like `date'; if FORMAT is"
                            " FORMAT1<newline>FORMAT2, FORMAT1 applies to"
                            " non-recent files and FORMAT2 to recent files;"
                            " if STYLE is prefixed with `posix-', STYLE"
                            " takes effect only outside the POSIX locale");
  app.add_option("-t",
                 [&]() { *circumstance.flag<std::string>("sort") = "time"; },
                 "sort by modification time, newest first");

  app.add_flag<size_t>("-T, --tabsize=COLS",
                       "assume tab stops at each COLS instead of 8");
  app.add_flag("u",
               "with -lt: sort by, and show, access time, "
               "with -l: show access time and sort by name,"
               " otherwise: sort by access time");

  app.add_option("U", [&]() { *circumstance.flag<std::string>("sort") = ""; },
                 "do not sort; list entries in directory order");

  app.add_option(
      "v",
      [&]() { *circumstance.flag<std::string>("sort") = "natural-number"; },
      "natural sort of (version) numbers within text");

  app.add_flag<size_t>("-w, --width=COLS",
                       "assume screen width instead of current value");

  app.add_flag("x", "list entries by lines instead of by columns");

  app.add_option(
      "X",
      [&]() { *circumstance.flag<std::string>("sort") = "alphabetically"; },
      "sort alphabetically by entry extension");

  app.add_flag(
      "Z|context", "print any SELinux security context of each file");

  app.add_flag("1", "list one file per line");

  app.add_option("--version", []() {
                                std::cout
                                    << "ls, demo of options_parser 0.01 ..."
                                    << std::endl;
                                exit(1);
                              },
                 "output version information and exit");

  app.add_help("--help");

  app.parse(argc, argv);

  auto parse_result = app.parse(argc, argv);

  if (parse_result.error) {
    std::cerr << *parse_result.error.get() << std::endl;
    if (parse_result.error_full) {
      std::cerr << *parse_result.error_full.get() << std::endl;
    }
    return 1;
  }

  std::cout << circumstance.to_str() << std::endl;

  return 0;
}
