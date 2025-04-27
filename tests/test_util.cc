/**
 * tests/test_cycles_printer.cc
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

#define BOOST_TEST_MODULE Unit test of utility functions

#include <boost/test/unit_test.hpp>

#include "../src/include_graph_cycles_printer.h"

using namespace clang_include_graph::util;

BOOST_AUTO_TEST_CASE(test_glob_to_regex)
{
    BOOST_TEST(match_flag_glob("-Wall", "-Wall"));
    BOOST_TEST(match_flag_glob("-Wall", "-W*"));
    BOOST_TEST(match_flag_glob("-Wall", "-Wa*"));
    BOOST_TEST(!match_flag_glob("-Wall", "-Wno-*"));
}