/**
 * src/path_printer.h
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

#include "path_printer.h"
#include "config.h"

namespace clang_include_graph {

path_printer_t path_printer_t::from_config(config_t config)
{
    if (config.relative_to.has_value()) {
        return path_relative_printer_t{config.relative_to.value()};
    }
    else if (config.filenames_only) {
        return path_name_printer_t{};
    }

    return path_printer_t{};
}

} // namespace clang_include_graph