/**
 * src/include_graph_json_printer.cc
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

#include "include_graph_json_printer.h"

#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/labeled_graph.hpp>

#include <algorithm>
#include <boost/json.hpp>
#include <cassert>
#include <ostream>
#include <vector>

namespace clang_include_graph {

include_graph_json_printer_t::include_graph_json_printer_t(
    const include_graph_t &graph, const path_printer_t &pp)
    : include_graph_printer_t{graph, pp}
{
}

void include_graph_json_printer_t::operator()(std::ostream &os) const
{
    using namespace boost::json;

    assert(include_graph().dag());

    auto begin = boost::vertices(include_graph().dag().value().graph()).first;
    auto end = boost::vertices(include_graph().dag().value().graph()).second;

    object g;
    g["directed"] = true;
    g["type"] = "include_graph";

    if (include_graph().title())
        g["label"] = *include_graph().title();

    object meta;
    meta["cli_arguments"] = include_graph().cli_arguments();
    g["metadata"] = std::move(meta);

    object nodes;

    std::for_each(
        begin, end, [&](const include_graph_t::graph_t::vertex_descriptor &v) {
            const auto &graph = include_graph().graph().graph();
            const auto &vertex = graph[v];

            const auto file_path = path_printer().print(vertex.file);
            object node;
            node["label"] = file_path;
            node["metadata"] = object{};

            if (include_graph().numeric_ids())
                nodes[std::to_string(v)] = std::move(node);
            else
                nodes[file_path] = std::move(node);
        });

    g["nodes"] = std::move(nodes);

    array edges;

    auto edges_begin = boost::edges(include_graph().graph()).first;
    auto edges_end = boost::edges(include_graph().graph()).second;

    std::for_each(edges_begin, edges_end,
        [&](const include_graph_t::graph_t::edge_descriptor &e) {
            const auto &graph = include_graph().graph().graph();

            const include_graph_t::graph_t::vertex_descriptor &from =
                boost::source(e, graph);
            const include_graph_t::graph_t::vertex_descriptor &to =
                boost::target(e, graph);

            object edge;
            if (include_graph().numeric_ids()) {
                edge["target"] = std::to_string(to);
                edge["source"] = std::to_string(from);
            }
            else {
                const auto &from_vertex = graph[from];
                const auto from_file_path =
                    path_printer().print(from_vertex.file);

                const auto &to_vertex = graph[to];
                const auto to_file_path = path_printer().print(to_vertex.file);

                edge["target"] = to_file_path;
                edge["source"] = from_file_path;
            }
            edge["relation"] = "system";

            edges.emplace_back(std::move(edge));
        });

    g["edges"] = std::move(edges);

    os << g;
}

} // namespace clang_include_graph
