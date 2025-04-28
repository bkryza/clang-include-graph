/**
 * src/config.cc
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

#include "config.h"
#include "util.h"

#include <boost/filesystem/path.hpp>
#include <boost/optional/optional.hpp>
#include <boost/program_options/variables_map.hpp>

#include <cstdlib>
#include <iostream>

namespace clang_include_graph {

void config_t::init(
    boost::program_options::variables_map &vm, const std::string &cli_arguments)
{
    cli_arguments_ = cli_arguments;

    if (vm.count("verbose") == 1) {
        verbosity_ = vm["verbose"].as<int>();
    }

    if (vm.count("log-file") == 1) {
        log_file_ = vm["log-file"].as<std::string>();
    }

    if (vm.count("jobs") == 1) {
        auto jobs_arg = vm["jobs"].as<unsigned>();
        if (jobs_arg != 0) {
            jobs_ = jobs_arg;
        }
    }

    if (vm.count("compilation-database-dir") == 1) {
        compilation_database_directory_ = util::to_absolute_path(
            vm["compilation-database-dir"].as<std::string>());
    }
    else {
        compilation_database_directory_ = util::to_absolute_path(".");
    }

    if (vm.count("output") == 1) {
        output_file_ = vm["output"].as<std::string>();
    }

    if (!compilation_database_directory_) {
        std::cerr << "ERROR: Cannot find compilation database - aborting..."
                  << '\n';
        exit(-1);
    }

    if (vm.count("relative-to") == 1) {
        relative_to_ =
            util::to_absolute_path(vm["relative-to"].as<std::string>());
    }

    if (vm.count("names-only") == 1) {
        filenames_only_ = true;
    }

    if (vm.count("relative-only") == 1) {
        relative_only_ = true;
    }

    if (relative_only_ && !relative_to_) {
        relative_to_ = util::to_absolute_path(".");
    }

    if (relative_to_ && filenames_only_) {
        std::cerr << "ERROR: --relative-to and --names-only cannot be enabled "
                     "at the same time"
                  << " - aborting..." << '\n';
        exit(-1);
    }

    if (vm.count("translation-unit") == 1) {
        translation_unit_ =
            vm["translation-unit"].as<std::vector<boost::filesystem::path>>();
    }

    if (vm.count("add-compile-flag") == 1) {
        add_compile_flag_ =
            vm["add-compile-flag"].as<std::vector<std::string>>();
    }

    if (vm.count("remove-compile-flag") == 1) {
        remove_compile_flag_ =
            vm["remove-compile-flag"].as<std::vector<std::string>>();
    }

    if (vm.count("tree") + vm.count("reverse-tree") + vm.count("cycles") +
            vm.count("graphviz") + vm.count("topological-sort") +
            vm.count("plantuml") >
        1) {
        std::cerr << "ERROR: Only one output method can be selected at a time"
                  << " - aborting..." << '\n';
        exit(-1);
    }

    if (vm.count("dependants-of") > 0) {
        printer_ = printer_t::dependants;
        dependants_of_ = resolve_path(vm["dependants-of"].as<std::string>());
    }

    if (vm.count("translation-units-only") == 1) {
        translation_units_only_ = true;
    }

    if (vm.count("title") == 1) {
        title_ = vm["title"].as<std::string>();
    }

    if (vm.count("json-numeric-ids") == 1) {
        json_printer_opts_.numeric_ids = true;
    }

    if (vm.count("tree") > 0U) {
        printer_ = printer_t::tree;
    }
    else if (vm.count("reverse-tree") > 0U) {
        printer_ = printer_t::reverse_tree;
    }
    else if (vm.count("graphviz") > 0U) {
        printer_ = printer_t::graphviz;
    }
    else if (vm.count("topological-sort") > 0U) {
        printer_ = printer_t::topological_sort;
    }
    else if (vm.count("cycles") > 0U) {
        printer_ = printer_t::cycles;
    }
    else if (vm.count("plantuml") > 0U) {
        printer_ = printer_t::plantuml;
    }
    else if (vm.count("json") > 0U) {
        printer_ = printer_t::json;
    }
}

int config_t::verbosity() const noexcept { return verbosity_; }

const boost::optional<boost::filesystem::path> &
config_t::log_file() const noexcept
{
    return log_file_;
}

const boost::optional<boost::filesystem::path> &
config_t::compilation_database_directory() const noexcept
{
    return compilation_database_directory_;
}

const boost::optional<boost::filesystem::path> &
config_t::output_file() const noexcept
{
    return output_file_;
}

const std::vector<boost::filesystem::path> &
config_t::translation_unit() const noexcept
{
    return translation_unit_;
}

const boost::optional<boost::filesystem::path> &
config_t::relative_to() const noexcept
{
    return relative_to_;
}

void config_t::relative_to(std::string rt) { relative_to_ = std::move(rt); };

const boost::optional<boost::filesystem::path> &
config_t::dependants_of() const noexcept
{
    return dependants_of_;
}

void config_t::dependants_of(std::string file)
{
    dependants_of_ = std::move(file);
}

bool config_t::filenames_only() const noexcept { return filenames_only_; };

bool config_t::relative_only() const noexcept { return relative_only_; }

void config_t::relative_only(bool ro) noexcept { relative_only_ = ro; }

bool config_t::translation_units_only() const noexcept
{
    return translation_units_only_;
}

void config_t::translation_units_only(bool tuo) noexcept
{
    translation_units_only_ = tuo;
}

const boost::optional<std::string> &config_t::title() const noexcept
{
    return title_;
}

printer_t config_t::printer() const noexcept { return printer_; }

void config_t::printer(printer_t printer) noexcept { printer_ = printer; }

unsigned config_t::jobs() const noexcept { return jobs_; }

void config_t::jobs(unsigned j) noexcept { jobs_ = j; }

const std::vector<std::string> &config_t::add_compile_flag() const noexcept
{
    return add_compile_flag_;
}

const std::vector<std::string> &config_t::remove_compile_flag() const noexcept
{
    return remove_compile_flag_;
}

const std::string &config_t::cli_arguments() const noexcept
{
    return cli_arguments_;
}

const json_printer_opts_t &config_t::json_printer_opts() const noexcept
{
    return json_printer_opts_;
}

json_printer_opts_t &config_t::json_printer_opts() noexcept
{
    return json_printer_opts_;
}

boost::filesystem::path config_t::resolve_path(
    const boost::filesystem::path &p) const
{
    auto result{p};
    if (p.is_absolute())
        return p;

    if (relative_to_)
        result = boost::filesystem::path{*relative_to_} / p;

    if (result.is_relative())
        result = util::to_absolute_path(result);

    return result;
}
} // namespace clang_include_graph
