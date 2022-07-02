//
// Created by bartek on 02.07.22.
//

#ifndef CLANG_INCLUDE_GRAPH_CONFIG_H
#define CLANG_INCLUDE_GRAPH_CONFIG_H

#include "util.h"

#include <boost/program_options.hpp>

#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

namespace clang_include_graph {

struct config_t {
    bool verbose{false};
    std::optional<std::string> compilation_database_directory;
    std::optional<std::string> translation_unit;

    void init(boost::program_options::variables_map &vm)
    {
        if (vm.count("verbose") == 1)
            verbose = true;

        compilation_database_directory = util::to_absolute_path(".");

        if (vm.count("compilation-database-dir") == 1) {
            compilation_database_directory = util::to_absolute_path(
                vm["compilation-database-dir"].as<std::string>());
        }

        if (!compilation_database_directory) {
            std::cerr
                << "ERROR: Cannot find compilation database - aborting...";
            exit(-1);
        }

        if (vm.count("translation-unit") == 1) {
            translation_unit = util::to_absolute_path(
                vm["translation-unit"].as<std::string>());
            if (!translation_unit) {
                std::cerr << "ERROR: Cannot find translation unit source at "
                          << vm["translation-unit"].as<std::string>()
                          << " - aborting...";
                exit(-1);
            }
        }
    }
};

} // namespace clang_include_graph

#endif // CLANG_INCLUDE_GRAPH_CONFIG_H
