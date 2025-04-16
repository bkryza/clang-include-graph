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
#include "compilation_database.h"
#include "config.h"
#include "glob/glob.hpp"
#include "include_graph.h"
#include "util.h"

#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/asio/post.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/range/algorithm.hpp>

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
    auto database = clang_CompilationDatabase_fromDirectory(
        config_.compilation_database_directory().value().c_str(), &error);

    if (error != CXCompilationDatabase_NoError) {
        LOG(error) << "Failed to load compilation database from "
                   << config_.compilation_database_directory().value() << '\n';
        exit(-1);
    }

    auto compilation_database_files = get_all_files(database);
    const auto is_fixed = compilation_database_files.empty();

    if (is_fixed) {
        LOG(debug) << "Using fixed compilation database";
    }

    std::set<boost::filesystem::path> compilation_database_files_absolute;

    // Make sure compilation database file paths are absolute and canonical
    boost::range::transform(compilation_database_files,
        std::inserter(compilation_database_files_absolute,
            compilation_database_files_absolute.begin()),
        [&](const boost::filesystem::path &p) {
            auto result = p;
            LOG(trace) << "Resolving compilation database file: "
                       << result.string() << '\n';
            if (!result.is_absolute()) {
                result =
                    config_.compilation_database_directory().value() / result;
            }

            return boost::filesystem::weakly_canonical(result);
        });

    // First check if the glob patterns contain only negative patterns
    bool only_negative_glob_patterns{!config_.translation_unit().empty()};
    for (const auto &glob : config_.translation_unit()) {
        if (!glob.string().empty() && glob.string()[0] != '!') {
            only_negative_glob_patterns = false;
            break;
        }
    }

    std::vector<CXCompileCommands> matching_compile_commands;

    if (!config_.translation_unit().empty()) {
        std::set<boost::filesystem::path> glob_files_absolute;

        // First find all files matching whitelisted glob patterns
        // i.e. not starting with an `!`
        for (const auto &glob : config_.translation_unit()) {
            if (glob.string().empty() || glob.string()[0] == '!')
                continue;

            boost::filesystem::path absolute_glob_path{glob.string()};

            if (!absolute_glob_path.is_absolute())
                absolute_glob_path =
                    boost::filesystem::current_path() / absolute_glob_path;

            LOG(debug) << "Searching glob path " << absolute_glob_path.string()
                       << '\n';

            auto matches = glob::glob(absolute_glob_path.string(), true, false);

            LOG(debug) << "Found " << matches.size() << " files matching glob: "
                       << absolute_glob_path.string();

            for (const auto &match : matches) {
                const auto path = boost::filesystem::weakly_canonical(match);

                assert(path.is_absolute());

                glob_files_absolute.emplace(std::move(path));
            }
        }

        LOG(debug)
            << "Found " << glob_files_absolute.size()
            << " compilation database files matching positive glob patterns";

        // Now remove all paths which match the blacklisted glob patterns
        // i.e. start with `!`
        for (const auto &glob : config_.translation_unit()) {
            if (glob.string().size() < 2 || glob.string()[0] != '!')
                continue;

            boost::filesystem::path absolute_glob_path{glob.string().substr(1)};
            auto matches = glob::glob(absolute_glob_path.string(), true, false);

            for (const auto &match : matches) {
                const auto path = boost::filesystem::weakly_canonical(match);

                assert(path.is_absolute());

                LOG(trace) << "Removing match " << path.string()
                           << " based on glob " << glob.string();

                glob_files_absolute.erase(path);
            }
        }

        // Calculate intersection between glob matches and compilation database
        std::vector<std::string> matching_files;

        for (const auto &gm : glob_files_absolute) {
            auto preffered_path = gm;
            preffered_path.make_preferred();

            if (is_fixed ||
                boost::algorithm::any_of_equal(
                    compilation_database_files_absolute, gm) ||
                boost::algorithm::any_of_equal(
                    compilation_database_files_absolute, preffered_path)) {
                matching_files.emplace_back(gm.string());

                LOG(trace) << "Found matching compilation database file: "
                           << gm.string();
            }
        }

        for (const auto &file : matching_files) {
            matching_compile_commands.emplace_back(
                clang_CompilationDatabase_getCompileCommands(
                    database, file.c_str()));
        }
    }
    else {
        // Parse entire compilation database
        matching_compile_commands.push_back(
            clang_CompilationDatabase_getAllCompileCommands(database));
    }

    if (matching_compile_commands.empty()) {
        LOG(error) << "ERROR: Cannot find matching files "
                   << "in compilation database - aborting...";
        exit(-1);
    }

    boost::asio::thread_pool thread_pool{config_.jobs()};

    LOG(info) << "Starting thread pool with " << config_.jobs() << " threads\n";

    for (auto compile_commands : matching_compile_commands) {
        auto compile_commands_size =
            clang_CompileCommands_getSize(compile_commands);

        if (compile_commands_size == 0 && translation_units().empty()) {
            LOG(error)
                << "ERROR: No compilation database found and no translation "
                   "units specified"
                << '\n';
            exit(-1);
        }

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
                LOG(error) << "ERROR: Cannot find translation unit at "
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
                    LOG(info)
                        << "Parsing translation unit: " << include_path_str
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

                    for (auto i = 0U;
                         i < clang_CompileCommand_getNumArgs(command); i++) {
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

                        LOG(error)
                            << "ERROR: Unable to parse translation unit '"
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
                    clang_visitChildren(start_cursor, inclusion_cursor_visitor,
                        &visitor_context);

                    clang_disposeTranslationUnit(unit);
                });
        }
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
