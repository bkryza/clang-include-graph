/**
 * src/include_graph_printer.h
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

#ifndef CLANG_INCLUDE_GRAPH_INCLUDE_GRAPH_PRINTER_H
#define CLANG_INCLUDE_GRAPH_INCLUDE_GRAPH_PRINTER_H

#include "include_graph.h"
#include "path_printer.h"

namespace clang_include_graph {

class include_graph_printer_t {
public:
    include_graph_printer_t(
        const include_graph_t &graph, const path_printer_t &pp);

    virtual ~include_graph_printer_t() = default;

    include_graph_printer_t(const include_graph_printer_t &) = default;
    include_graph_printer_t(include_graph_printer_t &&) = default;
    include_graph_printer_t &operator=(
        const include_graph_printer_t &) = delete;
    include_graph_printer_t &operator=(include_graph_printer_t &&) = delete;

    virtual void operator()(std::ostream &) const = 0;

    const path_printer_t &path_printer() const;

    const include_graph_t &include_graph() const;

private:
    const include_graph_t &graph_;
    const path_printer_t &path_printer_;
};

std::ostream &operator<<(std::ostream &os, include_graph_printer_t &a);

} // namespace clang_include_graph

#endif // CLANG_INCLUDE_GRAPH_INCLUDE_GRAPH_PRINTER_H