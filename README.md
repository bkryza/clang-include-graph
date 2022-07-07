# clang-include-graph - simple tool for analyzing C++ include graphs

[![Build status](https://github.com/bkryza/clang-include-graph/actions/workflows/build.yml/badge.svg)](https://github.com/bkryza/clang-include-graph/actions)

`clang-include-graph` provides several simple commands for analyzing and visualizing C++ project include graphs.

Main features include:
* Generates correct inclusion order of all includes of the project or a single translation unit
* Prints include graph in several formats into stdout:
  * Topologically ordered include list
  * Tree format
  * Graphviz
  * Include cycles
* Handles cyclic include graphs

## Installation
### Building from source
First make sure that you have the following dependencies installed:

```bash
# Ubuntu (clang version will vary depending on Ubuntu version)
apt install cmake libboost-graph-dev libboost-filesystem-dev clang-12 libclang-12-dev libclang-cpp12-dev
```

Then proceed with building the sources:

```bash
git clone https://github.com/bkryza/clang-include-graph
cd clang-include-graph
# Please note that top level Makefile is just a convenience wrapper for CMake
make release
release/clang-include-graph --help

# To build using a specific installed version of LLVM use:
LLVM_CONFIG_PATH=/usr/bin/llvm-config-13 make release

export PATH=$PATH:$PWD/release
```

## Usage
### Basic usage


`clang-include-graph` provides the following command line options:

```bash
❯ release/clang-include-graph -h
clang-include-graph options:
  -h [ --help ]                         Print help message and exit
  -V [ --version ]                      Print program version and exit
  -v [ --verbose ]                      Print verbose information during 
                                        processing
  -d [ --compilation-database-dir ] arg Path to compilation database directory 
                                        (default $PWD)
  -u [ --translation-unit ] arg         Process a single source file from 
                                        compilation database
  -r [ --relative-to ] arg              Generate paths relative to path (except
                                        for system headers)
  -n [ --names-only ]                   Print only file names
  -l [ --relative-only ]                Include only files relative to 
                                        'relative-to' directory
  -s [ --topological-sort ]             Print output includes and translation 
                                        units in topologicalsort order
  -t [ --tree ]                         Print output graph in tree form
  -g [ --graphviz ]                     Print output graph in GraphViz format
  -c [ --cycles ]                       Print include graph cycles, if any

```

### Example output
The examples below assume the following commands were executed before:
```bash
cd clang-include-graph
make release
```

#### Topologically sorted includes for project including only project files with full paths
```
❯ release/clang-include-graph --compilation-database-dir release
/usr/include/boost/config/user.hpp
/usr/include/boost/config/detail/select_compiler_config.hpp
/usr/include/boost/config/compiler/clang.hpp
/usr/include/x86_64-linux-gnu/bits/wordsize.h
/usr/include/x86_64-linux-gnu/bits/timesize.h
/usr/include/features-time64.h
/usr/include/stdc-predef.h
/usr/include/x86_64-linux-gnu/bits/long-double.h
/usr/include/x86_64-linux-gnu/sys/cdefs.h
/usr/include/x86_64-linux-gnu/gnu/stubs-64.h
/usr/include/x86_64-linux-gnu/gnu/stubs.h
...
/usr/include/boost/mpl/aux_/unwrap.hpp
/usr/include/boost/utility/value_init.hpp
/usr/include/boost/mpl/for_each.hpp
/usr/include/boost/test/tree/test_case_template.hpp
/usr/include/boost/test/tree/global_fixture.hpp
/usr/include/boost/test/unit_test_suite.hpp
/usr/include/boost/test/unit_test.hpp
/home/bartek/devel/clang-include-graph/tests/test_utils.h
/home/bartek/devel/clang-include-graph/tests/test_tree_printer.cc
/home/bartek/devel/clang-include-graph/tests/test_cycles_printer.cc
/home/bartek/devel/clang-include-graph/tests/test_topological_sort_printer.cc
```

#### Include tree for project including only project files with relative paths
```
❯ release/clang-include-graph -d release -u src/main.cc -r . -l --tree
src/main.cc
├── src/config.h
│   └── src/util.h
├── src/include_graph.h
│   └── src/config.h
│       └── src/util.h
├── src/include_graph_cycles_printer.h
│   └── src/include_graph_printer.h
│       ├── src/include_graph.h
│       │   └── src/config.h
│       │       └── src/util.h
│       └── src/path_printer.h
│           └── src/config.h
│               └── src/util.h
├── src/include_graph_graphviz_printer.h
│   └── src/include_graph_printer.h
│       ├── src/include_graph.h
│       │   └── src/config.h
│       │       └── src/util.h
│       └── src/path_printer.h
│           └── src/config.h
│               └── src/util.h
├── src/include_graph_parser.h
│   ├── src/config.h
│   │   └── src/util.h
│   └── src/include_graph.h
│       └── src/config.h
│           └── src/util.h
├── src/include_graph_topological_sort_printer.h
│   └── src/include_graph_printer.h
│       ├── src/include_graph.h
│       │   └── src/config.h
│       │       └── src/util.h
│       └── src/path_printer.h
│           └── src/config.h
│               └── src/util.h
└── src/include_graph_tree_printer.h
    └── src/include_graph_printer.h
        ├── src/include_graph.h
        │   └── src/config.h
        │       └── src/util.h
        └── src/path_printer.h
            └── src/config.h
                └── src/util.h
```

#### Generate GraphViz include graph of project files
```bash
❯ release/clang-include-graph --compilation-database-dir release --relative-to src --relative-only --graphviz > /tmp/include.dot
❯ dot -Tpng -o/tmp/include.png /tmp/include.dot
```

#### Print all include cycles in the include graph
```
❯ release/clang-include-graph --compilation-database-dir release -u src/util.cc --cycles
[
  /usr/include/boost/preprocessor/control/while.hpp
  /usr/include/boost/preprocessor/list/fold_left.hpp
]
[
  /usr/include/boost/preprocessor/control/while.hpp
  /usr/include/boost/preprocessor/list/fold_right.hpp
  /usr/include/boost/preprocessor/list/detail/fold_right.hpp
  /usr/include/boost/preprocessor/list/fold_left.hpp
]
[
  /usr/include/boost/preprocessor/control/while.hpp
  /usr/include/boost/preprocessor/list/fold_right.hpp
  /usr/include/boost/preprocessor/list/detail/fold_right.hpp
  /usr/include/boost/preprocessor/list/reverse.hpp
  /usr/include/boost/preprocessor/list/fold_left.hpp
]
[
  /usr/include/boost/preprocessor/control/while.hpp
  /usr/include/boost/preprocessor/list/fold_right.hpp
]
```

#### Count all files that need to be parsed when processing a translation unit
```bash
❯ release/clang-include-graph --compilation-database-dir release --translation-unit src/util.cc | wc -l
572
```

## LICENSE

    Copyright 2022-present Bartek Kryza <bkryza@gmail.com>

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.