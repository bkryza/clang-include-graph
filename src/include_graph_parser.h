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

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>

#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace clang_include_graph {

void print_diagnostics(const CXTranslationUnit &tu);

void inclusion_visitor(CXFile cx_file, CXSourceLocation *inclusion_stack,
    unsigned include_len, CXClientData include_graph_ptr);

class include_graph_parser_t {
public:
    include_graph_parser_t(const config_t &config)
        : index_{clang_createIndex(0, 0)}
        , config_{config}
    {
    }

    ~include_graph_parser_t() { clang_disposeIndex(index_); }

    void parse(include_graph_t &include_graph)
    {
        include_graph.init(config_);

        auto error = CXCompilationDatabase_NoError;
        auto database = clang_CompilationDatabase_fromDirectory(
            config_.compilation_database_directory.value().c_str(), &error);

        CXCompileCommands compile_commands;
        if (config_.translation_unit.has_value()) {
            auto tu_path = config_.translation_unit.value();

            translation_units_.emplace(tu_path);
            compile_commands = clang_CompilationDatabase_getCompileCommands(
                database, tu_path.c_str());

            if (compile_commands == nullptr) {
                std::cerr << "ERROR: Cannot find " << tu_path
                          << " in compilation database - aborting...";
                exit(-1);
            }
        }
        else {
            // Parse entire compilation database
            compile_commands =
                clang_CompilationDatabase_getAllCompileCommands(database);
        }

        auto compile_commands_size =
            clang_CompileCommands_getSize(compile_commands);

        if (compile_commands_size == 0) {
            std::cerr
                << "Cannot find compilation commands in compilation database"
                << std::endl;
            exit(-1);
        }

        for (auto command_it = 0U; command_it < compile_commands_size;
             command_it++) {
            CXCompileCommand command =
                clang_CompileCommands_getCommand(compile_commands, command_it);

            std::string current_file{
                clang_getCString(clang_CompileCommand_getFilename(command))};
            auto include_path = boost::filesystem::canonical(
                boost::filesystem::path(current_file));
            auto include_path_str = include_path.string();
            translation_units_.emplace(include_path_str);

            if (config_.verbose)
                std::cout << "=== Parsing translation unit: "
                          << include_path_str << std::endl;

            std::vector<std::string> args;
            std::vector<const char *> args_cstr;
            args.reserve(clang_CompileCommand_getNumArgs(command));

            for (auto i = 0U; i < clang_CompileCommand_getNumArgs(command);
                 i++) {
                args.push_back(
                    clang_getCString(clang_CompileCommand_getArg(command, i)));
                args_cstr.push_back(args[i].c_str());
            }

            // Remove the file name from the arguments list
            args.pop_back();
            args_cstr.pop_back();

            CXTranslationUnit unit =
                clang_parseTranslationUnit(index_, include_path_str.c_str(),
                    args_cstr.data(), args_cstr.size(), nullptr, 0,
                    CXTranslationUnit_DetailedPreprocessingRecord |
                        CXTranslationUnit_IgnoreNonErrorsFromIncludedFiles |
                        CXTranslationUnit_KeepGoing);

            if (unit == nullptr) {
                std::cerr
                    << "ERROR: Unable to parse translation unit - aborting..."
                    << std::endl;
                exit(-1);
            }

            if (config_.verbose)
                print_diagnostics(unit);

            clang_getInclusions(unit, inclusion_visitor, &include_graph);

            clang_disposeTranslationUnit(unit);
        }
    }

    const std::set<std::string> &translation_units() const
    {
        return translation_units_;
    }

private:
    CXIndex index_;
    const config_t &config_;
    std::set<std::string> translation_units_;
};

void print_diagnostics(const CXTranslationUnit &tu)
{
    auto no = clang_getNumDiagnostics(tu);
    for (auto i = 0u; i != no; ++i) {
        auto diag = clang_getDiagnostic(tu, i);
        auto diag_loc = clang_getDiagnosticLocation(diag);
        CXString diag_file;
        unsigned line;
        clang_getPresumedLocation(diag_loc, &diag_file, &line, nullptr);
        std::string text = clang_getCString(clang_getDiagnosticSpelling(diag));

        std::cout << "[" << clang_getCString(diag_file) << ":" << line << "] "
                  << text << std::endl;
    }
}

bool is_relative(const std::string &filepath, const std::string &directory)
{
    return boost::starts_with(filepath, directory);
}

void inclusion_visitor(CXFile cx_file, CXSourceLocation *inclusion_stack,
    unsigned include_len, CXClientData include_graph_ptr)
{
    auto *include_graph = static_cast<include_graph_t *>(include_graph_ptr);

    auto relative_only = include_graph->relative_only();
    auto relative_to = include_graph->relative_to();

    auto cx_file_name = clang_getFileName(cx_file);
    std::string file_name = clang_getCString(cx_file_name);
    auto include_path =
        boost::filesystem::canonical(boost::filesystem::path(file_name));

    std::vector<std::string> includes;

    auto add_to_edges = [&](const std::string &from, const std::string &to,
                            bool is_translation_unit) {
        if (!relative_only ||
            (is_relative(from, relative_to.value()) &&
                is_relative(to, relative_to.value())))
            include_graph->add_edge(to, from, is_translation_unit);
    };

    if (inclusion_stack != nullptr) {
        for (auto i = 0U; i < include_len; i++) {

            if (i > 0) {
                clang_getSpellingLocation(inclusion_stack[i - 1], &cx_file,
                    nullptr, nullptr, nullptr);
                cx_file_name = clang_getFileName(cx_file);
                file_name = clang_getCString(cx_file_name);
                include_path = boost::filesystem::canonical(
                    boost::filesystem::path(file_name));
            }

            CXFile cx_included_from;

            clang_getSpellingLocation(inclusion_stack[i], &cx_included_from,
                nullptr, nullptr, nullptr);

            if (cx_included_from != nullptr) {
                auto cx_from_file_name = clang_getFileName(cx_included_from);
                std::string f = clang_getCString(cx_from_file_name);

                auto from_path =
                    boost::filesystem::canonical(boost::filesystem::path(f));

                bool is_translation_unit{i == 0};
                add_to_edges(from_path.string(), include_path.string(),
                    is_translation_unit);
            }
        }
    }
}

} // namespace clang_include_graph

#endif // CLANG_INCLUDE_GRAPH_INCLUDE_GRAPH_PARSER_H
