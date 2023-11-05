# kash
A Unix shell built by me

This project was built for fun, mostly over the course of one weekend.

Still need to implement wildcards

## Instructions for building (on mac, other systems not tested)

Make sure that readline is installed using homebrew (macOS has a version built-int, but it's too old to work)
```bash
brew install --formulae readline
```

To determine the location it's installed at, run this:
```bash
brew --prefix readline
```

Use that path to compile the shell, like this:
```bash
g++ -I<path to readline>/include -L<path to readline>/lib kash.cpp -o kash -std=c++11 -lreadline
```

If your readline library is installed at `/usr/local/opt/readline`, the command will be this:
```bash
g++ -I/usr/local/opt/readline/include -L/usr/local/opt/readline/lib kash.cpp -o kash -std=c++11 -lreadline
```
