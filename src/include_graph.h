/**
 * src/include_graph.h
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

#ifndef CLANG_INCLUDE_GRAPH_INCLUDE_GRAPH_H
#define CLANG_INCLUDE_GRAPH_INCLUDE_GRAPH_H

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/optional.hpp>

#include <map>
#include <set>
#include <string>

namespace clang_include_graph {

class include_graph_t {
public:
    using id_t = unsigned int;
    using edges_t = std::set<std::pair<std::string, std::string>>;
    using vertices_t = std::set<std::string>;
    using vertices_ids_t = std::map<std::string, id_t>;
    using vertices_names_t = std::map<id_t, std::string>;

    using graph_t =
        boost::adjacency_list<boost::setS, boost::vecS, boost::directedS>;

    graph_t graph;
    edges_t edges;
    vertices_t vertices;
    vertices_ids_t vertices_ids;
    vertices_names_t vertices_names;
    boost::optional<std::string> relative_to;

    void add_edge(id_t from, id_t to) { boost::add_edge(from, to, graph); }

    void build()
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
};

} // namespace clang_include_graph

#endif // CLANG_INCLUDE_GRAPH_INCLUDE_GRAPH_H
