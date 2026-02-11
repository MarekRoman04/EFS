# EFS -- Efficient File Search

EFS is a systems-programming focused command-line search utility written
in C.

The goal of the project is not to replace existing tools like grep, but
to design and implement a search engine from scratch while exploring
core systems and performance-oriented concepts.

The project emphasizes a modular architecture where major components
such as search algorithms, argument parsing, and file streaming are
intentionally decoupled. This design allows individual subsystems to be
extended, tested, or reused as standalone libraries.

EFS is primarily a learning and portfolio project showcasing algorithmic
thinking, clean architecture, and performance-aware software design.

------------------------------------------------------------------------

## Build

EFS uses CMake (\>= 3.10) and requires a standard C compiler (GCC /
Clang). No compiler-specific extensions are required.

### Debug build

``` bash
mkdir -p build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Release build

``` bash
mkdir -p build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Optional: memory allocation/free logging (Debug only)

EFS can be built with allocation/free logging enabled. This is
controlled by the LOG_MEMORY CMake option and is only active for Debug
builds.

``` bash
mkdir -p build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DLOG_MEMORY=ON
cmake --build build
```

## Usage

``` bash
efs [OPTIONS] [PATTERN] [FILES...]
```

-   `PATTERN` -- pattern to search for
-   `FILES` -- one or more files or directories

Examples:

``` bash
efs "needle" file.txt
efs -n "needle" file.txt
efs -c "needle" file.txt
efs -r "needle" ./documents
efs -f needles.txt file.txt
```

------------------------------------------------------------------------

## Options

  -----------------------------------------------------------------------
  Flag                   Description
  ---------------------- ------------------------------------------------
  `-c`, `--count`        Print only a count of matching lines per file

  `-f`, `--file`         Search patterns from file (pattern = line in
                         file)

  `-i`, `--ignore-case`  Perform case-insensitive matching

  `-n`, `--line-number`  Show line number for each match

  `-l`, `--list`         List only names of matching files

  `-q`, `--quiet`        Suppress normal output

  `-r`, `--recursive`    Recursively search directories

  `-v`, `--invert-match` Select non-matching lines (ignored with `-q` or
                         `-l`)

  `-w`, `--word`         Match only whole words

  `--output=FILE`        Write output to FILE instead of standard output

  `--buffer-size=N`      Set internal buffer size for disk reads
                         (default: 16KB)

  `--help`               Display help and exit
  -----------------------------------------------------------------------

------------------------------------------------------------------------

## Testing

EFS includes a small collection of utilities and scripts used for
validating search correctness and behavior under different scenarios.

The project provides:

-   **test_generator.py** -- generates random text data and patterns for
    testing
-   **pattern_subtest.sh** -- validates regular pattern searching
-   **file_subtest.sh** -- tests pattern-from-file search mode
-   **traversal_subtest.sh** -- verifies recursive directory traversal
    behavior

These tools are primarily intended for development and verification,
allowing results to be compared against standard utilities such as grep.

Typical usage:

``` bash
./pattern_subtest.sh
./file_subtest.sh
./traversal_subtest.sh 3 4 5  # (depth, dirs per level, files per dir)
```

## Architecture Overview

EFS is built from modular components that were modified for better
optimization within the system:

-   **Argument Parser** -- CLI flag handling and validation
-   **Search Engine** -- Pattern evaluation and file processing
-   **File Stream Layer** -- Buffered disk I/O and directory traversal
-   **Algorithms** -- Boyer--Moore--Horspool, Rabin--Karp
-   **Logging / Diagnostics** -- Debug and memory logging

High-level flow:

CLI → Parser → Search Engine → File Stream → Algorithm

## Search Algorithms

EFS currently includes:

-   **Boyer--Moore--Horspool** -- Optimized for short patterns and
    typical text search
-   **Boyer--Moore** -- More efficient for longer patterns and reduced
    comparisons
-   **Rabin--Karp** -- Hash-based approach enabling efficient
    multi-pattern search

Algorithm selection depends on search mode. Single-pattern searches use
Boyer--Moore variants, while pattern-from-file search relies on the
Rabin--Karp algorithm.

The modular design allows additional algorithms to be integrated without
modifying the CLI or file streaming layers.

## Future Plans

-   Rabin--Karp correctness and stability improvements
-   CI/CD pipeline integration
-   Parallel search execution for recursive traversal
-   Completion of Boyer--Moore implementation
-   File exclusion rules similar to `.gitignore`
-   Extraction of EFS modules into standalone libraries
