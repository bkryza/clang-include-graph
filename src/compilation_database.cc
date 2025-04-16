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
        const boost::filesystem::path directory =
            clang_getCString(clang_CompileCommand_getDirectory(command));
        const boost::filesystem::path file =
            clang_getCString(clang_CompileCommand_getFilename(command));

        LOG(trace) << "Found file " << file.string()
                   << " in compilation database\n";

        if (!file.is_absolute())
            result.emplace(directory / file);
        else
            result.emplace(file);
    }

    return result;
}

} // namespace clang_include_graph