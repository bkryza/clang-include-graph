#
# Makefile
#
# Copyright (c) 2022-present Bartek Kryza <bkryza@gmail.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# This Makefile is just a handy wrapper around cmake
#

.DEFAULT_GOAL := debug

NUMPROC ?= $(shell nproc)

LLVM_CONFIG_PATH ?=
CMAKE_CXX_FLAGS ?=
CMAKE_EXE_LINKER_FLAGS ?=

GIT_VERSION	?= $(shell git describe --tags --always --abbrev=7)
PKG_VERSION	?= $(shell git describe --tags --always --abbrev=7 | tr - .)
GIT_COMMIT ?= $(shell git rev-parse HEAD)
GIT_BRANCH ?= $(shell git rev-parse --abbrev-ref HEAD)

WITH_JSON ?= ON

.PHONY: clean
clean:
	rm -rf debug release

debug/CMakeLists.txt:
	cmake -S . -B debug \
		-DGIT_VERSION=$(GIT_VERSION) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_CXX_FLAGS="$(CMAKE_CXX_FLAGS)" \
		-DCMAKE_EXE_LINKER_FLAGS="$(CMAKE_EXE_LINKER_FLAGS)" \
		-DLLVM_CONFIG_PATH=$(LLVM_CONFIG_PATH) \
		-DWITH_JSON=$(WITH_JSON)

release/CMakeLists.txt:
	cmake -S . -B release \
		-DGIT_VERSION=$(GIT_VERSION) \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_CXX_FLAGS="$(CMAKE_CXX_FLAGS)" \
		-DCMAKE_EXE_LINKER_FLAGS="$(CMAKE_EXE_LINKER_FLAGS)" \
		-DLLVM_CONFIG_PATH=$(LLVM_CONFIG_PATH) \
		-DWITH_JSON=$(WITH_JSON)

debug: debug/CMakeLists.txt
	echo "Using ${NUMPROC} cores"
	make -C debug -j$(NUMPROC)

release: release/CMakeLists.txt
	echo "Using ${NUMPROC} cores"
	make -C release -j$(NUMPROC)

test: debug
	CTEST_OUTPUT_ON_FAILURE=1 make -C debug test

test_release: release
	CTEST_OUTPUT_ON_FAILURE=1 make -C release test

.PHONY: check-formatting
check-formatting:
	./util/check_formatting.sh

.PHONY: format
format:
	docker run --rm -v $(CURDIR):/root/sources bkryza/clang-format-check:1.5

.PHONY: tidy
tidy:
	cmake --build debug --target clang-tidy

.PHONY: fedora/%
fedora/%:
	mkdir -p packaging/_BUILD/fedora/$*
	git archive --format=tar.gz --prefix=clang-include-graph-$(PKG_VERSION)/ v$(GIT_VERSION) >packaging/_BUILD/fedora/$*/clang-include-graph-$(PKG_VERSION).tar.gz
	docker run --cpuset-cpus=0-7 -v $(PWD):$(PWD) fedora:$* sh -c "dnf install -y make git && cd ${PWD} && make OS=fedora DIST=$* VERSION=${PKG_VERSION} COMMIT=${GIT_COMMIT} BRANCH=${GIT_BRANCH} -C packaging rpm"

