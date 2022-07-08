/**
 * src/include_graph_graphviz_printer.cc
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

#include "include_graph_graphviz_printer.h"

namespace clang_include_graph {

namespace detail {

label_writer::label_writer(
    const include_graph_t::graph_t &graph, const path_printer_t &pp)
    : graph_{graph}
    , path_printer_{pp}
{
}
template <typename Vertex>
void label_writer::operator()(std::ostream &out, const Vertex &v) const
{
    out << "[label=\"" << path_printer_.print(graph_.graph()[v].file) << "\"]";
}

} // namespace detail

void include_graph_graphviz_printer_t::operator()(std::ostream &os) const
{
    detail::label_writer writer{include_graph().graph(), path_printer()};

    boost::write_graphviz(os, include_graph().graph(), writer);
}

} // namespace clang_include_graph
