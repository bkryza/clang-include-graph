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

namespace {
using visitor_context_t = std::pair<include_graph_t &, std::string>;
}

void print_diagnostics(const CXTranslationUnit &tu);

enum CXChildVisitResult inclusion_cursor_visitor(
    CXCursor cursor, CXCursor parent, CXClientData include_graph_ptr);

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

            auto tu_path = boost::filesystem::canonical(
                boost::filesystem::path(current_file));

            auto include_path_str = tu_path.string();
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
                if (config_.verbose)
                    print_diagnostics(unit);

                std::cerr
                    << "ERROR: Unable to parse translation unit - aborting..."
                    << std::endl;
                exit(-1);
            }

            if (config_.verbose)
                print_diagnostics(unit);

            visitor_context_t visitor_context{include_graph, tu_path.string()};

            CXCursor start_cursor = clang_getTranslationUnitCursor(unit);
            clang_visitChildren(
                start_cursor, inclusion_cursor_visitor, &visitor_context);

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

        std::cout << "=== [" << clang_getCString(diag_file) << ":" << line
                  << "] " << text << std::endl;
    }
}

bool is_relative(const std::string &filepath, const std::string &directory)
{
    return boost::starts_with(filepath, directory);
}

enum CXChildVisitResult inclusion_cursor_visitor(
    CXCursor cursor, CXCursor /*parent*/, CXClientData include_graph_ptr)
{
    auto &include_graph =
        std::get<0>(*static_cast<visitor_context_t *>(include_graph_ptr));

    auto tu_path =
        std::get<1>(*static_cast<visitor_context_t *>(include_graph_ptr));

    auto relative_only = include_graph.relative_only();
    auto relative_to = include_graph.relative_to();

    if (clang_getCursorKind(cursor) == CXCursor_InclusionDirective) {
        CXFile source_file;
        clang_getFileLocation(clang_getCursorLocation(cursor), &source_file,
            nullptr, nullptr, nullptr);

        std::string source_file_str{
            clang_getCString(clang_getFileName(source_file))};

        CXFile included_file = clang_getIncludedFile(cursor);

        std::string included_file_str{
            clang_getCString(clang_getFileName(included_file))};

        auto included_path = boost::filesystem::canonical(
            boost::filesystem::path(included_file_str));

        auto from_path = boost::filesystem::canonical(
            boost::filesystem::path(source_file_str));

        if (!relative_only ||
            (is_relative(from_path.string(), relative_to.value()) &&
                is_relative(included_path.string(), relative_to.value()))) {
            include_graph.add_edge(included_path.string(), from_path.string(),
                tu_path == from_path);
        }
    }
    return CXChildVisit_Continue;
}

} // namespace clang_include_graph

#endif // CLANG_INCLUDE_GRAPH_INCLUDE_GRAPH_PARSER_H
