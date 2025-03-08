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

#include <boost/optional/optional.hpp>
#include <boost/program_options/variables_map.hpp>

#include <cstdlib>
#include <iostream>
#include <ostream>

namespace clang_include_graph {

void config_t::init(boost::program_options::variables_map &vm)
{
    if (vm.count("verbose") == 1) {
        verbose_ = true;
    }

    compilation_database_directory_ = util::to_absolute_path(".");

    if (vm.count("compilation-database-dir") == 1) {
        compilation_database_directory_ = util::to_absolute_path(
            vm["compilation-database-dir"].as<std::string>());
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
            util::to_absolute_path(vm["translation-unit"].as<std::string>());
        if (!translation_unit_) {
            std::cerr << "ERROR: Cannot find translation unit source at "
                      << vm["translation-unit"].as<std::string>()
                      << " - aborting..." << '\n';
            exit(-1);
        }
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
        dependants_of_ =
            util::to_absolute_path(vm["dependants-of"].as<std::string>());
    }

    if (vm.count("translation-units-only") == 1) {
        translation_units_only_ = true;
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
}

bool config_t::verbose() const noexcept { return verbose_; }

const boost::optional<std::string> &
config_t::compilation_database_directory() const noexcept
{
    return compilation_database_directory_;
}

const boost::optional<std::string> &config_t::translation_unit() const noexcept
{
    return translation_unit_;
}

const boost::optional<std::string> &config_t::relative_to() const noexcept
{
    return relative_to_;
}

void config_t::relative_to(std::string rt) { relative_to_ = std::move(rt); };

const boost::optional<std::string> &config_t::dependants_of() const noexcept
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

printer_t config_t::printer() const noexcept { return printer_; }

void config_t::printer(printer_t printer) noexcept { printer_ = printer; }

} // namespace clang_include_graph
