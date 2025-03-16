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

#include <boost/core/null_deleter.hpp>
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
        LOG(error) << "ERROR: Failed to resolve absolute path from '"
                   << relative_path << "': " << e.what() << 'n';
        throw e; // NOLINT
    }
}

void setup_logging(int log_level)
{
    const boost::shared_ptr<
        sinks::synchronous_sink<sinks::text_ostream_backend>>
        sink(new sinks::synchronous_sink<sinks::text_ostream_backend>);

    sink->locked_backend()->add_stream(
        boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));

    logging::core::get()->add_sink(sink);

    sink->set_formatter(expr::format("=== %1%") % expr::smessage);

    auto log_severity = boost::log::trivial::info;

    switch (log_level) {
    case 0:
        log_severity = boost::log::trivial::warning;
        break;
    case 1:
        log_severity = boost::log::trivial::info;
        break;
    case 2:
        log_severity = boost::log::trivial::debug;
        break;
    default:
        log_severity = boost::log::trivial::trace;
    }

    sink->set_filter(logging::trivial::severity >= log_severity);

    LOG(info) << "Setup console logging level to: " << to_string(log_severity)
              << '\n';
}

} // namespace util
} // namespace clang_include_graph
