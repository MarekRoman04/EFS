# EFS -- Efficient File Search

EFS is command-line interface utility written in C. EFS searches given pattern in files. 

## Features

- Efficient file I/O for large files  
- Recursive search through directories  
- Pattern matching with **Rabin-Karp** and **Boyer-Moore** algorithms  
- Fast multiple-pattern search using **Rabin-Karp**  
- Case-insensitive and whole-word matching

## Future Plans

-   Rabin--Karp correctness and stability improvements
-   CI/CD pipeline integration
-   Parallel search execution for recursive traversal
-   Completion of Boyer--Moore implementation
-   File exclusion rules similar to `.gitignore`

------------------------------------------------------------------------

## Build

EFS uses CMake (\>= 3.10) and requires a standard C compiler (GCC /
Clang). No compiler-specific extensions are required.

### Release build

``` bash
mkdir -p build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Debug build

``` bash
mkdir -p build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
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
efs [OPTIONS] [PATTERN] [FILE...]
```

-   `PATTERN` -- pattern to search for
-   `FILES` -- one or more files or directories

Examples:

``` bash
efs "needle" file.txt
efs -c "needle" file.txt
efs -r "needle" ./documents
efs -f needles.txt file.txt
```

### Options

| Flag | Description |
|-----|-------------|
| `-c`, `--count` | Print only the number of matching lines per file |
| `-f`, `--file FILE` | Read search patterns from `FILE` (one pattern per line) |
| `-i`, `--ignore-case` | Perform case-insensitive matching |
| `-n`, `--line-number` | Show line number for each match |
| `-l`, `--list` | List only names of files containing matches |
| `-q`, `--quiet` | Suppress normal output |
| `-r`, `--recursive` | Recursively search directories |
| `-v`, `--invert-match` | Select non-matching lines (ignored with `-q` or `-l`) |
| `-w`, `--word` | Match whole words only |
| `--output=FILE` | Write output to `FILE` instead of standard output |
| `--buffer-size=N` | Set internal disk read buffer size (default: `16 KB`) |
| `--help` | Display help and exit |


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