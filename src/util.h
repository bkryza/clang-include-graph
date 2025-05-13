/**
 * src/util.h
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

#include <boost/filesystem.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/optional.hpp>

#include <regex>
#include <unordered_set>

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(global_logger,
    boost::log::sources::severity_logger_mt<
        boost::log::trivial::severity_level>)

// NOLINTNEXTLINE
#define LOG(lvl)                                                               \
    BOOST_LOG_SEV(                                                             \
        global_logger::get(), boost::log::trivial::severity_level::lvl)

namespace clang_include_graph {
namespace util {

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

std::string join_cli_args(int argc, char **argv);

boost::filesystem::path to_absolute_path(
    const boost::filesystem::path &relative_path);

void setup_logging(
    int log_level, boost::optional<boost::filesystem::path> log_file);

std::regex glob_to_regex(const std::string &glob_pattern);

bool match_flag_glob(const std::string &flag, const std::string &glob);
} // namespace util
} // namespace clang_include_graph

#endif // CLANG_INCLUDE_GRAPH_UTIL_H
