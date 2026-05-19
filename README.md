# System Programming Lab 2

This repository contains Lab 2 exercises for a system programming course. The main program is a small Unix-like shell written in C, with helper code for command parsing and a simple pipe demonstration program.

## Files

- `myshell.c` - interactive shell implementation.
- `mypipe.c` - simple process and pipe example.
- `LineParser.c`, `LineParser.h` - command-line parsing utilities used by the shell.
- `makefile` - builds the project executables.
- `LAB2.html` - original lab instructions.
- `in.txt`, `out.txt` - small sample input/output files.

## Build

The makefile uses `cc -m32`, so a 32-bit C build environment is required.

```sh
make
```

This builds:

- `myshell`
- `mypipe`

## Run

```sh
./myshell
```

Run in debug mode:

```sh
./myshell -d
```

Run the pipe demo:

```sh
./mypipe "hello"
```

## Clean

```sh
make clean
```

Compiled binaries are intentionally not committed to the repository.
