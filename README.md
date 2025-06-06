```
______         _    _
| ___ \       | |  | |
| |_/ /  __ _ | |_ | |__
|  __/  / _` || __|| '_ \
| |    | (_| || |_ | | | |
\_|     \__,_| \__||_| |_|  v25.6.5

```
A simple cross-platform utility for printing paths.

## Description

The `path` command provides a quick and easy interface for locating directories (or even files). A directory may be a symlink, and file can either be a "regular file" or symlink, a character or block device, a FIFO/pipe, or a socket.

### Installing

The program is written in plain vanilla C and implemented as a single source file (roughly ~400 LOC) so it can be compiled directly with your favorite compiler. Otherwise the provided makefile can be used if 'make' is available on your system (Windows and Linux-like platforms).

Clone the repository:

```bash
git clone https://github.com/SpyrjaGaldr/path && cd path
```

Build and install:

```bash
# compile with gcc
gcc -o path path.c

# compile with make
make all

# compile with clang
make all CC=clang

# install to /usr/bin (Linux) or C:\Program Files (Windows)
sudo make install

# compile and install somewhere else
make install INSTALL_DIR=/home/somewhere/else

# compile and install as the "where" command
sudo make install NAME=where

# uninstall the "where" command
sudo make uninstall NAME=where
```

### Usage

By default, `path` performs a case-insensitive, wildcard search for directories (or optionally files) matching one or more set of criteria. If none is specified, all candidates will be marked as matching. If the search directory is unspecified, the program will first look in the current folder and then in the system root directory.

* -d DIRECTORY

    Adds a single directory to the search path; multiple directories can be included by separating them with the ':' or ';' character (see example below)

* -f

    Search for files instead of directories

* -n MAX

    Return MAX results

* -c

    Perform a case-sensitive search

* -z

    Use fuzzy-search mode; a string such as "guiab" will match folder "confi[g][u]rat[i]on-[a]vaila[b]le"

* -p

    Prefix-search mode; target must start with a given sequence; the pattern "jav" will match "[jav]script-snippets", "[jav]a-files", etc

* -s

    Suffix-search mode; target must end with a given sequence; ".jpg" will match "winter-in-maine.jpg" (assuming `-f` flag has been specified)

* -q

    Put all matches with embedded spaces in quotes

* -x

    Print everything on a single line (like Linux `xargs`; matches with embedded spaces will be quoted)

* -w

    Report a warning if a given directory cannot be traversed (ie. due to insufficient permissions)

* -v

    Print out version information

Note: the -e, -z, -p, and -s flags are mutually exclusive; attempting to supply more than one will result in an error

### Examples

```bash
#  print all directories in file-system
path

#  print all files in the current folder recursively
path -fd.

# find all directories named "src" (or even "SRC"),
#  starting in the current folder, then from the system root directory
path src

# find all directories (starting from the current folder, then from the root folder)
#  named "Downloads" (case-sensitive)
path -c Downloads

# search for various types of image files
path -sf .jpg .jpeg .png

# list all game ROM's available, restricted
#  to the $PATH environment variable
path -fd $PATH *.rom

# list all files ending in ".so" or ".so.1", checking
#  first in the /lib32 folder, then in /lib64
# Warning: NEVER search for executable content
#  within insecure folders; both /lib32 and /lib64
#  in this example reside in "system" directories
path -fsd /lib32:/lib64 .so .so.1
```

### License: MIT
Copyright © 2025 SpyrjaGaldr
