/**
 * src/include_graph_dependants_printer.cc
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

#include "include_graph_dependants_printer.h"

#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/labeled_graph.hpp>

#include <algorithm>
#include <cassert>
#include <iterator>
#include <ostream>

namespace clang_include_graph {

include_graph_dependants_printer_t::include_graph_dependants_printer_t(
    const include_graph_t &graph, const path_printer_t &pp)
    : include_graph_printer_t{graph, pp}
{
}

void include_graph_dependants_printer_t::operator()(std::ostream &os) const
{
    assert(include_graph().dag());

    const auto &dag = include_graph().dag().value().graph();
    const auto &graph = include_graph().graph().graph();

    const auto &dependants_root = include_graph().dependants_of().value();

    auto vertices_range = boost::vertices(dag);
    auto it = std::find_if(vertices_range.first, vertices_range.second,
        [&](include_graph_t::graph_t::vertex_descriptor v) {
            return graph[v].file == dependants_root;
        });

    if (it == vertices_range.second) {
        std::cout << "Dependants root '" << dependants_root << "' not found.\n";
        return;
    }

    auto start = *it;

    std::set<include_graph_t::graph_t::vertex_descriptor> dependants;

    std::set<include_graph_t::graph_t::vertex_descriptor> current_out_set;
    current_out_set.emplace(start);

    // Perform breadth first search over all outgoing edges starting from
    // vertex representing root of dependants tree
    while (!current_out_set.empty()) {
        std::set<include_graph_t::graph_t::vertex_descriptor> next_out_set;

        for (const auto &itt : current_out_set) {
            for (const auto &it : graph.out_edge_list(itt)) {
                if (dependants.count(it.get_target()) == 0) {
                    next_out_set.emplace(it.get_target());
                }
            }
        }

        dependants.insert(current_out_set.begin(), current_out_set.end());

        std::swap(next_out_set, current_out_set);
    }

    for (const auto &v : dependants) {
        if (v == start) {
            continue;
        }

        if (!include_graph().translation_units_only() ||
            graph[v].is_translation_unit) {
            auto dependant_path = path_printer().print(graph[v].file);
            os << dependant_path << '\n';
        }
    }
}

} // namespace clang_include_graph
