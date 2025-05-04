/**
 * src/include_graph_graphml_printer.cc
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

#include "include_graph_graphml_printer.h"
#include "include_graph.h"
#include "path_printer.h"

#include <boost/graph/graphml.hpp>

#include <ostream>

namespace clang_include_graph {

void include_graph_graphml_printer_t::operator()(std::ostream &os) const
{
    const auto &printer = path_printer();

    boost::dynamic_properties dp;

    if (include_graph().title()) {
        auto title_map =
            boost::make_constant_property<include_graph_t::graph_t *,
                std::string>(*include_graph().title());
        dp.property("title", title_map);
    }

    auto file_map = boost::make_transform_value_property_map(
        [&printer](
            const include_graph_t::vertex_t &v) { return printer.print(v); },
        get(boost::vertex_bundle, include_graph().graph()));

    dp.property("file", file_map);

    auto is_system_map = boost::make_transform_value_property_map(
        [](const bool is_system) { return is_system; },
        get(&include_graph_t::edge_t::is_system, include_graph().graph()));

    dp.property("is_system", is_system_map);

    boost::write_graphml(os, include_graph().graph(), dp, true);
}

} // namespace clang_include_graph
