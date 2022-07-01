/**
 * main.cc
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

#include <clang-c/Index.h>
#include <clang-c/CXCompilationDatabase.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <filesystem>
#include <set>
#include <vector>

using namespace std;
namespace po = boost::program_options;

#ifndef LIBCLANG_VERSION_STRING
#define LIBCLANG_VERSION_STRING "0.0.0"
#endif

using edges_t = std::set<std::pair<std::string, std::string>>;
using vertices_t = std::set<std::string>;
using vertices_ids_t = std::map<std::string, unsigned int>;
using vertices_names_t = std::map<unsigned int, std::string>;

using graph_t =
        boost::adjacency_list<boost::setS, boost::vecS, boost::directedS>;

struct include_path_t {
    using kind = boost::vertex_property_tag;
};

using graph_property_t = boost::property<include_path_t, std::string>;

void print_version();

void process_command_line_options(int argc, char **argv, po::variables_map &vm);

void print_diagnostics(const CXTranslationUnit &tu);

void
inclusion_visitor(CXFile cx_file, CXSourceLocation *inclusion_stack, unsigned include_len,
                  CXClientData edges_ptr);

std::optional<std::string> to_absolute_path(std::string_view relative_path);

void print_include_topological_order(graph_t &include_graph, const vertices_names_t &vertices_names);

void print_include_tree(const std::set<std::string> &translation_units, graph_t &include_graph,
                        const vertices_names_t &vertices_names, const vertices_ids_t &vertices_ids);

void print_tu_subtree(const long tu_id, const int level, graph_t &include_graph,
                      const vertices_names_t &vertices_names, const vertices_ids_t &vertices_ids,
                      std::vector<bool> continuation_line);

std::string current_file;


int main(int argc, char **argv) {
    edges_t edges;
    graph_t include_graph;
    vertices_t vertices;
    vertices_ids_t vertices_ids;
    vertices_names_t vertices_names;
    std::set<std::string> translation_units;


    po::variables_map vm;
    process_command_line_options(argc, argv, vm);

    bool verbose{false};
    if (vm.count("verbose") == 1)
        verbose = true;

    auto compilation_database_directory{to_absolute_path(".")};

    if (vm.count("compilation-database-dir") == 1) {
        compilation_database_directory = to_absolute_path(vm["compilation-database-dir"].as<std::string>());
    }

    if (!compilation_database_directory) {
        std::cerr << "ERROR: Cannot find compilation database - aborting...";
        exit(-1);
    }

    if (verbose)
        cout << "=== Loading compilation database from " << compilation_database_directory.value() << std::endl;

    CXIndex index = clang_createIndex(0, 0);

    auto error = CXCompilationDatabase_NoError;
    auto database = clang_CompilationDatabase_fromDirectory(compilation_database_directory.value().c_str(), &error);

    CXCompileCommands compile_commands;
    if (vm.count("translation-unit") == 1) {
        auto tu_path = to_absolute_path(vm["translation-unit"].as<std::string>());
        if (!tu_path) {
            std::cerr << "ERROR: Cannot find translation unit source at " << vm["translation-unit"].as<std::string>()
                      << " - aborting...";
            exit(-1);
        }

        translation_units.emplace(tu_path.value());
        compile_commands = clang_CompilationDatabase_getCompileCommands(database, tu_path.value().c_str());

        if (compile_commands == nullptr) {
            std::cerr << "ERROR: Cannot find " << tu_path.value() << " in compilation database - aborting...";
            exit(-1);
        }
    } else {
        // Parse entire compilation database
        compile_commands = clang_CompilationDatabase_getAllCompileCommands(database);
    }

    auto compile_commands_size = clang_CompileCommands_getSize(compile_commands);

    if (compile_commands_size == 0) {
        std::cerr << "Cannot find compilation commands in compilation database" << std::endl;
        exit(-1);
    }

    for (auto command_it = 0U; command_it < compile_commands_size; command_it++) {
        CXCompileCommand command = clang_CompileCommands_getCommand(compile_commands, command_it);

        current_file = clang_getCString(clang_CompileCommand_getFilename(command));
        auto include_path = std::filesystem::canonical(std::filesystem::path(current_file));
        auto include_path_str = include_path.string();
        translation_units.emplace(include_path_str);

        if (verbose)
            cout << "=== Processing translation unit: " << include_path_str << std::endl;

        std::vector<std::string> args;
        std::vector<const char *> args_cstr;
        args.reserve(clang_CompileCommand_getNumArgs(command));

        for (auto i = 0U; i < clang_CompileCommand_getNumArgs(command); i++) {
            args.push_back(clang_getCString(clang_CompileCommand_getArg(command, i)));
            args_cstr.push_back(args[i].c_str());
        }

        // Remove the file name from the arguments list
        args.pop_back();
        args_cstr.pop_back();

        CXTranslationUnit unit = clang_parseTranslationUnit(
                index,
                include_path_str.c_str(), args_cstr.data(), args_cstr.size(),
                nullptr, 0,
                CXTranslationUnit_DetailedPreprocessingRecord | CXTranslationUnit_IgnoreNonErrorsFromIncludedFiles |
                CXTranslationUnit_KeepGoing);


        if (unit == nullptr) {
            cerr << "Unable to parse translation unit. Quitting." << endl;
            exit(-1);
        }

        print_diagnostics(unit);

        clang_getInclusions(unit,
                            inclusion_visitor,
                            &edges);

        clang_disposeTranslationUnit(unit);
    }

    // Build mapping from vertices to paths
    for (const auto &[to, from]: edges) {
        vertices.insert(to);
        vertices.insert(from);
    }

    unsigned vertice_id = 0;
    for (const auto &v: vertices) {
        vertices_ids[v] = vertice_id;
        vertices_names[vertice_id] = v;
        vertice_id++;
    }

    for (const auto &[to, from]: edges) {
        boost::add_edge(vertices_ids[from], vertices_ids[to], include_graph);
    }


    if (vm.count("tree")) {
        print_include_tree(translation_units, include_graph, vertices_names, vertices_ids);
    } else {
        print_include_topological_order(include_graph, vertices_names);
    }

    clang_disposeIndex(index);
}

void print_include_topological_order(graph_t &include_graph, const vertices_names_t &vertices_names) {
    std::vector<unsigned int> include_order;
    boost::topological_sort(include_graph, std::back_inserter(include_order));

    for (const auto id: include_order) {
        cout << vertices_names.at(id) << endl;
    }
}

void print_include_tree(const std::set<std::string> &translation_units, graph_t &include_graph,
                        const vertices_names_t &vertices_names, const vertices_ids_t &vertices_ids) {
    boost::graph_traits<graph_t>::adjacency_iterator it, it_end;

    for (const auto &tu: translation_units) {
        std::cout << tu << '\n';
        const auto tu_id = vertices_ids.at(tu);

        print_tu_subtree(tu_id, 0, include_graph, vertices_names, vertices_ids, {});
    }
}

void print_tu_subtree(const long tu_id, const int level, graph_t &include_graph,
                      const vertices_names_t &vertices_names, const vertices_ids_t &vertices_ids,
                      std::vector<bool> continuation_line) {
    boost::graph_traits<graph_t>::adjacency_iterator it, it_end;

    boost::tie(it, it_end) = boost::adjacent_vertices(tu_id, include_graph);
    for (; it != it_end; ++it) {
        auto continuation_line_tmp = continuation_line;
        if (level > 0)
            for (auto i = 0; i < level; i++) {
                if (i % 4 == 0 && continuation_line[i / 4])
                    std::cout << "│";
                else
                    std::cout << " ";
            }

        if (std::next(it) == it_end) {
            std::cout << "└── ";
            continuation_line_tmp.push_back(false);
        } else {
            std::cout << "├── ";
            continuation_line_tmp.push_back(true);
        }

        std::cout << vertices_names.at(*it) << '\n';

        print_tu_subtree(*it, level + 4, include_graph, vertices_names, vertices_ids, continuation_line_tmp);
    }
}

void process_command_line_options(int argc, char **argv, po::variables_map &vm) {
    po::options_description options("clang-include-graph options");
    options.add_options()
            ("help,h", "Print help message and exit")
            ("version,V", "Print program version and exit")
            ("verbose,v", "Print verbose information during processing")
            ("compilation-database-dir,d", po::value<std::string>(),
             "Path to compilation database directory (default $PWD)")
            ("translation-unit,u", po::value<std::string>(), "Process a single source file from compilation database")
            ("tree,t", "Print output graph in tree form");


    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);


    if (vm.count("help")) {
        cout << options << "\n";
        exit(0);
    }

    if (vm.count("version")) {
        print_version();
        exit(0);
    }
}

void print_version() {
    std::cout << "clang-include-graph 0.1.0" << std::endl;
    std::cout << "Copyright (C) 2022-present Bartek Kryza <bkryza@gmail.com>"
              << '\n';
    std::cout << "Built with libclang: "
              << LIBCLANG_VERSION_STRING << std::endl;
}

void print_diagnostics(const CXTranslationUnit &tu) {
    auto no = clang_getNumDiagnostics(tu);
    for (auto i = 0u; i != no; ++i) {
        auto diag = clang_getDiagnostic(tu, i);
        auto diag_loc = clang_getDiagnosticLocation(diag);
        CXString diag_file;
        unsigned line;
        clang_getPresumedLocation(diag_loc, &diag_file, &line, nullptr);
        std::string text = clang_getCString(clang_getDiagnosticSpelling(diag));

        cout << "[" << clang_getCString(diag_file) << ":" << line << "] " << text << std::endl;
    }
}

void
inclusion_visitor(CXFile cx_file, CXSourceLocation *inclusion_stack, unsigned include_len,
                  CXClientData edges_ptr) {

    auto *edge_set = static_cast<edges_t *>(edges_ptr);

    auto cx_file_name = clang_getFileName(cx_file);
    std::string file_name = clang_getCString(cx_file_name);
    auto include_path = std::filesystem::canonical(std::filesystem::path(file_name));

    std::vector<std::string> includes;
    includes.push_back(include_path.string());

    if (inclusion_stack != nullptr) {
        for (auto i = 0U; i < include_len; i++) {

            if (i > 0) {
                clang_getSpellingLocation(inclusion_stack[i - 1],
                                          &cx_file, nullptr, nullptr, nullptr
                );
                cx_file_name = clang_getFileName(cx_file);
                file_name = clang_getCString(cx_file_name);
                include_path = std::filesystem::canonical(std::filesystem::path(file_name));
            }

            CXFile cx_included_from;

            clang_getSpellingLocation(inclusion_stack[i],
                                      &cx_included_from, nullptr, nullptr, nullptr
            );

            if (cx_included_from != nullptr) {
                auto cx_from_file_name = clang_getFileName(cx_included_from);
                std::string f = clang_getCString(cx_from_file_name);

                auto from_path = std::filesystem::canonical(std::filesystem::path(f));

                includes.push_back(from_path.string());

                edge_set->insert(std::pair(include_path.string(), from_path.string()));
            }
        }
    }
}

std::optional<std::string> to_absolute_path(std::string_view relative_path) {
    try {
        return std::filesystem::canonical(std::filesystem::path(relative_path)).string();
    }
    catch (const std::filesystem::filesystem_error &e) {
        return {};
    }
}
