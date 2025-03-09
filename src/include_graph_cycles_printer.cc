/**
 * src/include_graph_cycles_printer.cc
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

#include "include_graph_cycles_printer.h"
#include "include_graph.h"
#include "path_printer.h"

#include <boost/graph/tiernan_all_cycles.hpp>

#include <ostream>

namespace boost {
void renumber_vertex_indices(
    clang_include_graph::include_graph_t::graph_adjlist_t const & /*unused*/)
{
}
} // namespace boost

namespace clang_include_graph {
namespace detail {

cycle_printer_t::cycle_printer_t(const include_graph_t::graph_t &graph,
    const path_printer_t &pp, std::ostream &stream)
    : graph_{graph}
    , path_printer_{pp}
    , os{stream}
{
}

template <typename Path, typename Graph>
void cycle_printer_t::cycle(const Path &p, const Graph & /*g*/)
{
    os << "[\n";
    for (auto it = p.begin(); it != p.end(); ++it) {
        os << "  " << path_printer_.print(graph_.graph()[*it].file) << "\n";
    }
    os << "]\n";
}

} // namespace detail

void include_graph_cycles_printer_t::operator()(std::ostream &os) const
{
    const detail::cycle_printer_t cycle_printer_visitor{
        include_graph().graph(), path_printer(), os};

    boost::tiernan_all_cycles(
        include_graph().graph().graph(), cycle_printer_visitor);
}

} // namespace clang_include_graph
