# clang-include-graph - simple tool for analyzing C++ include graphs

[![Build status](https://github.com/bkryza/clang-include-graph/actions/workflows/build.yml/badge.svg)](https://github.com/bkryza/clang-include-graph/actions)
[![Build status](https://github.com/bkryza/clang-include-graph/actions/workflows/macos.yml/badge.svg)](https://github.com/bkryza/clang-include-graph/actions)
[![Version](https://img.shields.io/badge/version-0.1.1-blue)](https://github.com/bkryza/clang-include-graph/releases)
[![Version](https://img.shields.io/badge/LLVM-12..20-orange)](https://github.com/bkryza/clang-include-graph/releases)

`clang-include-graph` provides several simple commands for analyzing and visualizing C++ project include graphs.

Main features include:
* Generating include graph in several formats:
  * Graphviz
  * PlantUML
  * GraphML
  * JSON
* Topologically ordered include list
* Printing include tree and reverse include tree
* Printing include graph cycles list
* Listing of all dependants of specified header file

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
apt install git make gcc g++ cmake clang-18 libclang-18-dev libclang-cpp18-dev libboost-graph1.83-dev libboost-filesystem1.83-dev libboost-test1.83-dev libboost-program-options1.83-dev
```

Then proceed with building the sources:

```bash
git clone https://github.com/bkryza/clang-include-graph
cd clang-include-graph
# Please note that top level Makefile is just a convenience wrapper for CMake
make release
release/clang-include-graph --help

# To build using a specific installed version of LLVM use:
LLVM_CONFIG_PATH=/usr/bin/llvm-config-18 make release

export PATH=$PATH:$PWD/release
```

## Usage
### Generating compile commands database
`clang-include-graph` requires an up-to-date
[compile_commands.json](https://clang.llvm.org/docs/JSONCompilationDatabase.html)
file, containing the list of commands used for compiling the source code
or alternatively a list of compilation flags in a file called
`compile_flags.txt`
(see [here](https://clang.llvm.org/docs/JSONCompilationDatabase.html#alternatives)).

See also [here](https://blog.bkryza.com/posts/compile-commands-json-gallery/)
for instructions on how to generate `compile_commands.json` using some of the
existing C++ build systems.

### Basic usage

`clang-include-graph` provides the following command line options:

```
❯ release/clang-include-graph -h
clang-include-graph options:
  -h [ --help ]                         Print help message and exit
  -V [ --version ]                      Print program version and exit
  -v [ --verbose ] arg                  Set log verbosity level
  --log-file arg                        Log to specified file instead of 
                                        console
  -J [ --jobs ] arg                     Number of threads used to parse 
                                        translation units
  -d [ --compilation-database-dir ] arg Path to compilation database directory 
                                        (default: $PWD)
  --add-compile-flag arg                Add a compile flag to the compilation 
                                        database
  --remove-compile-flag arg             Remove a compile flag from the 
                                        compilation database. Can contain a '*'
                                        to match multiple flags
  -u [ --translation-unit ] arg         Path or glob patterns to match 
                                        translation units for processing 
                                        (relative to $PWD). 
  -r [ --relative-to ] arg              Print paths relative to path (except 
                                        for system headers)
  -n [ --names-only ]                   Print only file names
  -l [ --relative-only ]                Include only files relative to 
                                        'relative-to' directory
  -o [ --output ] arg                   Write the output to a specified file 
                                        instead of stdout
  --json-numeric-ids                    Use numeric ids for nodes instead of 
                                        paths
  -e [ --dependants-of ] arg            Print all files that depend on a 
                                        specific header
  --translation-units-only              Print only translation units
  --exclude-system-headers              Exclude system headers from include 
                                        graph
  --system-headers-with-full-path       Print systems headers with their full 
                                        path
  --title arg                           Graph title that can be added 
  -s [ --topological-sort ]             Print output includes and translation 
                                        units in topologicalsort order
  -t [ --tree ]                         Print include graph in tree form
  -T [ --reverse-tree ]                 Print reverse include graph in tree 
                                        form
  -j [ --json ]                         Print include graph in Json Graph 
                                        format
  -c [ --cycles ]                       Print include graph cycles, if any
  -g [ --graphviz ]                     Print include graph in GraphViz format
  -G [ --graphml ]                      Print include graph in GraphML format
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
/usr/include/boost/config/compiler/clang_version.hpp
/usr/include/boost/config/compiler/clang.hpp
/usr/include/x86_64-linux-gnu/bits/wordsize.h
/usr/include/x86_64-linux-gnu/bits/timesize.h
/usr/include/features-time64.h
/usr/include/stdc-predef.h
/usr/include/x86_64-linux-gnu/bits/long-double.h
/usr/include/x86_64-linux-gnu/sys/cdefs.h
...
usr/include/boost/property_tree/detail/ptree_implementation.hpp
/usr/include/boost/property_tree/ptree.hpp
/usr/include/boost/property_tree/detail/file_parser_error.hpp
/usr/include/boost/property_tree/detail/xml_parser_error.hpp
/usr/include/boost/property_tree/detail/xml_parser_writer_settings.hpp
/usr/include/boost/property_tree/detail/xml_parser_utils.hpp
/usr/include/boost/graph/graphml.hpp
/home/bartek/devel/clang-include-graph/src/include_graph_graphml_printer.cc
/home/bartek/devel/clang-include-graph/src/main.cc
```

#### Include tree for a single translation unit including only project files with relative paths
```
❯ release/clang-include-graph -d release -u src/main.cc -r . -l --tree
src/main.cc
├── src/config.h
│   └── src/util.h
├── src/util.h
├── src/include_graph.h
│   └── src/config.h
│       └── src/util.h
├── src/include_graph_cycles_printer.h
│   └── src/include_graph_printer.h
│       ├── src/include_graph.h
│       │   └── src/config.h
│       │       └── src/util.h
│       └── src/path_printer.h
│           ├── src/config.h
│           │   └── src/util.h
│           └── src/include_graph.h
│               └── src/config.h
│                   └── src/util.h
├── src/include_graph_dependants_printer.h
│   └── src/include_graph_printer.h
│       ├── src/include_graph.h
│       │   └── src/config.h
│       │       └── src/util.h
│       └── src/path_printer.h
│           ├── src/config.h
│           │   └── src/util.h
│           └── src/include_graph.h
│               └── src/config.h
│                   └── src/util.h
├── src/include_graph_graphml_printer.h
│   └── src/include_graph_printer.h
│       ├── src/include_graph.h
│       │   └── src/config.h
│       │       └── src/util.h
│       └── src/path_printer.h
│           ├── src/config.h
│           │   └── src/util.h
│           └── src/include_graph.h
│               └── src/config.h
│                   └── src/util.h
├── src/include_graph_graphviz_printer.h
│   └── src/include_graph_printer.h
│       ├── src/include_graph.h
│       │   └── src/config.h
│       │       └── src/util.h
│       └── src/path_printer.h
│           ├── src/config.h
│           │   └── src/util.h
│           └── src/include_graph.h
│               └── src/config.h
│                   └── src/util.h
├── src/include_graph_json_printer.h
│   └── src/include_graph_printer.h
│       ├── src/include_graph.h
│       │   └── src/config.h
│       │       └── src/util.h
│       └── src/path_printer.h
│           ├── src/config.h
│           │   └── src/util.h
│           └── src/include_graph.h
│               └── src/config.h
│                   └── src/util.h
├── src/include_graph_parser.h
│   ├── src/config.h
│   │   └── src/util.h
│   └── src/include_graph.h
│       └── src/config.h
│           └── src/util.h
├── src/include_graph_plantuml_printer.h
│   └── src/include_graph_printer.h
│       ├── src/include_graph.h
│       │   └── src/config.h
│       │       └── src/util.h
│       └── src/path_printer.h
│           ├── src/config.h
│           │   └── src/util.h
│           └── src/include_graph.h
│               └── src/config.h
│                   └── src/util.h
├── src/include_graph_topological_sort_printer.h
│   └── src/include_graph_printer.h
│       ├── src/include_graph.h
│       │   └── src/config.h
│       │       └── src/util.h
│       └── src/path_printer.h
│           ├── src/config.h
│           │   └── src/util.h
│           └── src/include_graph.h
│               └── src/config.h
│                   └── src/util.h
└── src/include_graph_tree_printer.h
    └── src/include_graph_printer.h
        ├── src/include_graph.h
        │   └── src/config.h
        │       └── src/util.h
        └── src/path_printer.h
            ├── src/config.h
            │   └── src/util.h
            └── src/include_graph.h
                └── src/config.h
                    └── src/util.h
```

#### Reverse include tree for a single translation unit including only project files with relative paths
```
❯ release/clang-include-graph -d release -u src/main.cc -r . -l --reverse-tree
src/util.h
├── src/config.h
│   ├── src/main.cc
│   ├── src/include_graph.h
│   │   ├── src/main.cc
│   │   ├── src/include_graph_printer.h
│   │   │   ├── src/include_graph_cycles_printer.h
│   │   │   │   └── src/main.cc
│   │   │   ├── src/include_graph_dependants_printer.h
│   │   │   │   └── src/main.cc
│   │   │   ├── src/include_graph_graphml_printer.h
│   │   │   │   └── src/main.cc
│   │   │   ├── src/include_graph_graphviz_printer.h
│   │   │   │   └── src/main.cc
│   │   │   ├── src/include_graph_json_printer.h
│   │   │   │   └── src/main.cc
│   │   │   ├── src/include_graph_plantuml_printer.h
│   │   │   │   └── src/main.cc
│   │   │   ├── src/include_graph_topological_sort_printer.h
│   │   │   │   └── src/main.cc
│   │   │   └── src/include_graph_tree_printer.h
│   │   │       └── src/main.cc
│   │   ├── src/path_printer.h
│   │   │   └── src/include_graph_printer.h
│   │   │       ├── src/include_graph_cycles_printer.h
│   │   │       │   └── src/main.cc
│   │   │       ├── src/include_graph_dependants_printer.h
│   │   │       │   └── src/main.cc
│   │   │       ├── src/include_graph_graphml_printer.h
│   │   │       │   └── src/main.cc
│   │   │       ├── src/include_graph_graphviz_printer.h
│   │   │       │   └── src/main.cc
│   │   │       ├── src/include_graph_json_printer.h
│   │   │       │   └── src/main.cc
│   │   │       ├── src/include_graph_plantuml_printer.h
│   │   │       │   └── src/main.cc
│   │   │       ├── src/include_graph_topological_sort_printer.h
│   │   │       │   └── src/main.cc
│   │   │       └── src/include_graph_tree_printer.h
│   │   │           └── src/main.cc
│   │   └── src/include_graph_parser.h
│   │       └── src/main.cc
│   ├── src/path_printer.h
│   │   └── src/include_graph_printer.h
│   │       ├── src/include_graph_cycles_printer.h
│   │       │   └── src/main.cc
│   │       ├── src/include_graph_dependants_printer.h
│   │       │   └── src/main.cc
│   │       ├── src/include_graph_graphml_printer.h
│   │       │   └── src/main.cc
│   │       ├── src/include_graph_graphviz_printer.h
│   │       │   └── src/main.cc
│   │       ├── src/include_graph_json_printer.h
│   │       │   └── src/main.cc
│   │       ├── src/include_graph_plantuml_printer.h
│   │       │   └── src/main.cc
│   │       ├── src/include_graph_topological_sort_printer.h
│   │       │   └── src/main.cc
│   │       └── src/include_graph_tree_printer.h
│   │           └── src/main.cc
│   └── src/include_graph_parser.h
│       └── src/main.cc
└── src/main.cc
```

#### List of files dependent on a specific header
```
❯ release/clang-include-graph --dependants-of src/include_graph_printer.h --relative-to=. --relative-only -d release
src/include_graph_tree_printer.h
src/include_graph_tree_printer.cc
src/include_graph_cycles_printer.h
src/include_graph_cycles_printer.cc
tests/test_tree_printer.cc
src/include_graph_topological_sort_printer.h
src/include_graph_topological_sort_printer.cc
tests/test_topological_sort_printer.cc
src/include_graph_dependants_printer.h
tests/test_dependants_printer.cc
src/include_graph_json_printer.h
tests/test_json_printer.cc
src/include_graph_printer.cc
src/include_graph_dependants_printer.cc
src/include_graph_json_printer.cc
tests/test_util.cc
tests/test_cycles_printer.cc
src/include_graph_graphviz_printer.h
tests/test_graphviz_printer.cc
src/include_graph_plantuml_printer.h
src/include_graph_plantuml_printer.cc
src/include_graph_graphml_printer.h
tests/test_graphml_printer.cc
tests/test_plantuml_printer.cc
src/include_graph_graphml_printer.cc
src/main.cc
src/include_graph_graphviz_printer.cc
```

It is also possible to only list translation units with this generator:
```
❯ release/clang-include-graph --dependants-of src/include_graph_printer.h --translation-units-only --relative-to=. --relative-only -d release
src/include_graph_cycles_printer.cc
tests/test_cycles_printer.cc
src/include_graph_json_printer.cc
tests/test_topological_sort_printer.cc
tests/test_graphml_printer.cc
src/include_graph_dependants_printer.cc
src/include_graph_printer.cc
tests/test_dependants_printer.cc
src/include_graph_plantuml_printer.cc
src/include_graph_tree_printer.cc
tests/test_plantuml_printer.cc
src/include_graph_graphml_printer.cc
tests/test_tree_printer.cc
tests/test_util.cc
src/include_graph_topological_sort_printer.cc
tests/test_graphviz_printer.cc
tests/test_json_printer.cc
src/include_graph_graphviz_printer.cc
src/main.cc
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
F_0 <--  F_1
F_2 <--  F_0
F_3 <--  F_2
F_4 <--  F_3
F_5 <--  F_4
F_6 <--  F_2
F_4 <--  F_6
F_3 <--  F_6
@enduml
```

#### JSON representation of the include graph

```bash
❯ release/clang-include-graph --compilation-database-dir release -r . -u src/include_graph_tree_printer.cc -j -l | jq .
```
```json
{
  "directed": true,
  "type": "include_graph",
  "metadata": {
    "cli_arguments": "--compilation-database-dir release -r . -u src/include_graph_tree_printer.cc -j -l"
  },
  "nodes": {
    "src/include_graph_tree_printer.h": {
      "label": "src/include_graph_tree_printer.h",
      "metadata": {
        "is_system_header": false,
        "is_translation_unit": false
      }
    },
    "src/include_graph_tree_printer.cc": {
      "label": "src/include_graph_tree_printer.cc",
      "metadata": {
        "is_system_header": false,
        "is_translation_unit": true
      }
    },
    "src/include_graph_printer.h": {
      "label": "src/include_graph_printer.h",
      "metadata": {
        "is_system_header": false,
        "is_translation_unit": false
      }
    },
    "src/include_graph.h": {
      "label": "src/include_graph.h",
      "metadata": {
        "is_system_header": false,
        "is_translation_unit": false
      }
    },
    "src/config.h": {
      "label": "src/config.h",
      "metadata": {
        "is_system_header": false,
        "is_translation_unit": false
      }
    },
    "src/util.h": {
      "label": "src/util.h",
      "metadata": {
        "is_system_header": false,
        "is_translation_unit": false
      }
    },
    "src/path_printer.h": {
      "label": "src/path_printer.h",
      "metadata": {
        "is_system_header": false,
        "is_translation_unit": false
      }
    }
  },
  "edges": [
    {
      "target": "src/include_graph_tree_printer.h",
      "source": "src/include_graph_tree_printer.cc",
      "is_system": false
    },
    {
      "target": "src/include_graph_printer.h",
      "source": "src/include_graph_tree_printer.h",
      "is_system": false
    },
    {
      "target": "src/include_graph.h",
      "source": "src/include_graph_printer.h",
      "is_system": false
    },
    {
      "target": "src/config.h",
      "source": "src/include_graph.h",
      "is_system": false
    },
    {
      "target": "src/util.h",
      "source": "src/config.h",
      "is_system": false
    },
    {
      "target": "src/path_printer.h",
      "source": "src/include_graph_printer.h",
      "is_system": false
    },
    {
      "target": "src/config.h",
      "source": "src/path_printer.h",
      "is_system": false
    },
    {
      "target": "src/include_graph.h",
      "source": "src/path_printer.h",
      "is_system": false
    }
  ]
}
```

#### GraphML representation of the include graph

```bash
❯ release/clang-include-graph --compilation-database-dir release -r . -u src/include_graph_tree_printer.cc -G -l
```

```xml
<?xml version="1.0" encoding="UTF-8"?>
<graphml xmlns="http://graphml.graphdrawing.org/xmlns" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://graphml.graphdrawing.org/xmlns http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd">
  <key id="key0" for="node" attr.name="file" attr.type="string" />
  <key id="key1" for="edge" attr.name="is_system" attr.type="boolean" />
  <graph id="G" edgedefault="directed" parse.nodeids="canonical" parse.edgeids="canonical" parse.order="nodesfirst">
    <node id="n0">
      <data key="key0">src/include_graph_tree_printer.h</data>
    </node>
    <node id="n1">
      <data key="key0">src/include_graph_tree_printer.cc</data>
    </node>
    <node id="n2">
      <data key="key0">src/include_graph_printer.h</data>
    </node>
    <node id="n3">
      <data key="key0">src/include_graph.h</data>
    </node>
    <node id="n4">
      <data key="key0">src/config.h</data>
    </node>
    <node id="n5">
      <data key="key0">src/util.h</data>
    </node>
    <node id="n6">
      <data key="key0">src/path_printer.h</data>
    </node>
    <edge id="e0" source="n1" target="n0">
      <data key="key1">0</data>
    </edge>
    <edge id="e1" source="n0" target="n2">
      <data key="key1">0</data>
    </edge>
    <edge id="e2" source="n2" target="n3">
      <data key="key1">0</data>
    </edge>
    <edge id="e3" source="n3" target="n4">
      <data key="key1">0</data>
    </edge>
    <edge id="e4" source="n4" target="n5">
      <data key="key1">0</data>
    </edge>
    <edge id="e5" source="n2" target="n6">
      <data key="key1">0</data>
    </edge>
    <edge id="e6" source="n6" target="n4">
      <data key="key1">0</data>
    </edge>
    <edge id="e7" source="n6" target="n3">
      <data key="key1">0</data>
    </edge>
  </graph>
</graphml>
```

#### Count all files that need to be parsed when processing a translation unit
```bash
❯ release/clang-include-graph --compilation-database-dir release --translation-unit src/util.cc | wc -l
2223
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