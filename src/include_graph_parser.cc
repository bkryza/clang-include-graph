/**
 * src/include_graph_parser.cc
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

#include "include_graph_parser.h"
#include "config.h"
#include "include_graph.h"
#include "util.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/asio/post.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <clang-c/CXCompilationDatabase.h>

#include <cstdlib>
#include <iostream>
#include <ostream>
#include <set>
#include <string>
#include <utility>
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

include_graph_parser_t::include_graph_parser_t(const config_t &config)
    : index_{clang_createIndex(0, 0)}
    , config_{config}
{
}

include_graph_parser_t::~include_graph_parser_t()
{
    clang_disposeIndex(index_);
}

void include_graph_parser_t::parse(include_graph_t &include_graph)
{
    include_graph.init(config_);

    auto error = CXCompilationDatabase_NoError;
    auto *database = clang_CompilationDatabase_fromDirectory(
        config_.compilation_database_directory().value().c_str(), &error);

    CXCompileCommands compile_commands{nullptr};
    if (config_.translation_unit()) {
        auto tu_path = config_.translation_unit().value();

        translation_units_.emplace(tu_path);
        compile_commands = clang_CompilationDatabase_getCompileCommands(
            database, tu_path.c_str());

        if (compile_commands == nullptr) {
            LOG(error) << "ERROR: Cannot find " << tu_path
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

    if (error !=
        CXCompilationDatabase_NoError) { // compile_commands_size == 0) {
        LOG(error)
            << "ERROR: Cannot find compilation commands in compilation database"
            << '\n';
        exit(-1);
    }

    if (compile_commands_size == 0 && translation_units().empty()) {
        LOG(error) << "ERROR: No compilation database found and no translation "
                      "units specified"
                   << '\n';
        exit(-1);
    }

    boost::asio::thread_pool thread_pool{config_.jobs()};

    LOG(info) << "Starting thread pool with " << config_.jobs() << " threads\n";

    for (auto command_it = 0U; command_it < compile_commands_size;
         command_it++) {
        CXCompileCommand command =
            clang_CompileCommands_getCommand(compile_commands, command_it);

        const boost::filesystem::path current_file{
            clang_getCString(clang_CompileCommand_getFilename(command))};

        // Skip compile commands for headers (e.g. precompiled headers)
        if (!current_file.has_extension() ||
            current_file.extension().string().find(".h") == 0) {
            continue;
        }

        if (!boost::filesystem::exists(current_file)) {
            LOG(error) << "ERROR: Cannot find translation unit at  "
                       << current_file << '\n';
            exit(-1);
        }

        auto tu_path = boost::filesystem::canonical(current_file);

        auto include_path_str = tu_path.string();
        translation_units_.emplace(include_path_str);

        auto &index = index_;

        boost::asio::post(thread_pool,
            [&include_graph, &index, tu_path, include_path_str, command,
                current_file]() {
                LOG(info) << "Parsing translation unit: " << include_path_str
                          << '\n';

                auto flags =
                    static_cast<unsigned int>(
                        CXTranslationUnit_DetailedPreprocessingRecord) |
                    static_cast<unsigned int>(
                        CXTranslationUnit_IgnoreNonErrorsFromIncludedFiles) |
                    static_cast<unsigned int>(CXTranslationUnit_KeepGoing);

                std::vector<std::string> args;
                std::vector<const char *> args_cstr;
                args.reserve(clang_CompileCommand_getNumArgs(command));

                for (auto i = 0U; i < clang_CompileCommand_getNumArgs(command);
                     i++) {
                    std::string arg = clang_getCString(
                        clang_CompileCommand_getArg(command, i));

                    // Skip precompiled headers args
                    if (arg == "-Xclang") {
                        const std::string nextArg{clang_getCString(
                            clang_CompileCommand_getArg(command, i + 1))};
                        if (nextArg.find("pch") != std::string::npos ||
                            nextArg.find("gch") != std::string::npos) {
                            i++;
                            continue;
                        }
                    }

                    args.emplace_back(std::move(arg));
                    args_cstr.emplace_back(args.back().c_str());
                }

                // Remove the file name from the arguments list
                args.pop_back();
                args_cstr.pop_back();

                args.emplace_back("-Wno-unknown-warning-option");
                args_cstr.emplace_back(args.back().c_str());

                CXTranslationUnit unit = clang_parseTranslationUnit(index,
                    include_path_str.c_str(), args_cstr.data(),
                    static_cast<int>(args_cstr.size()), nullptr, 0, flags);

                if (unit == nullptr) {
                    print_diagnostics(unit);

                    LOG(error) << "ERROR: Unable to parse translation unit '"
                               << current_file << "' - aborting..." << '\n';
                    exit(-1);
                }

                if (global_logger::get().open_record(
                        // NOLINTNEXTLINE
                        boost::log::keywords::severity =
                            boost::log::trivial::debug)) {
                    print_diagnostics(unit);
                }

                visitor_context_t visitor_context{
                    include_graph, tu_path.string()};

                const CXCursor start_cursor =
                    clang_getTranslationUnitCursor(unit);
                clang_visitChildren(
                    start_cursor, inclusion_cursor_visitor, &visitor_context);

                clang_disposeTranslationUnit(unit);
            });
    }

    thread_pool.join();

    thread_pool.stop();
}

const std::set<boost::filesystem::path> &
include_graph_parser_t::translation_units() const
{
    return translation_units_;
}

void print_diagnostics(const CXTranslationUnit &tu)
{
    auto no = clang_getNumDiagnostics(tu);
    for (auto i = 0U; i != no; ++i) {
        auto *diag = clang_getDiagnostic(tu, i);
        auto diag_loc = clang_getDiagnosticLocation(diag);
        CXString diagnostic_file;
        unsigned line{0U};
        clang_getPresumedLocation(diag_loc, &diagnostic_file, &line, nullptr);
        const std::string text =
            clang_getCString(clang_getDiagnosticSpelling(diag));

        LOG(error) << "=== [" << clang_getCString(diagnostic_file) << ":"
                   << line << "] " << text << '\n';
    }
}

bool is_relative(const boost::filesystem::path &filepath,
    const boost::filesystem::path &directory)
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
        CXFile source_file{nullptr};
        unsigned line{};
        unsigned column{};
        unsigned offset{};
        clang_getFileLocation(clang_getCursorLocation(cursor), &source_file,
            &line, &column, &offset);

        const std::string source_file_str{
            clang_getCString(clang_getFileName(source_file))};

        CXFile included_file = clang_getIncludedFile(cursor);

        if (included_file == nullptr) {
            std::cerr
                << "WARNING: Cannot find header from include directive at "
                << source_file_str << ":" << line << '\n';
            return CXChildVisit_Continue;
        }

        const std::string included_file_str{
            clang_getCString(clang_getFileName(included_file))};

        auto included_path = boost::filesystem::canonical(
            boost::filesystem::path(included_file_str));

        auto from_path = boost::filesystem::canonical(
            boost::filesystem::path(source_file_str));

        if (!relative_only ||
            (is_relative(from_path, relative_to.value()) &&
                is_relative(included_path, relative_to.value()))) {
            include_graph.add_edge(included_path.string(), from_path.string(),
                tu_path == from_path);
        }
    }
    return CXChildVisit_Continue;
}

} // namespace clang_include_graph
