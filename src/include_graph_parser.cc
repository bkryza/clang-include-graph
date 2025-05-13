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
#include "include_graph.h"
#include "util.h"

#include <boost/algorithm/string/join.hpp>
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

using visitor_context_t = std::pair<include_graph_t &, std::string>;

void print_diagnostics(const CXTranslationUnit &tu);

enum CXChildVisitResult inclusion_cursor_visitor(
    CXCursor cursor, CXCursor parent, CXClientData include_graph_ptr);

void inclusion_visitor(CXFile cx_file, CXSourceLocation *inclusion_stack,
    unsigned include_len, CXClientData include_graph_ptr);

void process_translation_unit(const config_t &config,
    include_graph_t &include_graph, CXCompileCommand command,
    const boost::filesystem::path &tu_path, std::string &include_path_str,
    CXIndex &index)
{
    LOG(info) << "Parsing translation unit: " << include_path_str << '\n';

    auto flags = static_cast<unsigned int>(
                     CXTranslationUnit_DetailedPreprocessingRecord) |
        static_cast<unsigned int>(
            CXTranslationUnit_IgnoreNonErrorsFromIncludedFiles) |
        static_cast<unsigned int>(CXTranslationUnit_KeepGoing);

    std::vector<std::string> args;
    args.reserve(clang_CompileCommand_getNumArgs(command));

    std::vector<const char *> args_cstr;
    args_cstr.reserve(clang_CompileCommand_getNumArgs(command));

    for (auto i = 0U; i < clang_CompileCommand_getNumArgs(command); i++) {
        std::string arg{
            clang_getCString(clang_CompileCommand_getArg(command, i))};
        args.emplace_back(std::move(arg));
    }

    if (!config.add_compile_flag().empty()) {
        args.insert(
            // Add flags after argv[0]
            args.begin() + 1, config.add_compile_flag().begin(),
            config.add_compile_flag().end());
    }

    for (const auto &flag : config.remove_compile_flag()) {
        args.erase(std::remove_if(args.begin(), args.end(),
                       [&flag](const auto &arg) {
                           return util::match_flag_glob(arg, flag);
                       }),
            args.end());
    }

    for (auto i = 0U; i < args.size(); i++) {
        if (args.at(i) == "-c") {
            // Remove the source file name from the arguments list
            i++;
            continue;
        }

        args_cstr.emplace_back(args.at(i).c_str());
    }

    LOG(trace) << "Parsing " << tu_path << " with the following compile flags: "
               << boost::algorithm::join(args, " ");

    CXTranslationUnit unit{nullptr};

    const CXErrorCode err = clang_parseTranslationUnit2(index,
        include_path_str.c_str(), args_cstr.data(),
        static_cast<int>(args_cstr.size()), nullptr, 0, flags, &unit);

    if (err != CXError_Success) {
        std::string error_str;

        switch (err) {
        case CXError_Success:
            break;

        case CXError_Failure:
            error_str = "clang_parseTranslationUnit2: unknown error";
        case CXError_Crashed:
            error_str = "clang_parseTranslationUnit2: libclang crash";
        case CXError_InvalidArguments:
            error_str =
                "clang_parseTranslationUnit2: invalid compilation arguments";
        case CXError_ASTReadError:
            error_str =
                "clang_parseTranslationUnit2: AST deserialization failed";
        }

        if (unit != nullptr)
            print_diagnostics(unit);

        LOG(error) << "ERROR: Unable to parse translation unit '" << tu_path
                   << "' due to " << error_str << " - aborting..." << '\n';
        exit(-1);
    }

    if (global_logger::get().open_record(
            // NOLINTNEXTLINE
            boost::log::keywords::severity = boost::log::trivial::debug)) {
        print_diagnostics(unit);
    }

    visitor_context_t visitor_context{include_graph, tu_path.string()};

    const CXCursor start_cursor = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(
        start_cursor, inclusion_cursor_visitor, &visitor_context);

    clang_disposeTranslationUnit(unit);
}

bool is_system_header(CXCursor cursor)
{
    auto *tu = clang_Cursor_getTranslationUnit(cursor);

    const CXSourceRange include_range = clang_getCursorExtent(cursor);

    CXToken *tokens = nullptr;
    unsigned num_tokens = 0;
    clang_tokenize(tu, include_range, &tokens, &num_tokens);

    bool is_system = false;
    for (unsigned i = 0; i < num_tokens; ++i) {
        const CXString spelling = clang_getTokenSpelling(tu, tokens[i]);
        const std::string tok = clang_getCString(spelling);
        clang_disposeString(spelling);

        if (tok == "<") {
            is_system = true;
            break;
        }
    }

    clang_disposeTokens(tu, tokens, num_tokens);
    return is_system;
}

