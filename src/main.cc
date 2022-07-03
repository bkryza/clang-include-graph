/**
 * src/main.cc
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
#include "include_graph.h"
#include "include_graph_graphviz_printer.h"
#include "include_graph_parser.h"
#include "include_graph_topological_sort_printer.h"
#include "include_graph_tree_printer.h"

#include <boost/program_options.hpp>

#include <iostream>
#include <memory>
#include <set>

#ifndef LIBCLANG_VERSION_STRING
#define LIBCLANG_VERSION_STRING "0.0.0"
#endif

namespace po = boost::program_options;

void print_version();

void process_command_line_options(int argc, char **argv, po::variables_map &vm,
    clang_include_graph::config_t &config);

int main(int argc, char **argv)
{
    using namespace clang_include_graph;

    include_graph_t include_graph;
    config_t config;
    po::variables_map vm;

    process_command_line_options(argc, argv, vm, config);

    if (config.verbose)
        std::cout << "=== Loading compilation database from "
                  << config.compilation_database_directory.value() << std::endl;

    // Parse translation units and build the include graph
    include_graph_parser_t include_graph_parser{config};
    include_graph_parser.parse(include_graph);
    include_graph.build();

    // Select path printer based on config
    std::unique_ptr<path_printer_t> path_printer =
        path_printer_t::from_config(config);

    // Generate output using selected printer
    if (config.printer == printer_t::tree) {
        if (config.verbose)
            std::cout << "=== Printing include graph tree" << '\n';

        include_graph.build_dag();

        include_graph_tree_printer_t<std::decay<
            decltype(include_graph_parser.translation_units())>::type>
            printer{*path_printer, include_graph_parser.translation_units()};
        printer.print(include_graph);
    }
    else if (config.printer == printer_t::topological_sort) {
        if (config.verbose)
            std::cout
                << "=== Printing include graph sorted in topological order"
                << '\n';

        include_graph.build_dag();

        include_graph_topological_sort_printer_t printer{*path_printer};
        printer.print(include_graph);
    }
    else if (config.printer == printer_t::graphviz) {
        if (config.verbose)
            std::cout
                << "=== Printing include graph sorted in topological order"
                << '\n';

        include_graph_graphviz_printer_t printer{*path_printer};
        printer.print(include_graph);
    }
    else {
        std::cout << "ERROR: Invalid output printer - aborting..." << std::endl;
        exit(-1);
    }
}

void process_command_line_options(int argc, char **argv, po::variables_map &vm,
    clang_include_graph::config_t &config)
{
    po::options_description options("clang-include-graph options");

    // clang-format off
    options.add_options()
        ("help,h", "Print help message and exit")
        ("version,V", "Print program version and exit")
        ("verbose,v", "Print verbose information during processing")
        ("compilation-database-dir,d", po::value<std::string>(),
            "Path to compilation database directory (default $PWD)")
        ("translation-unit,u", po::value<std::string>(),
            "Process a single source file from compilation database")
        ("relative-to,r", po::value<std::string>(),
            "Generate paths relative to path (except for system headers)")
        ("names-only,n", "Print only file names")
        ("relative-only,l",
            "Include only files relative to 'relative-to' directory")
        ("topological-sort,s",
            "Print output includes and translation units in topological"
            "sort order")
        ("tree,t", "Print output graph in tree form")
        ("graphviz,g", "Print output graph in GraphViz format");
    // clang-format on

    try {
        po::store(po::parse_command_line(argc, argv, options), vm);
        po::notify(vm);
    }
    catch (const po::error &e) {
        std::cerr << "ERROR: Invalid options - " << e.what() << std::endl;
        exit(-1);
    }

    if (vm.count("help")) {
        std::cout << options << "\n";
        exit(0);
    }

    if (vm.count("version")) {
        print_version();
        exit(0);
    }

    config.init(vm);
}

void print_version()
{
    std::cout << "clang-include-graph 0.1.0" << std::endl;
    std::cout << "Copyright (C) 2022-present Bartek Kryza <bkryza@gmail.com>"
              << '\n';
    std::cout << "Built with libclang: " << LIBCLANG_VERSION_STRING
              << std::endl;
}
