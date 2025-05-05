/**
 * tests/test_json_printer.cc
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

#define BOOST_TEST_MODULE Unit test of include_graph_json_printer_t

#include <boost/json.hpp>
#include <boost/test/unit_test.hpp>

#include "../src/include_graph_json_printer.h"

#include <map>
#include <sstream>
#include <string>

using namespace clang_include_graph;

BOOST_AUTO_TEST_CASE(test_json_printer)
{
    include_graph_t graph;
    graph.add_edge("include1.h", "main.cc", true);
    graph.add_edge("include2.h", "main.cc", true);
    graph.add_edge("include3.h", "util.cc", true);
    graph.add_edge("include4.h", "util.cc", true);
    graph.add_edge("string", "main.cc", true, true);
    graph.add_edge("include5.h", "include4.h");

    graph.build_dag();

    path_printer_t pp;

    include_graph_json_printer_t p{graph, pp};

    std::stringstream ss;
    ss << p;

    BOOST_CHECK_NO_THROW({
        boost::json::value root = boost::json::parse(ss.str());

        BOOST_TEST(root.is_object());
        auto &obj = root.as_object();

        BOOST_CHECK(obj.contains("nodes"));
        BOOST_TEST(obj["nodes"].is_object());

        auto &nodes = obj["nodes"].as_object();
        BOOST_CHECK(nodes.count("include1.h"));
        BOOST_CHECK(nodes.count("include2.h"));
        BOOST_CHECK(nodes.count("include3.h"));
        BOOST_CHECK(nodes.count("include4.h"));
        BOOST_CHECK(nodes.count("util.cc"));
        BOOST_CHECK(nodes.count("main.cc"));

        BOOST_CHECK(obj.contains("edges"));
        BOOST_TEST(obj["edges"].is_array());

        auto &edges = obj["edges"].as_array();

        BOOST_CHECK(edges.size() == 6);

        std::string expected =
            R"({"directed":true,"type":"include_graph","metadata":{"cli_arguments":""},"nodes":{"include1.h":{"label":"include1.h","metadata":{"is_system_header":false,"is_translation_unit":false}},"main.cc":{"label":"main.cc","metadata":{"is_system_header":false,"is_translation_unit":true}},"include2.h":{"label":"include2.h","metadata":{"is_system_header":false,"is_translation_unit":false}},"include3.h":{"label":"include3.h","metadata":{"is_system_header":false,"is_translation_unit":false}},"util.cc":{"label":"util.cc","metadata":{"is_system_header":false,"is_translation_unit":true}},"include4.h":{"label":"include4.h","metadata":{"is_system_header":false,"is_translation_unit":false}},"string":{"label":"string","metadata":{"is_system_header":true,"is_translation_unit":false}},"include5.h":{"label":"include5.h","metadata":{"is_system_header":false,"is_translation_unit":false}}},"edges":[{"target":"include1.h","source":"main.cc","is_system":false},{"target":"include2.h","source":"main.cc","is_system":false},{"target":"include3.h","source":"util.cc","is_system":false},{"target":"include4.h","source":"util.cc","is_system":false},{"target":"string","source":"main.cc","is_system":true},{"target":"include5.h","source":"include4.h","is_system":false}]})";

        BOOST_TEST(ss.str() == expected);
    });
}

BOOST_AUTO_TEST_CASE(test_json_printer_numeric_ids)
{
    include_graph_t graph;
    config_t config;
    config.json_printer_opts().numeric_ids = true;

    graph.init(config);
    graph.add_edge("include1.h", "main.cc", true);
    graph.add_edge("include2.h", "main.cc", true);

    graph.build_dag();

    path_printer_t pp;

    include_graph_json_printer_t p{graph, pp};

    std::stringstream ss;
    ss << p;

    boost::json::value root = boost::json::parse(ss.str());

    BOOST_TEST(root.is_object());
    auto &obj = root.as_object();

    BOOST_CHECK(obj.contains("nodes"));
    BOOST_TEST(obj["nodes"].is_object());

    auto &nodes = obj["nodes"].as_object();

    std::map<std::string, std::string> node_ids;
    for (const auto &node_it : nodes) {
        node_ids.emplace(
            node_it.value().as_object().at("label").as_string().c_str(),
            node_it.key());
    }

    BOOST_CHECK(nodes.count(node_ids.at("include1.h")));
    BOOST_CHECK(nodes.count(node_ids.at("include2.h")));
    BOOST_CHECK(nodes.count(node_ids.at("main.cc")));

    BOOST_CHECK(obj.contains("edges"));
    BOOST_TEST(obj["edges"].is_array());

    auto &edges = obj["edges"].as_array();

    BOOST_CHECK(edges.size() == 2);
}
