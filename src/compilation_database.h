/**
 * src/compilation_database.h
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

#ifndef CLANG_INCLUDE_GRAPH_COMPILATION_DATABASE_H
#define CLANG_INCLUDE_GRAPH_COMPILATION_DATABASE_H

#include "util.h"

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <clang-c/CXCompilationDatabase.h>

#include <set>

namespace clang_include_graph {

std::set<boost::filesystem::path> get_all_files(CXCompilationDatabase database);

void intersect_glob_matches_with_compilation_database(void *database,
    bool is_fixed,
    const std::set<boost::filesystem::path>
        &compilation_database_files_absolute,
    std::vector<CXCompileCommands> &matching_compile_commands,
    std::set<boost::filesystem::path> &glob_files_absolute);

void resolve_whitelist_glob_patterns(
    const std::vector<boost::filesystem::path> &translation_unit_patterns,
    std::set<boost::filesystem::path> &glob_files_absolute);

void filter_blacklist_glob_patterns(
    const std::vector<boost::filesystem::path> &translation_unit_patterns,
    std::set<boost::filesystem::path> &glob_files_absolute);

boost::filesystem::path get_canonical_file(CXCompileCommand command);

} // namespace clang_include_graph

#endif // CLANG_INCLUDE_GRAPH_COMPILATION_DATABASE_H