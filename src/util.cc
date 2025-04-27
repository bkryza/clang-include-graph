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
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/exception_handler.hpp>

#include <fstream>
#include <iostream>

namespace clang_include_graph {
namespace util {

std::string join_cli_args(int argc, char *argv[])
{
    std::string result;
    for (auto i = 1U; i < argc; ++i) {
        if (i > 1)
            result += ' ';
        result += argv[i];
    }
    return result;
}

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

void setup_logging(
    int log_level, boost::optional<boost::filesystem::path> log_file)
{
    const boost::shared_ptr<
        sinks::synchronous_sink<sinks::text_ostream_backend>>
        sink(new sinks::synchronous_sink<sinks::text_ostream_backend>);

    boost::optional<
        boost::shared_ptr<sinks::synchronous_sink<sinks::text_ostream_backend>>>
        file_sink;

    sink->locked_backend()->add_stream(
        boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));

    sink->set_formatter(expr::format("=== %1%") % expr::smessage);

    logging::core::get()->add_sink(sink);

    if (log_file) {
        // Make sure the log directory exists
        try {
            boost::filesystem::create_directories(log_file->parent_path());
        }
        catch (const boost::filesystem::filesystem_error &e) {
            std::cerr << "ERROR: Cannot create log directory "
                      << log_file->parent_path() << ": '" << e.what() << "'"
                      << '\n';
            exit(-1);
        }

        // Try to create the log file if it doesn't exist yet
        try {
            if (!boost::filesystem::exists(*log_file)) {
                const std::ofstream ofs(log_file->string());
                if (!ofs) {
                    std::cerr << "ERROR: Cannot create log file "
                              << log_file->string() << '\n';
                    exit(-1);
                }
            }
        }
        catch (const boost::filesystem::filesystem_error &e) {
            std::cerr << "ERROR: Cannot create log file "
                      << log_file->parent_path() << ": '" << e.what() << "'"
                      << '\n';
            exit(-1);
        }

        file_sink = boost::shared_ptr<
            sinks::synchronous_sink<sinks::text_ostream_backend>>(
            new sinks::synchronous_sink<sinks::text_ostream_backend>);

        file_sink.value()->locked_backend()->add_stream(
            boost::shared_ptr<std::ostream>(
                new std::ofstream(log_file->string().c_str())));

        // Set up a formatter for all sinks in the core
        file_sink.value()->set_formatter(expr::format("[%1%] [%2%] - %3%") %
            expr::attr<boost::posix_time::ptime>("TimeStamp") %
            expr::attr<attrs::current_thread_id::value_type>("ThreadID") %
            expr::smessage);

        logging::core::get()->add_global_attribute(
            "TimeStamp", attrs::local_clock());
        logging::core::get()->add_global_attribute(
            "ThreadID", attrs::current_thread_id());

        logging::core::get()->add_sink(*file_sink);
    }

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

    LOG(info) << "Set console logging level to: " << to_string(log_severity)
              << '\n';
}

std::regex glob_to_regex(const std::string &glob_pattern)
{
    std::string regex_pattern;
    regex_pattern.reserve(glob_pattern.size() * 2);

    regex_pattern += '^'; // Match start of string

    for (const auto c : glob_pattern) {
        switch (c) {
        case '*':
            regex_pattern += ".*";
            break;
        case '?':
            regex_pattern += ".";
            break;
        case '.':
        case '+':
        case '(':
        case ')':
        case '{':
        case '}':
        case '[':
        case ']':
        case '^':
        case '$':
        case '|':
        case '\\':
            // Escape special regex characters
            regex_pattern += '\\';
            regex_pattern += c;
            break;
        default:
            regex_pattern += c;
        }
    }

    regex_pattern += '$';

    return std::regex(regex_pattern);
}

bool match_flag_glob(const std::string &flag, const std::string &glob)
{
    std::cmatch m;
    std::regex_match(flag.c_str(), m, util::glob_to_regex(glob));

    return m.size() == 1;
}

} // namespace util
} // namespace clang_include_graph
