/**
 * src/include_graph.cc
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

#include "include_graph.h"

namespace clang_include_graph {

void include_graph_t::add_edge(id_t from, id_t to)
{
    boost::add_edge(from, to, graph);
}

void include_graph_t::build()
{
    // Build mapping from vertices to paths
    auto it = edges.begin();
    for (; it != edges.end(); it++) {
        vertices.insert(it->first);
        vertices.insert(it->second);
    }

    unsigned vertex_id = 0;
    for (const auto &v : vertices) {
        vertices_ids[v] = vertex_id;
        vertices_names[vertex_id] = v;
        vertex_id++;
    }

    it = edges.begin();
    for (; it != edges.end(); it++) {
        add_edge(vertices_ids[it->second], vertices_ids[it->first]);
    }
}

void include_graph_t::build_dag()
{
    dag = include_graph_t::graph_t{};
    detail::dag_include_graph_visitor_t visitor{*this, *dag};

    boost::depth_first_search(graph, boost::visitor(visitor));
}

} // namespace clang_include_graph
