/**
 * src/include_graph_plantuml_printer.cc
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

#include "include_graph_plantuml_printer.h"

namespace clang_include_graph {

include_graph_plantuml_printer_t::include_graph_plantuml_printer_t(
    const include_graph_t &graph, const path_printer_t &pp)
    : include_graph_printer_t{graph, pp}
{
}

void include_graph_plantuml_printer_t::operator()(std::ostream &os) const
{
    os << "@startuml" << '\n';

    if (include_graph().title()) {
        os << "title " << *include_graph().title() << '\n';
    }

    // First generate vertices with PlantUML aliases
    auto vertex_begin = boost::vertices(include_graph().graph()).first;
    const auto vertex_end = boost::vertices(include_graph().graph()).second;

    std::for_each(vertex_begin, vertex_end,
        [&](const include_graph_t::graph_t::vertex_descriptor &v) {
            const auto &vertex = include_graph().graph().graph()[v];
            os << "file \"" << path_printer().print(vertex) << "\" as F_" << v
               << '\n';
        });

    // Now generate include relationships based on edge list
    auto edges_begin = boost::edges(include_graph().graph()).first;
    auto edges_end = boost::edges(include_graph().graph()).second;

    std::for_each(edges_begin, edges_end,
        [&](const include_graph_t::graph_t::edge_descriptor &e) {
            const include_graph_t::graph_t::vertex_descriptor &from =
                boost::source(e, include_graph().graph());
            const include_graph_t::graph_t::vertex_descriptor &to =
                boost::target(e, include_graph().graph());

            os << "F_" << to;

            if (include_graph().graph()[e].is_system)
                os << " <.. ";
            else
                os << " <-- ";

            os << " F_" << from << '\n';
        });

    os << "@enduml" << '\n';
}

} // namespace clang_include_graph
