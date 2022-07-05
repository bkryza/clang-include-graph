/**
 * src/include_graph_cycles_printer.h
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

#ifndef CLANG_INCLUDE_GRAPH_INCLUDE_GRAPH_CYCLES_PRINTER_H
#define CLANG_INCLUDE_GRAPH_INCLUDE_GRAPH_CYCLES_PRINTER_H

#include "include_graph_printer.h"

#include <boost/graph/tiernan_all_cycles.hpp>

#include <iostream>

namespace boost {
void renumber_vertex_indices(
    clang_include_graph::include_graph_t::graph_adjlist_t const &)
{
}
} // namespace boost

namespace clang_include_graph {

namespace detail {
struct cycle_printer_t {
    cycle_printer_t(const include_graph_t::graph_t &graph,
        const path_printer_t &pp, std::ostream &stream)
        : graph_{graph}
        , path_printer_{pp}
        , os{stream}
    {
    }

    template <typename Path, typename Graph>
    void cycle(const Path &p, const Graph & /*g*/)
    {
        os << "[\n";
        for (auto it = p.begin(); it != p.end(); ++it) {
            os << "  " << path_printer_.print(graph_.graph()[*it].file) << "\n";
        }
        os << "]\n";
    }

private:
    const include_graph_t::graph_t &graph_;
    const path_printer_t &path_printer_;
    std::ostream &os;
};
}

class include_graph_cycles_printer_t : public include_graph_printer_t {
public:
    using include_graph_printer_t::include_graph_printer_t;

    void operator()(std::ostream &os) const override
    {
        detail::cycle_printer_t cycle_printer_visitor{
            include_graph().graph(), path_printer(), os};

        boost::tiernan_all_cycles(
            include_graph().graph().graph(), cycle_printer_visitor);
    }
};

} // namespace clang_include_graph

#endif // CLANG_INCLUDE_GRAPH_INCLUDE_GRAPH_CYCLES_PRINTER_H