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

#ifndef CLANG_INCLUDE_GRAPH_PATH_PRINTER_H
#define CLANG_INCLUDE_GRAPH_PATH_PRINTER_H

#include "config.h"
#include "include_graph.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

namespace clang_include_graph {

class path_printer_t {
public:
    path_printer_t() = default;

    explicit path_printer_t(config_t config)
        : config_{config}
    {
    }

    virtual ~path_printer_t() = default;
    path_printer_t(const path_printer_t &) = default;
    path_printer_t(path_printer_t &&) = default;
    path_printer_t &operator=(const path_printer_t &) = default;
    path_printer_t &operator=(path_printer_t &&) = default;

    virtual std::string print(const include_graph_t::vertex_t &v) const
    {
        return v.file;
    }

    static std::unique_ptr<path_printer_t> from_config(const config_t &config);

protected:
    config_t config_;
};

class path_relative_printer_t final : public path_printer_t {
public:
    explicit path_relative_printer_t(config_t config)
        : path_printer_t(config)
        , relative_to_{std::move(*config.relative_to())}
    {
    }

    std::string print(const include_graph_t::vertex_t &v) const override
    {
        // Only return relative path, if path is in relative_to_ directory
        if (boost::starts_with(v.file, relative_to_.string())) {
            return boost::filesystem::relative(v.file, relative_to_).string();
        }

        return v.file;
    }

private:
    boost::filesystem::path relative_to_;
};

class path_name_printer_t final : public path_printer_t {
public:
    using path_printer_t::path_printer_t;

    std::string print(const include_graph_t::vertex_t &v) const override
    {
        return boost::filesystem::path(v.file).filename().string();
    }
};

} // namespace clang_include_graph

#endif // CLANG_INCLUDE_GRAPH_PATH_PRINTER_H
