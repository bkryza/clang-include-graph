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

void include_graph_t::add_edge(
    const std::string &to, const std::string &from, bool from_translation_unit)
{
    if (graph_.vertex(to) == graph_.null_vertex()) {
        const auto &to_v = boost::add_vertex(to, graph_);
        graph_.graph()[to_v].file = to;
    }

    if (graph_.vertex(from) == graph_.null_vertex()) {
        const auto &from_v = boost::add_vertex(from, graph_);
        graph_.graph()[from_v].file = from;
        graph_.graph()[from_v].is_translation_unit = from_translation_unit;
    }

    boost::add_edge_by_label(from, to, graph_);
}

void include_graph_t::init(const config_t &config)
{
    relative_to_ = config.relative_to;
    relative_only_ = config.relative_only;
}

void include_graph_t::build_dag()
{
    dag_ = include_graph_t::graph_t{};
    detail::dag_include_graph_visitor_t visitor{*this, *dag_};

    boost::depth_first_search(graph_, boost::visitor(visitor));
}

} // namespace clang_include_graph
