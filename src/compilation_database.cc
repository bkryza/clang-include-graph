/**
 * src/compilation_database.cc
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

#include "compilation_database.h"
#include "glob/glob.hpp"

#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <clang-c/CXCompilationDatabase.h>

namespace clang_include_graph {

std::set<boost::filesystem::path> get_all_files(CXCompilationDatabase database)
{
    std::set<boost::filesystem::path> result;

    auto *compile_commands =
        clang_CompilationDatabase_getAllCompileCommands(database);

    auto compile_commands_size =
        clang_CompileCommands_getSize(compile_commands);

    for (auto i = 0U; i < compile_commands_size; i++) {
        auto *command = clang_CompileCommands_getCommand(compile_commands, i);

        auto canonical_file = get_canonical_file(command);

        LOG(trace) << "Found file " << canonical_file.string()
                   << " in compilation database\n";

        result.emplace(std::move(canonical_file));
    }

    return result;
}

void intersect_glob_matches_with_compilation_database(void *database,
    const bool is_fixed,
    const std::set<boost::filesystem::path>
        &compilation_database_files_absolute,
    std::vector<CXCompileCommands> &matching_compile_commands,
    std::set<boost::filesystem::path> &glob_files_absolute)
{
    std::vector<std::string> matching_files;

    for (const auto &gm : glob_files_absolute) {
        assert(gm.is_absolute());

        auto preferred_path = gm;
        preferred_path.make_preferred();

        if (is_fixed ||
            boost::algorithm::any_of_equal(
                compilation_database_files_absolute, gm) ||
            boost::algorithm::any_of_equal(
                compilation_database_files_absolute, preferred_path)) {
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

void filter_blacklist_glob_patterns(
    const std::vector<boost::filesystem::path> &translation_unit_patterns,
    std::set<boost::filesystem::path> &glob_files_absolute)
{
    for (const auto &glob : translation_unit_patterns) {
        if (glob.string().size() < 2 || glob.string()[0] != '!')
            continue;

        const boost::filesystem::path absolute_glob_path{
            glob.string().substr(1)};
        auto matches = glob::glob(absolute_glob_path.string(), true, false);

        for (const auto &match : matches) {
            const auto path = boost::filesystem::weakly_canonical(match);

            assert(path.is_absolute());

            LOG(trace) << "Removing match " << path.string()
                       << " based on glob " << glob.string();

            glob_files_absolute.erase(path);
        }
    }
}

void resolve_whitelist_glob_patterns(
    const std::vector<boost::filesystem::path> &translation_unit_patterns,
    std::set<boost::filesystem::path> &glob_files_absolute)
{
    for (const auto &glob : translation_unit_patterns) {
        if (glob.string().empty() || glob.string()[0] == '!')
            continue;

        boost::filesystem::path absolute_glob_path{glob.string()};

        if (!absolute_glob_path.is_absolute())
            absolute_glob_path =
                boost::filesystem::current_path() / absolute_glob_path;

        LOG(debug) << "Searching glob path " << absolute_glob_path.string()
                   << " [" << glob.string() << "]";

        auto matches = glob::glob(absolute_glob_path.string(), true, false);

        LOG(debug) << "Found " << matches.size()
                   << " files matching glob: " << absolute_glob_path.string();

        for (const auto &match : matches) {
            const auto path = boost::filesystem::weakly_canonical(match);

            assert(path.is_absolute());

            glob_files_absolute.emplace(path);
        }
    }
}

boost::filesystem::path get_canonical_file(CXCompileCommand command)
{
    boost::filesystem::path file{
        clang_getCString(clang_CompileCommand_getFilename(command))};

    if (file.is_absolute())
        return file;

    const boost::filesystem::path directory{
        clang_getCString(clang_CompileCommand_getDirectory(command))};

    return weakly_canonical(directory / file);
}
} // namespace clang_include_graph