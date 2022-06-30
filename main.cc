/**
 * main.cc
 *
 * Copyright (c) 2022 Bartek Kryza <bkryza@gmail.com>
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

#include <iostream>
#include <filesystem>
#include <set>
#include <vector>

using namespace std;

using edges_t = std::set<std::pair<std::string, std::string>>;
using vertices_t = std::set<std::string>;
using vertices_ids_t = std::map<std::string, unsigned int>;
using vertices_names_t = std::map<unsigned int, std::string>;

using graph_t =
        boost::adjacency_list<boost::setS, boost::vecS, boost::directedS>;

struct include_path_t{
    using kind = boost::vertex_property_tag;
};

using graph_property_t = boost::property<include_path_t, std::string>;

ostream &operator<<(ostream &stream, const CXString &str) {
    stream << clang_getCString(str);
    clang_disposeString(str);
    return stream;
}

std::string current_file;

void
inclusion_visitor(CXFile cx_file, CXSourceLocation *inclusion_stack, unsigned include_len,
                  CXClientData edges_ptr) {

    auto *edge_set = static_cast<edges_t *>(edges_ptr);

    auto cx_file_name = clang_getFileName(cx_file);
    std::string file_name = clang_getCString(cx_file_name);
    auto include_path = std::filesystem::canonical(std::filesystem::path(file_name));
    //cout << "# " << include_path.string() << " : " << include_len << std::endl;

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
                //cout << "# " << include_path.string() << " : " << include_len << std::endl;
            }

            CXFile cx_included_from;

            clang_getSpellingLocation(inclusion_stack[i],
                                      &cx_included_from, nullptr, nullptr, nullptr
            );

            if (cx_included_from != nullptr) {
                auto cx_from_file_name = clang_getFileName(cx_included_from);
                std::string f = clang_getCString(cx_from_file_name);

                auto from_path = std::filesystem::canonical(std::filesystem::path(f));

                //cout << "### " <<  from_path.string() << std::endl;

                includes.push_back(from_path.string());

                edge_set->insert(std::pair(include_path.string(), from_path.string()));
            }
        }
    }
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

int main(int /*argc*/, char **/*argv*/) {
    edges_t edges;
    graph_t include_graph;
    vertices_t vertices;
    vertices_ids_t vertices_ids;
    vertices_names_t vertices_names;

    CXIndex index = clang_createIndex(0, 0);

    cout << "Loading compilation database";

    auto error = CXCompilationDatabase_NoError;
    auto database = clang_CompilationDatabase_fromDirectory(".", &error);

    auto compile_commands = clang_CompilationDatabase_getAllCompileCommands(database);

    auto compile_commands_size = clang_CompileCommands_getSize(compile_commands);

    if (compile_commands_size == 0) {
        cerr << "Cannot find compilation commands in compilation database" << std::endl;
        exit(-1);
    }

    for (auto command_it = 0U; command_it < compile_commands_size; command_it++) {

        CXCompileCommand command = clang_CompileCommands_getCommand(compile_commands, command_it);

        current_file = clang_getCString(clang_CompileCommand_getFilename(command));
        auto include_path = std::filesystem::canonical(std::filesystem::path(current_file));
        auto include_path_str = include_path.string();

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

    // Build mapping from vertices to
    for (const auto &[to, from]: edges) {
        vertices.insert(to);
        vertices.insert(from);
    }

    unsigned vertice_id = 0;
    for(const auto& v : vertices) {
        vertices_ids[v] = vertice_id;
        vertices_names[vertice_id] = v;
        vertice_id++;
    }

    for (const auto &[to, from]: edges) {
        boost::add_edge(vertices_ids[to], vertices_ids[from], include_graph);
    }

    std::deque<unsigned int> include_order;
    boost::topological_sort(include_graph, std::front_inserter(include_order));

    for(const auto id : include_order) {
        cout << id << " : " << vertices_names[id] << endl;
    }

    clang_disposeIndex(index);
}