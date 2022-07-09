/**
 * src/config.h
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

#ifndef CLANG_INCLUDE_GRAPH_CONFIG_H
#define CLANG_INCLUDE_GRAPH_CONFIG_H

#include "util.h"

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <string>

namespace clang_include_graph {

enum class printer_t {
    topological_sort,
    tree,
    cycles,
    graphviz,
    plantuml,
    unknown
};

class config_t {
public:
    void init(boost::program_options::variables_map &vm);

    bool verbose() const noexcept;

    const boost::optional<std::string> &
    compilation_database_directory() const noexcept;

    const boost::optional<std::string> &translation_unit() const noexcept;

    const boost::optional<std::string> &relative_to() const noexcept;

    bool filenames_only() const noexcept;

    bool relative_only() const noexcept;

    printer_t printer() const noexcept;

private:
    bool verbose_{false};
    boost::optional<std::string> compilation_database_directory_;
    boost::optional<std::string> translation_unit_;
    boost::optional<std::string> relative_to_;
    bool filenames_only_{false};
    bool relative_only_{false};
    printer_t printer_{printer_t::topological_sort};
};

} // namespace clang_include_graph

#endif // CLANG_INCLUDE_GRAPH_CONFIG_H
