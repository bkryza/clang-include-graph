/**
 * src/include_graph_topological_sort_printer.cc
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

#include "include_graph_topological_sort_printer.h"

#include <iostream>

namespace clang_include_graph {

void include_graph_topological_sort_printer_t::operator()(
    std::ostream &os) const
{
    assert(include_graph().dag().has_value());

    std::vector<include_graph_t::graph_t::vertex_descriptor> include_order;
    boost::topological_sort(
        include_graph().dag().value(), std::back_inserter(include_order));

    for (const auto id : include_order) {
        os << path_printer().print(include_graph().graph().graph()[id].file)
           << std::endl;
    }
}

} // namespace clang_include_graph
