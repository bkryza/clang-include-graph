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
                  << std::endl;
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

    if (relative_to_.has_value() && filenames_only_) {
        std::cerr << "ERROR: --relative-to and --names-only cannot be enabled "
                     "at the same time"
                  << " - aborting..." << std::endl;
        exit(-1);
    }

    if (vm.count("translation-unit") == 1) {
        translation_unit_ =
            util::to_absolute_path(vm["translation-unit"].as<std::string>());
        if (!translation_unit_) {
            std::cerr << "ERROR: Cannot find translation unit source at "
                      << vm["translation-unit"].as<std::string>()
                      << " - aborting..." << std::endl;
            exit(-1);
        }
    }

    if (vm.count("tree") + vm.count("graphviz") + vm.count("topological-sort") >
        1) {
        std::cerr << "ERROR: Only one output method can be selected at a time"
                  << " - aborting..." << std::endl;
        exit(-1);
    }

    if (vm.count("tree") > 0U) {
        printer_ = printer_t::tree;
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
};

const boost::optional<std::string> &config_t::translation_unit() const noexcept
{
    return translation_unit_;
};

const boost::optional<std::string> &config_t::relative_to() const noexcept
{
    return relative_to_;
};

bool config_t::filenames_only() const noexcept { return filenames_only_; };

bool config_t::relative_only() const noexcept { return relative_only_; }

printer_t config_t::printer() const noexcept { return printer_; }

} // namespace clang_include_graph
