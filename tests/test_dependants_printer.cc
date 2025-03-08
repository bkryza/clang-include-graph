/**
 * tests/test_dependants_printer.cc
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

#define BOOST_TEST_MODULE Unit test of include_graph_dependants_printer_t

#include <boost/test/unit_test.hpp>

#include "../src/include_graph_dependants_printer.h"
#include "test_utils.h"

#include <sstream>

using namespace clang_include_graph;

BOOST_AUTO_TEST_CASE(test_dependants_printer_prints_only_dependants_of_header)
{
    config_t config;
    config.dependants_of("include1.h");
    config.relative_to(".");
    config.relative_only(true);
    config.translation_units_only(false);
    config.printer(printer_t::dependants);

    include_graph_t graph;
    graph.init(config);

    graph.add_edge("include1.h", "main.cc", true);
    graph.add_edge("include2.h", "util.cc", true);
    graph.add_edge("include3.h", "other.cc", true);
    graph.add_edge("include1.h", "include2.h");
    graph.add_edge("include3.h", "include1.h");

    graph.build_dag();

    path_printer_t pp;

    include_graph_dependants_printer_t p{graph, pp};

    std::stringstream ss;
    ss << p;

    std::vector<std::string> includes;
    read_lines(ss, includes);

    BOOST_TEST(includes.size() == 3);
    BOOST_TEST(includes[0] == "main.cc");
    BOOST_TEST(includes[1] == "include2.h");
    BOOST_TEST(includes[2] == "util.cc");
}

BOOST_AUTO_TEST_CASE(
    test_dependants_printer_prints_only_tu_dependants_of_header)
{
    config_t config;
    config.dependants_of("include1.h");
    config.relative_to(".");
    config.relative_only(true);
    config.translation_units_only(true);
    config.printer(printer_t::dependants);

    include_graph_t graph;
    graph.init(config);

    graph.add_edge("include1.h", "main.cc", true);
    graph.add_edge("include2.h", "util.cc", true);
    graph.add_edge("include3.h", "other.cc", true);
    graph.add_edge("include1.h", "include2.h");
    graph.add_edge("include3.h", "include1.h");

    graph.build_dag();

    path_printer_t pp;

    include_graph_dependants_printer_t p{graph, pp};

    std::stringstream ss;
    ss << p;

    std::vector<std::string> includes;
    read_lines(ss, includes);

    BOOST_TEST(includes.size() == 2);
    BOOST_TEST(includes[0] == "main.cc");
    BOOST_TEST(includes[1] == "util.cc");
}