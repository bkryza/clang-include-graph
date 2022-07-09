# clang-include-graph - simple tool for analyzing C++ include graphs

[![Build status](https://github.com/bkryza/clang-include-graph/actions/workflows/build.yml/badge.svg)](https://github.com/bkryza/clang-include-graph/actions)
[![Version](https://img.shields.io/badge/version-0.1.1-blue)](https://github.com/bkryza/clang-include-graph/releases)

`clang-include-graph` provides several simple commands for analyzing and visualizing C++ project include graphs.

Main features include:
* Generates correct (topological) inclusion order of all includes of the project or a single translation unit
* Prints include graph in several formats into stdout:
  * Topologically ordered include list
  * Tree format
  * Include graph cycles list
  * Graphviz
  * PlantUML
* Handles cyclic include graphs

## Installation
### Distribution packages
#### Ubuntu
```bash
sudo add-apt-repository ppa:bkryza/clang-include-graph
sudo apt update
sudo apt install clang-include-graph
```

### Building from source
First make sure that you have the following dependencies installed:

```bash
# Ubuntu (clang version will vary depending on Ubuntu version)
apt install git make gcc g++ cmake clang-12 libclang-12-dev libclang-cpp12-dev libboost-graph1.71-dev libboost-filesystem1.71-dev libboost-test1.71-dev libboost-program-options1.71-dev
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
### Generating compile commands database
`clang-include-graph` requires an up-to-date
[compile_commands.json](https://clang.llvm.org/docs/JSONCompilationDatabase.html)
file, containing the list of commands used for compiling the source code.
Nowadays, this file can be generated rather easily using multiple methods:
* For [CMake](https://cmake.org/) projects, simply invoke the `cmake` command
  as `cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ...`
* For Make projects checkout [compiledb](https://github.com/nickdiego/compiledb) or [Bear](https://github.com/rizsotto/Bear)
* For Boost-based projects try [commands_to_compilation_database](https://github.com/tee3/commands_to_compilation_database)
* For SCons, invoke `compilation_db` tool (requires SCons > 4.0.0)

### Basic usage

`clang-include-graph` provides the following command line options:

```
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
  -t [ --tree ]                         Print include graph in tree form
  -c [ --cycles ]                       Print include graph cycles, if any
  -g [ --graphviz ]                     Print include graph in GraphViz format
  -p [ --plantuml ]                     Print include graph in PlantUML format
```

### Example output
The examples below assume the following commands were executed before:
```bash
cd clang-include-graph
make release
```

#### Topologically sorted includes and translation units for project with full paths
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

#### Include tree for a single translation unit including only project files with relative paths
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

#### GraphViz include graph of project files
```bash
❯ release/clang-include-graph --compilation-database-dir release --relative-to src --relative-only --graphviz > /tmp/include.dot
❯ dot -Tpng -o/tmp/include.png /tmp/include.dot
```

#### List of include cycles within a single translation unit
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

####  PlantUML diagram of include graph for a single translation unit
```
❯ release/clang-include-graph --compilation-database-dir release -r . -u src/include_graph_tree_printer.cc -p -l
@startuml
file "src/include_graph_tree_printer.h" as F_0
file "src/include_graph_tree_printer.cc" as F_1
file "src/include_graph_printer.h" as F_2
file "src/include_graph.h" as F_3
file "src/config.h" as F_4
file "src/util.h" as F_5
file "src/path_printer.h" as F_6
F_0 -->  F_2
F_1 -->  F_0
F_2 -->  F_3
F_2 -->  F_6
F_3 -->  F_4
F_4 -->  F_5
F_6 -->  F_4
@enduml
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