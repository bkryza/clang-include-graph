/**
 * src/include_graph_topological_sort_printer.h
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

#ifndef CLANG_INCLUDE_GRAPH_INCLUDE_GRAPH_TOPOLOGICAL_SORT_PRINTER_H
#define CLANG_INCLUDE_GRAPH_INCLUDE_GRAPH_TOPOLOGICAL_SORT_PRINTER_H

#include "include_graph_printer.h"

#include <iostream>

namespace clang_include_graph {

namespace detail {
class include_graph_topological_sort_visitor_t
    : public boost::default_dfs_visitor {
public:
    include_graph_topological_sort_visitor_t(
        std::vector<unsigned int> &vertices)
        : vertices_{vertices}
    {
    }

    template <typename V, typename G>
    void discover_vertex(V v, const G & /*g*/) const
    {
        vertices_.push_back(v);
    }

    std::vector<unsigned int> &vertices() const { return vertices_; }

private:
    std::vector<unsigned int> &vertices_;
};
}

class include_graph_topological_sort_printer_t
    : public include_graph_printer_t {
public:
    using include_graph_printer_t::include_graph_printer_t;

    void print(const include_graph_t &graph) const override
    {
        std::vector<unsigned int> include_order;
        include_order.reserve(graph.vertices.size());
        detail::include_graph_topological_sort_visitor_t visitor{include_order};

        boost::depth_first_search(graph.graph, boost::visitor(visitor));

        std::for_each(include_order.rbegin(), include_order.rend(),
            [&](const unsigned int id) {
                std::cout << path_printer().print(graph.vertices_names.at(id))
                          << std::endl;
            });
    }
};

} // namespace clang_include_graph

#endif // CLANG_INCLUDE_GRAPH_INCLUDE_GRAPH_TOPOLOGICAL_SORT_PRINTER_H