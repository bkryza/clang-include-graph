/**
 * src/include_graph.cc
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

#include "include_graph.h"

#include "config.h"

#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/labeled_graph.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/optional/optional.hpp>

#include <string>

namespace clang_include_graph {

void include_graph_t::add_edge(const std::string &to, const std::string &from,
    bool from_translation_unit, bool is_system)
{
    add_edge(to, from, to, from_translation_unit, is_system);
}

void include_graph_t::add_edge(const std::string &to, const std::string &from,
    const std::string &include_spelling, bool from_translation_unit,
    bool is_system)
{
    const std::lock_guard<std::mutex> guard{mutex_};

    if (graph_.vertex(to) == graph_t::null_vertex()) {
        LOG(trace) << "Adding target vertex " << to
                   << " [is_system_header=" << is_system
                   << ", include_spelling=" << include_spelling << "]";

        const auto &to_v = boost::add_vertex(to, graph_);
        graph_.graph()[to_v].file = to;
        graph_.graph()[to_v].is_system_header = is_system;
        graph_.graph()[to_v].include_spelling = include_spelling;
    }

    if (graph_.vertex(from) == graph_t::null_vertex()) {
        LOG(trace) << "Adding source vertex " << from
                   << " [is_system_header=" << is_system
                   << ", include_spelling=" << include_spelling << "]";

        const auto &from_v = boost::add_vertex(from, graph_);
        graph_.graph()[from_v].file = from;
        graph_.graph()[from_v].is_translation_unit = from_translation_unit;
    }

    std::pair<graph_t::edge_descriptor, bool> edge_pair;
    if (printer_ == printer_t::reverse_tree ||
        printer_ == printer_t::dependants) {
        edge_pair = boost::add_edge_by_label(to, from, graph_);
    }
    else {
        edge_pair = boost::add_edge_by_label(from, to, graph_);
    }

    if (edge_pair.second) {
        graph_.graph()[edge_pair.first].is_system = is_system;
    }
}

void include_graph_t::init(const config_t &config)
{
    relative_to_ = config.relative_to();
    relative_only_ = config.relative_only();
    dependants_of_ = config.dependants_of();
    translation_units_only_ = config.translation_units_only();
    printer_ = config.printer();
    cli_arguments_ = config.cli_arguments();
    numeric_ids_ = config.json_printer_opts().numeric_ids;
    title_ = config.title();
    exclude_system_headers_ = config.exclude_system_headers();
}

void include_graph_t::build_dag()
{
    const std::lock_guard<std::mutex> guard{mutex_};

    if (dag_) {
        return;
    }

    dag_ = include_graph_t::graph_t{};
    const detail::dag_include_graph_visitor_t visitor{*this};

    boost::depth_first_search(graph_, boost::visitor(visitor));
}

const boost::optional<include_graph_t::graph_t> &
include_graph_t::dag() const noexcept
{
    return dag_;
}

boost::optional<include_graph_t::graph_t> &include_graph_t::dag() noexcept
{
    return dag_;
}

const include_graph_t::graph_t &include_graph_t::graph() const noexcept
{
    return graph_;
}

bool include_graph_t::relative_only() const noexcept { return relative_only_; }

bool include_graph_t::translation_units_only() const noexcept
{
    return translation_units_only_;
}

bool include_graph_t::exclude_system_headers() const noexcept
{
    return exclude_system_headers_;
}

printer_t include_graph_t::printer() const noexcept { return printer_; }

const boost::optional<boost::filesystem::path> &
include_graph_t::relative_to() const noexcept
{
    return relative_to_;
}

const boost::optional<boost::filesystem::path> &
include_graph_t::dependants_of() const noexcept
{
    return dependants_of_;
}

const std::string &include_graph_t::cli_arguments() const noexcept
{
    return cli_arguments_;
}

bool include_graph_t::numeric_ids() const noexcept { return numeric_ids_; }

const boost::optional<std::string> &include_graph_t::title() const noexcept
{
    return title_;
}
} // namespace clang_include_graph
