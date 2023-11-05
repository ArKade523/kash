# Simple kash

This is my original version, built without wildcards or shell operators. The newer version uses an abstract syntax tree (AST) to parse shell operators and create nodes for all commands and operators to make sure that they are all handled correctly.

This one functions well, but is pretty barebones.

## Instructions for building (on mac, other systems not tested)

Make sure that readline is installed using homebrew (macOS has a version built-int, but it's too old to work)
```
brew install --formulae readline
```

To determine the location it's installed at, run this:
```
brew --prefix readline
```

Use that path to compile the shell, like this:
```
g++ -I<path to readline>/include -L<path to readline>/lib kash.cpp -o kash -std=c++11 -lreadline
```

If your readline library is installed at `/usr/local/opt/readline`, the command will be this:
```
g++ -I/usr/local/opt/readline/include -L/usr/local/opt/readline/lib kash.cpp -o kash -std=c++11 -lreadline
```