std::string get_raw_include_text(CXCursor cursor)
{
    const CXString cx_spelling = clang_getCursorSpelling(cursor);
    const char *cstr = clang_getCString(cx_spelling);
    std::string spelling = (cstr != nullptr) ? cstr : "";
    clang_disposeString(cx_spelling);

    if (spelling.size() >= 2) {
        const char first = spelling.front();
        const char last = spelling.back();
        if ((first == '<' && last == '>') || (first == '"' && last == '"')) {
            return spelling.substr(1, spelling.size() - 2);
        }
    }

    return spelling;
}

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

    std::vector<boost::filesystem::path> translation_unit_patterns =
        config_.translation_unit();

    if (only_negative_glob_patterns && !is_fixed) {
        translation_unit_patterns.emplace_back("**/*");
    }

    if (!translation_unit_patterns.empty()) {
        std::set<boost::filesystem::path> glob_files_absolute;

        LOG(info) << "Resolving whitelist patterns";
        // First find all files matching whitelisted glob patterns
        // i.e. not starting with an `!`
        resolve_whitelist_glob_patterns(
            translation_unit_patterns, glob_files_absolute);

        LOG(info)
            << "Found " << glob_files_absolute.size()
            << " compilation database files matching positive glob patterns";

        LOG(info) << "Resolving blacklist patterns";

        // Now remove all paths which match the blacklisted glob patterns
        // i.e. start with `!`
        filter_blacklist_glob_patterns(
            config_.translation_unit(), glob_files_absolute);

        intersect_glob_matches_with_compilation_database(database, is_fixed,
            compilation_database_files_absolute, matching_compile_commands,
            glob_files_absolute);
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

    LOG(info) << "Found " << matching_compile_commands.size()
              << " matching translation units";

    boost::asio::thread_pool thread_pool{config_.jobs()};

    LOG(info) << "Starting thread pool with " << config_.jobs() << " threads\n";

    for (auto *compile_commands : matching_compile_commands) {
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

            const boost::filesystem::path tu_path = get_canonical_file(command);

            assert(tu_path.is_absolute());

            // Skip translation units which don't actually exist
            if (!exists(tu_path)) {
                LOG(info) << "Skipping translation unit: " << tu_path
                          << " - file not found.";
                continue;
            }

            // Skip compile commands for headers (e.g. precompiled headers)
            if (!tu_path.has_extension() ||
                tu_path.extension().string().find(".h") == 0) {
                continue;
            }

            if (!boost::filesystem::exists(tu_path)) {
                LOG(error) << "ERROR: Cannot find translation unit at "
                           << tu_path << '\n';
                exit(-1);
            }

            auto include_path_str = tu_path.string();
            translation_units_.emplace(include_path_str);

            auto &index = index_;

            boost::asio::post(thread_pool,
                [&config = config_, &include_graph, &index, tu_path,
                    include_path_str, command]() mutable {
                    process_translation_unit(config, include_graph, command,
                        tu_path, include_path_str, index);
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
            LOG(debug)
                << "WARNING: Cannot find header from include directive at "
                << source_file_str << ":" << line << '\n';
            return CXChildVisit_Continue;
        }

        const auto is_system = is_system_header(cursor);

        if (include_graph.exclude_system_headers() && is_system)
            return CXChildVisit_Continue;

        const std::string included_file_str{
            clang_getCString(clang_getFileName(included_file))};

        const auto include_spelling = get_raw_include_text(cursor);

        const boost::filesystem::path included_path =
            boost::filesystem::weakly_canonical(
                boost::filesystem::path(included_file_str));

        const boost::filesystem::path from_path =
            boost::filesystem::weakly_canonical(
                boost::filesystem::path(source_file_str));

        if (!relative_only ||
            (is_relative(from_path, relative_to.value()) &&
                is_relative(included_path, relative_to.value()))) {
            include_graph.add_edge(included_path.string(), from_path.string(),
                include_spelling, tu_path == from_path, is_system);
        }
    }

    return CXChildVisit_Continue;
}

} // namespace clang_include_graph
