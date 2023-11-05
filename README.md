# kash
A Unix shell built by me

This project was built for fun, mostly over the course of one weekend.

Still need to implement wildcards, but shell operators are already partially implemented using a custom parser that builds an abstract syntax tree (AST code is in the AST.cpp and AST.hpp files). The parsing code is in my parse_commands.cpp file. 

There is a previous version of this that I built without using a complex parser and exists entirely within one file that has been migrated to the simple-kash/ folder

## Instructions for building (on mac, other systems not tested)

Make sure that readline is installed using homebrew (macOS has a version built-int, but it's too old to work)
```
brew install --formulae readline
```

To determine the location it's installed at, run this:
```
brew --prefix readline
```

Use that path to compile the shell by modifying the CMake compilation step, like this:
```
cd build
cmake .. -DREADLINE_INCLUDE_DIR=<path to readline>/include -DREADLINE_LIBRARY_DIR=<path to readline>/lib
make
```

If your readline library is installed at `/usr/local/opt/readline` (like mine), you can just build with the default cmake:
```
cd build
cmake ..
make
```
