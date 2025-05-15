/**
 * tests/test_tree_printer.cc
 *
 * Copyright (c) 2022-present Bartek Kryza <bkryza@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define BOOST_TEST_MODULE Unit test of include_graph_tree_printer_t

#include <boost/test/unit_test.hpp>

#include "../src/include_graph_tree_printer.h"
#include "test_utils.h"

#include <sstream>

using namespace clang_include_graph;

BOOST_AUTO_TEST_CASE(test_dag_prints_no_cycles)
{
    include_graph_t graph;
    graph.add_edge("include1.h", "main.cc", true);
    graph.add_edge("include2.h", "main.cc", true);
    graph.add_edge("include3.h", "util.cc", true);
    graph.add_edge("include4.h", "util.cc", true);
    graph.add_edge("include5.h", "include4.h");

    graph.build_dag();

    path_printer_t pp;

    include_graph_tree_printer_t p{graph, pp};

    std::stringstream ss;
    ss << p;

    std::vector<std::string> includes;
    read_lines(ss, includes);

    BOOST_TEST(includes.size() == 7);

#if !defined(_MSC_VER)
    BOOST_TEST(includes[0] == "main.cc");
    BOOST_TEST(includes[1] == "├── include1.h");
    BOOST_TEST(includes[2] == "└── include2.h");
    BOOST_TEST(includes[3] == "util.cc");
    BOOST_TEST(includes[4] == "├── include3.h");
    BOOST_TEST(includes[5] == "└── include4.h");
    BOOST_TEST(includes[6] == "    └── include5.h");
#else
    BOOST_TEST(includes[0] == "main.cc");
    BOOST_TEST(includes[1] == "+-- include1.h");
    BOOST_TEST(includes[2] == "\\-- include2.h");
    BOOST_TEST(includes[3] == "util.cc");
    BOOST_TEST(includes[4] == "+-- include3.h");
    BOOST_TEST(includes[5] == "\\-- include4.h");
    BOOST_TEST(includes[6] == "    \\-- include5.h");
#endif
}

BOOST_AUTO_TEST_CASE(test_dag_prints_with_cycles)
{
    include_graph_t graph;
    graph.add_edge("include1.h", "main.cc", true);
    graph.add_edge("include2.h", "main.cc", true);
    graph.add_edge("include3.h", "util.cc", true);
    graph.add_edge("include4.h", "util.cc", true);
    graph.add_edge("include2.h", "include1.h");
    graph.add_edge("include1.h", "include2.h");

    graph.build_dag();

    path_printer_t pp;

    include_graph_tree_printer_t p{graph, pp};

    std::stringstream ss;
    ss << p;

    std::vector<std::string> includes;
    read_lines(ss, includes);

    BOOST_TEST(includes.size() == 7);
#if !defined(_MSC_VER)
    BOOST_TEST(includes[0] == "main.cc");
    BOOST_TEST(includes[1] == "├── include1.h");
    BOOST_TEST(includes[2] == "│   └── include2.h");
    BOOST_TEST(includes[3] == "└── include2.h");
    BOOST_TEST(includes[4] == "util.cc");
    BOOST_TEST(includes[5] == "├── include3.h");
    BOOST_TEST(includes[6] == "└── include4.h");
#else
    BOOST_TEST(includes[0] == "main.cc");
    BOOST_TEST(includes[1] == "+-- include1.h");
    BOOST_TEST(includes[2] == "I   \\-- include2.h");
    BOOST_TEST(includes[3] == "\\-- include2.h");
    BOOST_TEST(includes[4] == "util.cc");
    BOOST_TEST(includes[5] == "+-- include3.h");
    BOOST_TEST(includes[6] == "\\-- include4.h");
#endif
}

BOOST_AUTO_TEST_CASE(test_dag_prints_reverse_tree)
{
    config_t config;
    config.printer(printer_t::reverse_tree);

    include_graph_t graph;
    graph.init(config);
    graph.add_edge("include1.h", "main.cc", true);
    graph.add_edge("include2.h", "main.cc", true);
    graph.add_edge("include3.h", "main.cc", true);
    graph.add_edge("include5.h", "main.cc", true);
    graph.add_edge("include4.h", "include5.h");

    graph.build_dag();

    path_printer_t pp;

    include_graph_tree_printer_t p{graph, pp};

    std::stringstream ss;
    ss << p;

    std::vector<std::string> includes;
    read_lines(ss, includes);

    BOOST_TEST(includes.size() == 9);
#if !defined(_MSC_VER)
    BOOST_TEST(includes[0] == "include1.h");
    BOOST_TEST(includes[1] == "└── main.cc");
    BOOST_TEST(includes[2] == "include2.h");
    BOOST_TEST(includes[3] == "└── main.cc");
    BOOST_TEST(includes[4] == "include3.h");
    BOOST_TEST(includes[5] == "└── main.cc");
    BOOST_TEST(includes[6] == "include4.h");
    BOOST_TEST(includes[7] == "└── include5.h");
    BOOST_TEST(includes[8] == "    └── main.cc");
#else
    BOOST_TEST(includes[0] == "include1.h");
    BOOST_TEST(includes[1] == "\\-- main.cc");
    BOOST_TEST(includes[2] == "include2.h");
    BOOST_TEST(includes[3] == "\\-- main.cc");
    BOOST_TEST(includes[4] == "include3.h");
    BOOST_TEST(includes[5] == "\\-- main.cc");
    BOOST_TEST(includes[6] == "include4.h");
    BOOST_TEST(includes[7] == "\\-- include5.h");
    BOOST_TEST(includes[8] == "    \\-- main.cc");
#endif
}