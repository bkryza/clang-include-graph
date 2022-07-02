/**
 * src/include_graph_tree_printer.h
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

#ifndef CLANG_INCLUDE_GRAPH_UTIL_H
#define CLANG_INCLUDE_GRAPH_UTIL_H

#include <filesystem>
#include <optional>
#include <string>

namespace clang_include_graph {
namespace util {

std::optional<std::string> to_absolute_path(std::string_view relative_path)
{
    try {
        return std::filesystem::canonical(std::filesystem::path(relative_path))
            .string();
    }
    catch (const std::filesystem::filesystem_error &e) {
        return {};
    }
}

std::string relative_to(
    std::string_view path, std::optional<std::string> directory)
{
    if (!directory)
        return std::string(path);

    return relative(
        std::filesystem::path(path), std::filesystem::path(directory.value()));
}

} // namespace util
} // namespace clang_include_graph

#endif // CLANG_INCLUDE_GRAPH_UTIL_H
