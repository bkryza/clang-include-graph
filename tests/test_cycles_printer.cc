/**
 * tests/test_cycles_printer.cc
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

#define BOOST_TEST_MODULE Unit test of include_graph_cycles_printer_t

#include <boost/test/unit_test.hpp>

#include "../src/include_graph_cycles_printer.h"
#include "test_utils.h"

#include <sstream>

using namespace clang_include_graph;

BOOST_AUTO_TEST_CASE(test_dag_prints_no_cycles)
{
    include_graph_t graph;
    graph.add_edge("include1.h", "main.cc", true);
    graph.add_edge("include2.h", "main.cc", true);
    graph.add_edge("include3.h", "include1.h");

    graph.build_dag();

    path_printer_t pp;

    include_graph_cycles_printer_t p{graph, pp};

    std::stringstream ss;
    ss << p;

    std::vector<std::string> includes;
    read_lines(ss, includes);

    BOOST_TEST(includes.size() == 0);
}

BOOST_AUTO_TEST_CASE(test_dag_prints_simple_cycles)
{
    include_graph_t graph;
    graph.add_edge("include1.h", "main.cc", true);
    graph.add_edge("include2.h", "main.cc", true);
    graph.add_edge("include2.h", "include1.h");
    graph.add_edge("include1.h", "include2.h");

    graph.build_dag();

    path_printer_t pp;

    include_graph_cycles_printer_t p{graph, pp};

    std::stringstream ss;
    ss << p;

    std::vector<std::string> includes;
    read_lines(ss, includes);

    BOOST_TEST(includes.size() == 4);
    BOOST_TEST(includes[0] == "[");
    BOOST_TEST(includes[1] == "  include1.h");
    BOOST_TEST(includes[2] == "  include2.h");
    BOOST_TEST(includes[3] == "]");
}

BOOST_AUTO_TEST_CASE(test_dag_prints_long_nested_cycles)
{
    include_graph_t graph;
    graph.add_edge("include1.h", "main.cc", true);
    graph.add_edge("include2.h", "main.cc", true);
    graph.add_edge("include2.h", "include1.h");
    graph.add_edge("include1.h", "include2.h");
    graph.add_edge("include3.h", "include1.h");
    graph.add_edge("include4.h", "include3.h");
    graph.add_edge("include5.h", "include4.h");
    graph.add_edge("include1.h", "include5.h");

    path_printer_t pp;

    include_graph_cycles_printer_t p{graph, pp};

    std::stringstream ss;
    ss << p;

    std::vector<std::string> includes;
    read_lines(ss, includes);

    BOOST_TEST(includes.size() == 10);
    BOOST_TEST(includes[0] == "[");
    BOOST_TEST(includes[1] == "  include1.h");
    BOOST_TEST(includes[2] == "  include2.h");
    BOOST_TEST(includes[3] == "]");
    BOOST_TEST(includes[4] == "[");
    BOOST_TEST(includes[5] == "  include1.h");
    BOOST_TEST(includes[6] == "  include3.h");
    BOOST_TEST(includes[7] == "  include4.h");
    BOOST_TEST(includes[8] == "  include5.h");
    BOOST_TEST(includes[9] == "]");
}