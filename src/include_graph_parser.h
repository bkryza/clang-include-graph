/**
 * src/include_graph_parser.h
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

#ifndef CLANG_INCLUDE_GRAPH_INCLUDE_GRAPH_PARSER_H
#define CLANG_INCLUDE_GRAPH_INCLUDE_GRAPH_PARSER_H

#include "config.h"
#include "include_graph.h"

#include <boost/asio/thread_pool.hpp>
#include <clang-c/Index.h>

#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace clang_include_graph {

class include_graph_parser_t {
public:
    explicit include_graph_parser_t(const config_t &config);

    ~include_graph_parser_t();
    include_graph_parser_t(const include_graph_parser_t &) = delete;
    include_graph_parser_t(include_graph_parser_t &&) = delete;
    include_graph_parser_t &operator=(const include_graph_parser_t &) = delete;
    include_graph_parser_t &operator=(include_graph_parser_t &&) = delete;

    void parse(include_graph_t &include_graph);

    const std::set<boost::filesystem::path> &translation_units() const;

private:
    CXIndex index_;
    const config_t &config_;
    std::set<boost::filesystem::path> translation_units_;
};

} // namespace clang_include_graph

#endif // CLANG_INCLUDE_GRAPH_INCLUDE_GRAPH_PARSER_H
