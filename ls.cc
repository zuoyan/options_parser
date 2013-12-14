#include "options_parser/options_parser_lib.h"
#include "options_parser/any.h"

#include <map>

int main(int argc, char* argv[]) {
  options_parser::Parser app(
      "Usage: ls [OPTION]... [FILE]...\n"
      "List information about the FILEs (the current directory by default).\n"
      "Sort entries alphabetically if none of -cftuvSUX nor --sort is "
      "specified.",
      "SIZE may be (or may be an integer optionally followed by) one of "
      "following:"
      " KB 1000, K 1024, MB 1000*1000, M 1024*1024, and so on for G, T, P, E, "
      "Z, Y.\n"
      "\n"
      "Using color to distinguish file types is disabled both by default and"
      " with --color=never.  With --color=auto, ls emits color codes only when"
      " standard output is connected to a terminal.  The LS_COLORS environment"
      " variable can change the settings.  Use the dircolors command to set "
      "it.\n"
      "\n"
      "Exit status:\n"
      " 0  if OK,\n"
      " 1  if minor problems (e.g., cannot access subdirectory),\n"
      " 2  if serious trouble (e.g., cannot access command-line argument).\n"
      "\n"
      "Report ls bugs to bug-coreutils@gnu.org\n"
      "GNU coreutils home page: <http://www.gnu.org/software/coreutils/>\n"
      "General help using GNU software: <http://www.gnu.org/gethelp/>\n"
      "For complete documentation, run: info coreutils 'ls invocation'\n");
  app.add_parser(options_parser::parser());
  std::string flags = R"FLAGS(
-a, --all                  do not ignore entries starting with .
-A, --almost-all           do not list implied . and ..
    --author               with -l, print the author of each file
-b, --escape               print C-style escapes for nongraphic characters
    --block-size=SIZE      scale sizes by SIZE before printing them.  E.g.,
                             `--block-size=M' prints sizes in units of
                             1,048,576 bytes.  See SIZE format below.
-B, --ignore-backups       do not list implied entries ending with ~
-c                         with -lt: sort by, and show, ctime (time of last
                             modification of file status information)
                             with -l: show ctime and sort by name
                             otherwise: sort by ctime, newest first
-C                         list entries by columns
    --color[=WHEN]         colorize the output.  WHEN defaults to `always'
                             or can be `never' or `auto'.  More info below
-d, --directory            list directory entries instead of contents,
                             and do not dereference symbolic links
-D, --dired                generate output designed for Emacs' dired mode
-f                         do not sort, enable -aU, disable -ls --color
-F, --classify             append indicator (one of */=>@|) to entries
    --file-type            likewise, except do not append `*'
    --format=WORD          across -x, commas -m, horizontal -x, long -l,
                             single-column -1, verbose -l, vertical -C
    --full-time            like -l --time-style=full-iso
-g                         like -l, but do not list owner
    --group-directories-first
                           group directories before files.
                             augment with a --sort option, but any
                             use of --sort=none (-U) disables grouping
-G, --no-group             in a long listing, don't print group names
-h, --human-readable       with -l, print sizes in human readable format
                             (e.g., 1K 234M 2G)
    --si                   likewise, but use powers of 1000 not 1024
-H, --dereference-command-line
                           follow symbolic links listed on the command line
    --dereference-command-line-symlink-to-dir
                           follow each command line symbolic link
                           that points to a directory
    --hide=PATTERN         do not list implied entries matching shell PATTERN
                             (overridden by -a or -A)
    --indicator-style=WORD  append indicator with style WORD to entry names:
                             none (default), slash (-p),
                             file-type (--file-type), classify (-F)
-i, --inode                print the index number of each file
-I, --ignore=PATTERN       do not list implied entries matching shell PATTERN
-k                         like --block-size=1K
-l                         use a long listing format
-L, --dereference          when showing file information for a symbolic
                             link, show information for the file the link
                             references rather than for the link itself
-m                         fill width with a comma separated list of entries
-n, --numeric-uid-gid      like -l, but list numeric user and group IDs
-N, --literal              print raw entry names (don't treat e.g. control
                             characters specially)
-o                         like -l, but do not list group information
-p, --indicator-style=slash
                           append / indicator to directories
-q, --hide-control-chars   print ? instead of non graphic characters
    --show-control-chars   show non graphic characters as-is (default
                           unless program is `ls' and output is a terminal)
-Q, --quote-name           enclose entry names in double quotes
    --quoting-style=WORD   use quoting style WORD for entry names:
                             literal, locale, shell, shell-always, c, escape
-r, --reverse              reverse order while sorting
-R, --recursive            list subdirectories recursively
-s, --size                 print the allocated size of each file, in blocks
-S                         sort by file size
    --sort=WORD            sort by WORD instead of name: none -U,
                           extension -X, size -S, time -t, version -v
    --time=WORD            with -l, show time as WORD instead of modification
                           time: atime -u, access -u, use -u, ctime -c,
                           or status -c; use specified time as sort key
                           if --sort=time
    --time-style=STYLE     with -l, show times using style STYLE:
                           full-iso, long-iso, iso, locale, +FORMAT.
                           FORMAT is interpreted like `date'; if FORMAT is
                           FORMAT1<newline>FORMAT2, FORMAT1 applies to
                           non-recent files and FORMAT2 to recent files;
                           if STYLE is prefixed with `posix-', STYLE
                           takes effect only outside the POSIX locale
-t                         sort by modification time, newest first
-T, --tabsize=COLS         assume tab stops at each COLS instead of 8
-u                         with -lt: sort by, and show, access time
                             with -l: show access time and sort by name
                             otherwise: sort by access time
-U                         do not sort; list entries in directory order
-v                         natural sort of (version) numbers within text
-w, --width=COLS           assume screen width instead of current value
-x                         list entries by lines instead of by columns
-X                         sort alphabetically by entry extension
-Z, --context              print any SELinux security context of each file
-1                         list one file per line
    --version  output version information and exit
)FLAGS";

  app.add_flags_lines(options_parser::split(flags, "\n"));

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

  std::cout << parse_result.situation.circumstance.to_str() << std::endl;

  return 0;
}
