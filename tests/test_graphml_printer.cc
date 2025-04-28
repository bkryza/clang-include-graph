/**
 * tests/test_graphml_printer.cc
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

#define BOOST_TEST_MODULE Unit test of include_graph_graphml_printer_t

#include <boost/test/unit_test.hpp>

#include "../src/include_graph_graphml_printer.h"

#include <sstream>

using namespace clang_include_graph;

BOOST_AUTO_TEST_CASE(test_simple_graphviz)
{
    include_graph_t graph;
    graph.add_edge("include1.h", "main.cc", true);
    graph.add_edge("include2.h", "main.cc", true);
    graph.add_edge("include3.h", "include1.h");

    path_printer_t pp;

    include_graph_graphml_printer_t p{graph, pp};

    std::stringstream ss;
    ss << p;

    std::string expected = R"(<?xml version="1.0" encoding="UTF-8"?>
<graphml xmlns="http://graphml.graphdrawing.org/xmlns" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://graphml.graphdrawing.org/xmlns http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd">
  <key id="key0" for="node" attr.name="file" attr.type="string" />
  <graph id="G" edgedefault="directed" parse.nodeids="canonical" parse.edgeids="canonical" parse.order="nodesfirst">
    <node id="n0">
      <data key="key0">include1.h</data>
    </node>
    <node id="n1">
      <data key="key0">main.cc</data>
    </node>
    <node id="n2">
      <data key="key0">include2.h</data>
    </node>
    <node id="n3">
      <data key="key0">include3.h</data>
    </node>
    <edge id="e0" source="n1" target="n0">
    </edge>
    <edge id="e1" source="n1" target="n2">
    </edge>
    <edge id="e2" source="n0" target="n3">
    </edge>
  </graph>
</graphml>
)";

    BOOST_TEST(ss.str() == expected);
}
