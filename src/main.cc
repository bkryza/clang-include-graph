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
#include "include_graph_cycles_printer.h"
#include "include_graph_dependants_printer.h"
#include "include_graph_graphml_printer.h"
#include "include_graph_graphviz_printer.h"
#include "include_graph_json_printer.h"
#include "include_graph_parser.h"
#include "include_graph_plantuml_printer.h"
#include "include_graph_topological_sort_printer.h"
#include "include_graph_tree_printer.h"
#include "util.h"

#include <boost/program_options/detail/parsers.hpp>
#include <boost/program_options/errors.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>

#include <cstdlib>
#include <iostream>
#include <memory>

namespace po = boost::program_options;

#ifndef LIBCLANG_VERSION_STRING
#define LIBCLANG_VERSION_STRING "0.0.0"
#endif

#ifndef GIT_VERSION
#define GIT_VERSION "0.1.0"
#endif

void print_version();

void process_command_line_options(int argc, char **argv, po::variables_map &vm,
    clang_include_graph::config_t &config);

std::ostream &get_output_stream(
    const boost::optional<boost::filesystem::path> &output_path);

int main(int argc, char **argv)
{
    using clang_include_graph::config_t;
    using clang_include_graph::include_graph_cycles_printer_t;
    using clang_include_graph::include_graph_dependants_printer_t;
    using clang_include_graph::include_graph_graphml_printer_t;
    using clang_include_graph::include_graph_graphviz_printer_t;
    using clang_include_graph::include_graph_json_printer_t;
    using clang_include_graph::include_graph_parser_t;
    using clang_include_graph::include_graph_plantuml_printer_t;
    using clang_include_graph::include_graph_t;
    using clang_include_graph::include_graph_topological_sort_printer_t;
    using clang_include_graph::include_graph_tree_printer_t;
    using clang_include_graph::path_printer_t;
    using clang_include_graph::printer_t;

    include_graph_t include_graph;
    config_t config;
    po::variables_map vm;

    process_command_line_options(argc, argv, vm, config);

    LOG(info) << "Loading compilation database from "
              << config.compilation_database_directory().value() << '\n';

    // Parse translation units and build the include graph
    include_graph_parser_t include_graph_parser{config};
    include_graph_parser.parse(include_graph);

    // Select file path printer based on config
    std::unique_ptr<path_printer_t> const path_printer =
        path_printer_t::from_config(config);

    auto &output = get_output_stream(config.output_file());

    // Generate output using selected printer
    if (config.printer() == printer_t::tree) {
        LOG(info) << "Printing include graph tree\n";

        include_graph.build_dag();

        include_graph_tree_printer_t printer{include_graph, *path_printer};

        output << printer;
    }
    else if (config.printer() == printer_t::reverse_tree) {
        LOG(info) << "Printing reverse include graph tree\n";

        include_graph.build_dag();

        include_graph_tree_printer_t printer{include_graph, *path_printer};

        output << printer;
    }
    else if (config.printer() == printer_t::json) {
        LOG(info) << "Printing include graph in JSON Graph Format\n";

        include_graph.build_dag();

        include_graph_json_printer_t printer{include_graph, *path_printer};

        output << printer;
    }
    else if (config.printer() == printer_t::dependants) {
        LOG(info) << "Printing dependants of " << *config.dependants_of()
                  << '\n';

        include_graph.build_dag();

        include_graph_dependants_printer_t printer{
            include_graph, *path_printer};

        output << printer;
    }
    else if (config.printer() == printer_t::topological_sort) {
        LOG(info) << "Printing include graph sorted in topological order\n";

        include_graph.build_dag();

        include_graph_topological_sort_printer_t printer{
            include_graph, *path_printer};

        output << printer;
    }
    else if (config.printer() == printer_t::cycles) {
        LOG(info) << "Printing include graph cycles\n";

        include_graph_cycles_printer_t printer{include_graph, *path_printer};

        output << printer;
    }
    else if (config.printer() == printer_t::graphviz) {
        LOG(info) << "Printing include graph in GraphViz format\n";

        include_graph_graphviz_printer_t printer{include_graph, *path_printer};

        output << printer;
    }
    else if (config.printer() == printer_t::graphml) {
        LOG(info) << "Printing include graph in GraphML format\n";

        include_graph_graphml_printer_t printer{include_graph, *path_printer};

        output << printer;
    }
    else if (config.printer() == printer_t::plantuml) {
        LOG(info) << "Printing include graph in PlantUML format\n";

        include_graph_plantuml_printer_t printer{include_graph, *path_printer};

        output << printer;
    }
    else {
        LOG(error) << "ERROR: Invalid output printer - aborting..." << '\n';
        exit(-1);
    }

    if (config.output_file()) {
        LOG(info) << "Output written to file: "
                  << config.output_file()->string() << '\n';
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
        ("verbose,v", po::value<int>(),
            "Set log verbosity level")
        ("log-file", po::value<std::string>(),
            "Log to specified file instead of console")
        ("jobs,J", po::value<unsigned>(),
            "Number of threads used to parse translation units")
        ("compilation-database-dir,d", po::value<std::string>(),
            "Path to compilation database directory (default: $PWD)")
        ("add-compile-flag", po::value<std::vector<std::string>>(),
             "Add a compile flag to the compilation database")
        ("remove-compile-flag", po::value<std::vector<std::string>>(),
             "Remove a compile flag from the compilation database. "
             "Can contain a '*' to match multiple flags")
        ("translation-unit,u", po::value<std::vector<boost::filesystem::path>>(),
            "Path or glob patterns to match translation units for processing "
            "(relative to $PWD). ")
        ("relative-to,r", po::value<std::string>(),
            "Print paths relative to path (except for system headers)")
        ("names-only,n", "Print only file names")
        ("relative-only,l",
            "Include only files relative to 'relative-to' directory")
        ("output,o", po::value<std::string>(),
            "Write the output to a specified file instead of stdout")
        ("json-numeric-ids", "Use numeric ids for nodes instead of paths")
        ("dependants-of,e", po::value<std::string>(),
            "Print all files that depend on a specific header")
        ("translation-units-only", "Print only translation units")
        ("title", po::value<std::string>(),
            "Graph title that can be added ")
        ("topological-sort,s",
            "Print output includes and translation units in topological"
            "sort order")
        ("tree,t", "Print include graph in tree form")
        ("reverse-tree,T", "Print reverse include graph in tree form")
        ("json,j", "Print include graph in Json Graph format")
        ("cycles,c", "Print include graph cycles, if any")
        ("graphviz,g", "Print include graph in GraphViz format")
        ("graphml,G", "Print include graph in GraphML format")
        ("plantuml,p", "Print include graph in PlantUML format");
    // clang-format on

    try {
        po::store(po::parse_command_line(argc, argv, options), vm);
        po::notify(vm);
    }
    catch (const po::error &e) {
        std::cerr << "ERROR: Invalid options - " << e.what() << '\n';
        exit(-1);
    }

    if (vm.count("help") > 0U) {
        std::cout << options << "\n";
        exit(0);
    }

    if (vm.count("version") > 0U) {
        print_version();
        exit(0);
    }

    config.init(vm, clang_include_graph::util::join_cli_args(argc, argv));

    clang_include_graph::util::setup_logging(
        config.verbosity(), config.log_file());
}

std::ostream &get_output_stream(
    const boost::optional<boost::filesystem::path> &output_path)
{
    if (!output_path) {
        return std::cout;
    }

    try {
        boost::filesystem::create_directories(
            output_path.value().parent_path());
    }
    catch (const boost::filesystem::filesystem_error &e) {
        std::cerr << "ERROR: Cannot create output directory "
                  << output_path.value().parent_path() << ": '" << e.what()
                  << "'" << '\n';
        exit(-1);
    }

    static std::ofstream output_file;

    if (output_file.is_open()) {
        output_file.close();
    }
    output_file.clear();

    output_file.open(output_path->string());
    if (!output_file) {
        LOG(error) << "Failed to open output file: " << output_path->string();
        exit(-1);
    }

    return output_file;
}

void print_version()
{
    std::cout << "clang-include-graph " << GIT_VERSION << '\n';
    std::cout << "Copyright (C) 2022-present Bartek Kryza <bkryza@gmail.com>"
              << '\n';
    std::cout << "Built with libclang: " << LIBCLANG_VERSION_STRING << '\n';
}
