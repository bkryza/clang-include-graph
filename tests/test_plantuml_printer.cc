/**
 * tests/test_plantuml_printer.cc
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

#define BOOST_TEST_MODULE Unit test of include_graph_plantuml_printer_t

#include <boost/test/unit_test.hpp>

#include "../src/include_graph_plantuml_printer.h"

#include <sstream>

using namespace clang_include_graph;

BOOST_AUTO_TEST_CASE(test_simple_plantuml)
{
    include_graph_t graph;
    graph.add_edge("include1.h", "main.cc", true);
    graph.add_edge("include2.h", "main.cc", true);
    graph.add_edge("include3.h", "include1.h");

    path_printer_t pp;

    include_graph_plantuml_printer_t p{graph, pp};

    std::stringstream ss;
    ss << p;

    std::string expected = R"(@startuml
file "include1.h" as F_0
file "main.cc" as F_1
file "include2.h" as F_2
file "include3.h" as F_3
F_0 -->  F_3
F_1 -->  F_0
F_1 -->  F_2
@enduml
)";

    BOOST_TEST(ss.str() == expected);
}
