/**
 * src/include_graph_topological_sort_printer.h
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

#ifndef CLANG_INCLUDE_GRAPH_INCLUDE_GRAPH_TOPOLOGICAL_SORT_PRINTER_H
#define CLANG_INCLUDE_GRAPH_INCLUDE_GRAPH_TOPOLOGICAL_SORT_PRINTER_H

#include "include_graph_printer.h"

#include <iostream>

namespace clang_include_graph {

class include_graph_topological_sort_printer_t
    : public include_graph_printer_t {
public:
    using include_graph_printer_t::include_graph_printer_t;

    ~include_graph_topological_sort_printer_t() override = default;

    include_graph_topological_sort_printer_t(
        const include_graph_topological_sort_printer_t &) = default;
    include_graph_topological_sort_printer_t(
        include_graph_topological_sort_printer_t &&) = default;
    include_graph_topological_sort_printer_t &operator=(
        const include_graph_topological_sort_printer_t &) = delete;
    include_graph_topological_sort_printer_t &operator=(
        include_graph_topological_sort_printer_t &&) = delete;

    void operator()(std::ostream &os) const override;
};

} // namespace clang_include_graph

#endif // CLANG_INCLUDE_GRAPH_INCLUDE_GRAPH_TOPOLOGICAL_SORT_PRINTER_H