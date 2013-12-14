#include "options_parser/options_parser_lib.h"

#include <map>

int main(int argc, char* argv[]) {
  options_parser::Parser app("test git like commands", "...");
  app.add_parser(options_parser::parser());
  app.add_help("--help");
  app.add_flag("--version", "Print version and exit");
  app.add_flag("-C <path>", "Run as if git was started ...");

  std::map<std::string, options_parser::Parser> subs;

  app.add_option(
      options_parser::value().peek().apply([&](std::string arg)->int {
        if (subs.count(arg)) {
          return options_parser::MATCH_EXACT;
        }
        if (!arg.size()) return 0;
        std::string choice;
        for (const auto& sub_parser : subs) {
          const auto & sub = sub_parser.first;
          if (arg.size() < sub.size() && sub.compare(0, arg.size(), arg) == 0) {
            if (choice.size()) return 0;
            choice = sub;
          }
        }
        return choice.size() ? options_parser::MATCH_POSITION : 0;
      }),
      [&](std::string cmd) {
        if (subs.count(cmd)) {
                  app.add_parser(subs[cmd], 1);
        } else {
          for (const auto & sub_parser : subs) {
            if (sub_parser.first.size() > cmd.size()
                && sub_parser.first.compare(0, cmd.size(), cmd) == 0) {
              app.add_parser(sub_parser.second, 1);
              break;
            }
          }
        }
        subs.clear();
      },
      {"<sub command>", "one of sub commands"});

  options_parser::Parser add("add - Add file contents to the index", "\n");
  subs["add"] = add;
  std::string add_flags = R"FLAGS(
-n, --dry-run
    Don’t actually add the file(s), just show if they exist and/or
    will be ignored.

-v, --verbose  Be verbose.

-f, --force  Allow adding otherwise ignored files.

-i, --interactive
    Add modified contents in the working tree interactively to the
    index. Optional path arguments may be supplied to limit operation
    to a subset of the working tree. See “Interactive mode” for
    details.

-p, --patch
    Interactively choose hunks of patch between the index and the work
    tree and add them to the index. This gives the user a chance to
    review the difference before adding modified contents to the
    index.

-e, --edit
    Open the diff vs. the index in an editor and let the user edit
    it. After the editor was closed, adjust the hunk headers and apply
    the patch to the index.

)FLAGS";
  add.add_flags_lines(options_parser::split(add_flags, "\n"), 4);

  options_parser::Parser commit("commit - Record changes to the repository", "\n");
  subs["commit"] = commit;
  std::string commit_flags = R"FLAGS(
-a, --all
    Tell the command to automatically stage files that have been
    modified and deleted, but new files you have not told Git about
    are not affected.

-p, --patch
    Use the interactive patch selection interface to chose which
    changes to commit. See git-add(1) for details.

-C <commit>, --reuse-message=<commit>
    Take an existing commit object, and reuse the log message and the
    authorship information (including the timestamp) when creating the
    commit.

-c <commit>, --reedit-message=<commit>
    Like -C, but with -c the editor is invoked, so that the user can
    further edit the commit message.

--fixup=<commit>
    Construct a commit message for use with rebase --autosquash. The
    commit message will be the subject line from the specified commit
    with a prefix of 'fixup! '. See git-rebase(1) for details.)FLAGS";
  commit.add_flags_lines(options_parser::split(commit_flags, "\n"), 4);

  options_parser::Parser status("status - Show the working tree status", "\n");
  subs["status"] = status;
  std::string status_flags = R"FLAGS(
-s, --short  Give the output in the short-format.

-b, --branch  Show the branch and tracking info even in short-format.

--porcelain
    Give the output in an easy-to-parse format for scripts. This is
    similar to the short output, but will remain stable across Git
    versions and regardless of user configuration. See below for
    details.

--long  Give the output in the long-format. This is the default.

-u[<mode>], --untracked-files[=<mode>]  Show untracked files.

--ignore-submodules[=<when>]
    Ignore changes to submodules when looking for changes.

--ignored  Show ignored files as well.

-z
    Terminate entries with NUL, instead of LF. This implies the
    --porcelain output format if no other format is given.

--column[=<options>], --no-column
    Display untracked files in columns. See configuration variable
    column.status for option syntax.--column and --no-column without
    options are equivalent to always and never respectively.  )FLAGS";
  status.add_flags_lines(options_parser::split(status_flags, "\n"), 4);

  options_parser::Parser branch("branch - List, create, or delete branches", "\n");
  subs["branch"] = branch;
  std::string branch_flags = R"FLAGS(
-d, --delete
    Delete a branch. The branch must be fully merged in its upstream
    branch, or in HEAD if no upstream was set with --track or
    --set-upstream.

-D
    Delete a branch irrespective of its merged status.

-l, --create-reflog
    Create the branch’s reflog. This activates recording of all
    changes made to the branch ref, enabling use of date based sha1
    expressions such as '<branchname>@{yesterday}'. Note that in
    non-bare repositories, reflogs are usually enabled by default by
    the core.logallrefupdates config option.

-f, --force
    Reset <branchname> to <startpoint> if <branchname> exists
    already. Without -fgit branch refuses to change an existing
    branch.

-m, --move
    Move/rename a branch and the corresponding reflog.

-M
    Move/rename a branch even if the new branch name already exists.

--color[=<when>]
    Color branches to highlight current, local, and remote-tracking
    branches. The value must be always (the default), never, or auto.

--no-color
    Turn off branch colors, even when the configuration file gives the
    default to color output. Same as --color=never.
)FLAGS";
  branch.add_flags_lines(options_parser::split(branch_flags, "\n"), 4);

  auto parse_result = app.parse(argc, argv);

  if (parse_result.error) {
    std::cerr << "parse-error: " << *parse_result.error.get() << std::endl;
    if (parse_result.error_full) {
      std::cerr << *parse_result.error_full.get() << std::endl;
    }
  }

  std::cout << parse_result.situation.circumstance.to_str() << std::endl;

  return 0;
}
