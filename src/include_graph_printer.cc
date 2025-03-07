/**
 * src/include_graph_printer.cc
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

#include "include_graph_printer.h"

#include <ostream>

namespace clang_include_graph {

include_graph_printer_t::include_graph_printer_t(
    const include_graph_t &graph, const path_printer_t &pp)
    : graph_{graph}
    , path_printer_{pp}
{
}

const path_printer_t &include_graph_printer_t::path_printer() const
{
    return path_printer_;
};

const include_graph_t &include_graph_printer_t::include_graph() const
{
    return graph_;
};

std::ostream &operator<<(std::ostream &os, include_graph_printer_t &a)
{
    a(os);
    return os;
}

} // namespace clang_include_graph
