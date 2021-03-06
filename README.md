# options parser for C++0x

This's a C++0x library for parsing options/command line arguments. Every option
is viewed as Match, Take, and Document. Generally, Match is a function from
current state(Position and Arguments) to newer state and Priority without side
effects. Take is a function from current state to newer state with side
effects. Document is used to show help message, or parsing error.

This library has simpified routines for general match and take, see
options_parser_test.cc and apt.cc for usage examples.

## Consideration

Match can be any functions, not just string match, and Take can be any
functions, not just assignments. I think, this approach is more flexible than
gflags, and getopt.

One of most used limits of options is order independent. But this library
processing options in order and stopped at first not konwn/failed options.

The most powerful design for options processing is using a real dynamic parser,
or parser DSL library. User just defines whole the syntax with embeded
actions. But this's a little hard to implement and even harder, i think, to make
it easy to use.

## Build

This library based on CMake, and the library can be build as static/shared
library, or header only.

To build example and library,

```bash
mkdir build && cd build # in options_parser directory
cmake ../
make
```

## Examples

For basic example, see file [stats.cc](https://github.com/zuoyan/options_parser/blob/master/stats.cc) and [ls.cc](https://github.com/zuoyan/options_parser/blob/master/ls.cc)

This library can parse most program --help message as flags, as demoed in
[ls.cc](https://github.com/zuoyan/options_parser/blob/master/ls.cc).

This library can also parse configuration file lines, as demoed in
[options_parser_test.cc:ConfigFile](https://github.com/zuoyan/options_parser/blob/master/options_parser_test.cc).

Fore more, please check some example files
[example_basic.cc](https://github.com/zuoyan/options_parser/blob/master/example_basic.cc),
[secure_sharing.cc](https://github.com/zuoyan/options_parser/blob/master/secure_sharing.cc),
[pjobs.cc](https://github.com/zuoyan/options_parser/blob/master/pjobs.cc),
[apt.cc](https://github.com/zuoyan/options_parser/blob/master/apt.cc).


```C++
options_parser::Parser parser("usage and info", "further info ...");
parser.add_parser(options_parser::parser()); // add default global parser
parser.add_option("-i, --int NUM", &vi, assigned int value");
parser.add_option("-s STRING --str STRING", &vs, "assigned string value");
parser.add_option("-f, --func <int-arg> <string-arg> <double-arg>",
  [&](int vi, std::string vs, double vd) {
     std::cerr << "vi " << vi << std::endl;
     std::cerr << "vs " << vs << std::endl;
     std::cerr << "vd " << vd << std::endl;
  },
  "call the function over arguments");
```

## Document

If we think the options parser from a very general view, and don't introduce any
special cases. Then, the options parser has to be a really simple parser,
consume current input, find and take the right action, then loop back to consume
input, until all input consumed, or, an error occured.

A (options) parser is a function consume input(argv) and run actions.

I think it's the rule works for all options.(Not true, It doesn't work
well for context sensitive options).

For convenience, we pack the input to options parser in one structure
`Situation`, combined by arguments, position in arguments and variable store
place, `Circumstance`, and current parser, option pointer.

To add options, we could just add functions of type `Situation -> (value,
Situation)`. But in a real imparative world, we can't just run the funcitons one
by one, catch the exceptions, and rollback the state. We can rollback the
arguments(just used old value again), but we can't rollback all unknown side
effects. Or, we can make it a rule, every function should rollback itself if it
doesn't happy with current `Situation`. But to allow ambiguous options, the
funciton doesn't know it should consume the current `Situation` and take the
real action(which may be not rollbackable), or give the chance to others.

In short, we have to find the unique function to execute at every
`Situation`. An easy way is splitting every parsing function to two parts,
`Matcher` and `Taker`. The `Matcher` consumes `Situation` and returns *priority*
with newer `Situation`. It should have no side effects. And the `Taker` consumes
`Situation` and gives a value(interpreted as error message, if not
*empty*/*negative*) with newer `Situation`. At any `Situation`, all `Matcher`s
are called, but at most one `Taker` is called.

`Matcher` and `Taker` have a common pattern, that, there're functions taking
`Situation` and returning pair of value(of some type), and `Situation`. A
function taking `S`, returning pair of `V`, and `S` is called
[state monad](http://en.wikipedia.org/wiki/State_monad#State_monads). We denote
it by `state<V, S>`, and implement it in file
[`options_parser/state.h`](https://github.com/zuoyan/options_parser/blob/master/options_parser/state.h).
state monads are used very often in parsing, to abstract away input consuming
from parsers combination.

We have model the options `Parser` as

```C++
struct Parser {
  OptionPtr add_option(Matcher matcher, Taker taker, Document document);
  ParseResult parse(Situation);
};
```

In error cases, the `Parser` will give error messages like,
`match-none`, `match-multiple`, `take-error`, with details.

It's not easy to use, and it's not descriptive. We have to do more
than just functions.

`Matcher` should be constructable from a description string, a state monad with
priority as value. For example, A string `-l N, --line-length=N` or `-l,
--line-length=N` means accept "-l 8128", "-l=8128", "--line-length 8128" and
"--line-length=8128".

`Taker` should be constructable from a value pointer, a function, and a state
monad. With a value pointer, the `Taker` should assign the value converted from
next argument in `Situation` to the destination of the pointer. With a function,
the `Taker` should consume a number, the arity of the function, arguments from
the `Situation`. And convert the arguments to the parameters of the function one
by one, invoke the function, and return the value and the moved
`Situation`. With a state moand, the `Taker` just calls the function wrapped in
the state monad.

`Document` is any function taking `int` argument as width of console, and return
vector of string as lines formated. When `Matcher` is constructed from a string,
`Document` should take the same string as match pattern.

A classical example:

```C++
Parser parser;
int flag = 0;
double x = 3.14;
double y = 1.414;
parser.add_option("-f, --flag[=true/flase]",
  value<bool>().optional().apply([&](Maybe<bool> f) {
    flag = f ? get_value(f) : true;
  }, "toggle flag on")
parser.add_option("--no-flag", [&]() {flag = 0;}, "toggle flag off")
parser.add_option("-x, --x-value FLOAT", &x, "x value");
parser.add_option("-y, --y-value FLOAT", &y, "y value");
parser.add_option("--xy FLOAT FLOAT", [&](double x_, double y_) {
  x = x_; y = y_; }, "x and y");
parser.add_flag("--verbose", "bool flag verbose");
parser.add_flag<int>("--int-verbose INT", "flag with int argument")
parser.add_flag("--style NAME", "flag with string argument")
```

A example, to allow variadic arguments:

```C++
parser.add_option("--files FILE...",
                 value().not_option().apply([](std::string fn) {
                   std::cerr << "--files take file: " << fn << std::endl;
                 }).many(),
                 "all following not starting with '-' are taken as FILE");
```

We can consume argument of type `T` by `value<T>()`, it's a type
`Value<T>` which inherits from state monad `state<T,
Situation>`. `not_option` adds check to accept arguments not starting
with '-'(except after '=').

A another variadic demo example:

```C++
parser.add_option(
    "--env-run <name=value>... <--sep> <cmd-arg>... <--sep>",
    value_gather(value().not_option().many(),
                 value().bind([](std::string sep) {
                   return value()
                       .check([sep](std::string a) { return a != sep; })
                       .many()
                       .bind([](std::vector<std::string> vs) {
                         return value().apply([vs](std::string) { return vs; });
                       });
                 })).apply(check_invoke([](std::vector<std::string> envs,
                                           std::vector<std::string> args) {
      std::cerr << "env-run envs " << join(envs, " | ") << std::endl;
      std::cerr << "env-run args " << join(args, " | ") << std::endl;
    })),
    "Run <cmd-arg>... with environments <name=value>...");
```

Another requirement is dynamic. The `Parser` runs the `Taker` before
moving to next position. So we can registered new options in the
`Taker` functions. This is very useful for plugins to insert options
dynamically.

```C++
parser.add_option(
  "--load-plugin FILE", [](string name) {
       dlopen(name.c_str(), ...);
     }, "plugin file");
```

In the plugin, if we take care that `Parser` is external linked, we
can call `add_option` directly in the initial
function. `options_parser::parser()` returns a static `Parser`, which
can be used across files, libraries.

To support sub commands a little easier, we can add `Parser` to another one. A
property of `Parser` is that, all `Parser`s copied from the one, holding the
same internal state, changing one, will change all of them. You don't need to
care about storage, and don't need to work in pointer semantic.

You can referer to
[git.cc](https://github.com/zuoyan/options_parser/blob/master/git.cc)
as a full example for how to use sub `Parser`s.

```C++
options_parser::Parser app("test git like commands", "epilog information");
app.add_help("--help");

options_parser::Parser add("add - Add file contents to the index", "\n");
// add.add_option(...);...

options_parser::Parser commit("commit - Record changes to the repository",
                              "\n");
// commit.add_option(...);...

std::string sub_command; // the active sub command

// There may be conflictions between sub commands, we have to
// add_parser dynamically
app.add_option(
    value().peek().apply([&](std::string c) {
      // return MATCH_EXACT if c is one of sub commands, otherwise return 0
    }),
    [&](std::string sub) {
      sub_command = cmd;
      // sub parser win when conflicting with app
      app.add_parser(sub_command_parsers[sub], 1);
	  sub_command_parsers.clear(); // only one sub command
    },
    {"<sub command>", "sub command of git, .."});
```
