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

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

namespace clang_include_graph {

class path_printer_t {
public:
    path_printer_t() = default;
    virtual ~path_printer_t() = default;

    virtual std::string print(const boost::filesystem::path &p) const
    {
        return p.string();
    }

    static std::unique_ptr<path_printer_t> from_config(const config_t &config);
};

class path_relative_printer_t final : public path_printer_t {
public:
    path_relative_printer_t(const std::string &relative_to)
        : relative_to_{relative_to}
    {
    }

    virtual std::string print(const boost::filesystem::path &p) const override
    {
        // Only return relative path, if path is in relative_to_ directory
        if (boost::starts_with(p.string(), relative_to_.string()))
            return boost::filesystem::relative(p, relative_to_).string();

        return p.string();
    }

private:
    boost::filesystem::path relative_to_;
};

class path_name_printer_t final : public path_printer_t {
public:
    std::string print(const boost::filesystem::path &p) const override
    {
        return p.filename().string();
    }
};

} // namespace clang_include_graph

#endif // CLANG_INCLUDE_GRAPH_PATH_PRINTER_H
