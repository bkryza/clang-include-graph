/**
 * src/util.cc
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

#include "util.h"

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>

#include <iostream>

namespace clang_include_graph {
namespace util {

boost::filesystem::path to_absolute_path(
    const boost::filesystem::path &relative_path)
{
    try {
        return boost::filesystem::canonical(relative_path).string();
    }
    catch (const boost::filesystem::filesystem_error &e) {
        std::cerr << "ERROR: Failed to resolve absolute path from '"
                  << relative_path << "': " << e.what() << 'n';
        throw e; // NOLINT
    }
}

} // namespace util
} // namespace clang_include_graph
