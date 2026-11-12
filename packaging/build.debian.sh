#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$SCRIPT_DIR/.."

BUILDER_TAG=clang-include-graph-debian-builder
PROJECT_DIR=/build
docker build --tag $BUILDER_TAG packaging/debian/builder
docker run --rm --memory 2GB --init -v $PWD:$PROJECT_DIR --workdir $PROJECT_DIR/packaging -it $BUILDER_TAG bash -c "git config --global --add safe.directory $PROJECT_DIR && make OS=debian DIST=trixie \"\$@\" debian" build.debian.sh "$@"
