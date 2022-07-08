/**
 * src/include_graph_tree_printer.cc
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

#include "include_graph_tree_printer.h"

namespace clang_include_graph {

include_graph_tree_printer_t::include_graph_tree_printer_t(
    const include_graph_t &graph, const path_printer_t &pp)
    : include_graph_printer_t{graph, pp}
{
}

void include_graph_tree_printer_t::operator()(std::ostream &os) const
{
    assert(include_graph().dag().has_value());

    auto begin = boost::vertices(include_graph().dag().value().graph()).first;
    auto end = boost::vertices(include_graph().dag().value().graph()).second;

    std::for_each(
        begin, end, [&](const include_graph_t::graph_t::vertex_descriptor &v) {
            const auto &vertex = include_graph().graph().graph()[v];
            if (vertex.is_translation_unit) {
                os << path_printer().print(vertex.file) << '\n';
                print_tu_subtree(os, v, 0, include_graph(), {});
            }
        });
}

void include_graph_tree_printer_t::print_tu_subtree(std::ostream &os,
    const include_graph_t::graph_t::vertex_descriptor tu_id,
    const unsigned int level, const include_graph_t &include_graph,
    std::vector<bool> continuation_line) const
{
    const auto kIndentWidth = 4U;

    boost::graph_traits<include_graph_t::graph_t>::adjacency_iterator it;
    boost::graph_traits<include_graph_t::graph_t>::adjacency_iterator it_end;

    boost::tie(it, it_end) =
        boost::adjacent_vertices(tu_id, include_graph.dag().value().graph());
    for (; it != it_end; ++it) {
        auto continuation_line_tmp = continuation_line;
        if (level > 0) {
            for (auto i = 0U; i < level; i++) {
                if (i % kIndentWidth == 0 &&
                    continuation_line[i / kIndentWidth]) {
                    os << "│";
                }
                else {
                    os << " ";
                }
            }
        }

        if (std::next(it) == it_end) {
            os << "└── ";
            continuation_line_tmp.push_back(false);
        }
        else {
            os << "├── ";
            continuation_line_tmp.push_back(true);
        }

        os << path_printer().print(include_graph.graph().graph()[*it].file)
           << '\n';
        print_tu_subtree(os, *it, level + kIndentWidth, include_graph,
            continuation_line_tmp);
    }
}

} // namespace clang_include_graph
