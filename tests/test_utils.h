/**
* tests/test_utils.h
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

#ifndef CLANG_INCLUDE_GRAPH_TEST_UTILS_H
#define CLANG_INCLUDE_GRAPH_TEST_UTILS_H

#include <string>
#include <vector>
#include <istream>

namespace clang_include_graph {

void read_lines(std::istream &s, std::vector<std::string> &o)
{
    while (!s.fail()) {
        std::string l;
        std::getline(s, l);
        if (!l.empty())
            o.push_back(l);
    }
}

}
#endif // CLANG_INCLUDE_GRAPH_TEST_UTILS_H
