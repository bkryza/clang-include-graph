# Building releases

* Update CHANGELOG.md
* Tag the release commit, e.g. ```git tag 0.1.0```

## Ubuntu

```bash
cd packaging
make DIST=plucky deb
make DIST=focal deb
make DIST=jammy deb
make DIST=noble deb
make DIST=oracular deb

# Repeat for each distro
cd _BUILD/ubuntu/focal
dput ppa:bkryza/clang-include-graph *.changes

```

## Debian

```bash
docker run --rm -v $PWD:$PWD -it debian:bookworm bash
apt update
apt install debhelper python3 python3-pip git make ccache pkg-config gcc g++ gdb cmake libyaml-cpp-dev llvm-19 llvm-19-dev clang-19 clang-tools-19 libclang-19-dev libclang-cpp19-dev libmlir-19-dev mlir-19-tools libboost-graph1.81-dev libboost-filesystem1.81-dev libboost-test1.81-dev libboost-program-options1.81-dev libboost-json1.81-dev libboost-log1.81-dev libdw-dev libunwind-dev
pip3 install --break-system-packages git-archive-all
git config --global --add safe.directory /home/bartek/devel/clang-include-graph
cd packaging
make OS=debian DIST=bookworm debian
```

## Anaconda

```bash
docker run --rm -v $PWD:$PWD continuumio/miniconda3 bash
conda install conda-build make anaconda-client
cd packaging
git config --global --add safe.directory $PWD/..
make CONDA_TOKEN=<CONDA_TOKEN> conda
```