## Installation
### Distribution packages
#### Ubuntu
```bash
sudo add-apt-repository ppa:bkryza/clang-include-graph
sudo apt update
sudo apt install clang-include-graph
```

#### Debian

```bash
# Bookworm
wget https://github.com/bkryza/clang-include-graph/releases/download/0.2.0/clang-include-graph_0.2.0-1_amd64.deb
sudo apt install ./clang-include-graph_0.2.0-1_amd64.deb
```

#### Fedora

```bash
# Fedora 39
wget https://github.com/bkryza/clang-include-graph/releases/download/0.2.0/clang-include-graph-0.2.0-1.fc39.x86_64.rpm
sudo dnf install ./clang-include-graph-0.2.0-1.fc39.x86_64.rpm

# Fedora 40
wget https://github.com/bkryza/clang-include-graph/releases/download/0.2.0/clang-include-graph-0.2.0-1.fc40.x86_64.rpm
sudo dnf install ./clang-include-graph-0.2.0-1.fc40.x86_64.rpm

# Fedora 41
wget https://github.com/bkryza/clang-include-graph/releases/download/0.2.0/clang-include-graph-0.2.0-1.fc41.x86_64.rpm
sudo dnf install ./clang-include-graph-0.2.0-1.fc41.x86_64.rpm
```

#### Conda

```bash
conda config --add channels conda-forge
conda config --set channel_priority strict
conda install -c bkryza/label/clang-include-graph clang-include-graph
```

#### Windows

Download and run the latest Windows installer from
[Releases page](https://github.com/bkryza/clang-include-graph/releases).


### Building from source

#### Linux
First make sure that you have the following dependencies installed:

```bash
# Ubuntu (clang version will vary depending on Ubuntu version)
apt install git make gcc g++ cmake clang-19 libclang-19-dev libclang-cpp19-dev libboost-log1.83-dev libboost-graph1.83-dev libboost-filesystem1.83-dev libboost-test1.83-dev libboost-json1.83-dev libboost-program-options1.83-dev
```

Then proceed with building the sources:

```bash
git clone https://github.com/bkryza/clang-include-graph
cd clang-include-graph
# Please note that top level Makefile is just a convenience wrapper for CMake
make release
release/clang-include-graph --help

# To build using a specific installed version of LLVM use:
LLVM_CONFIG_PATH=/usr/bin/llvm-config-18 make release

export PATH=$PATH:$PWD/release
```

#### Windows
##### Visual Studio native build

These steps present how to build and use `clang-include-graph` natively using Microsoft Visual Studio only.

First, install the following dependencies manually:

* [Python 3](https://www.python.org/downloads/windows/)
* [Git](https://git-scm.com/download/win)
* [CMake](https://cmake.org/download/)
* [Visual Studio](https://visualstudio.microsoft.com/vs/community/)

> All the following steps should be invoked in `Developer PowerShell for VS`.

Create installation directory for `clang-uml` and its dependencies:

```bash
# This is where clang-uml binary and its dependencies will be installed after build
# If you change this path, adapt all consecutive steps
mkdir C:\clang-include-graph-llvm20
# This directory will be removed after build
mkdir C:\clang-include-graph-llvm20-tmp
cd C:\clang-include-graph-llvm20-tmp
```

Build and install `LLVM`:

```bash
pip install psutil
# Update the LLVM branch if necessary
git clone --branch llvmorg-20.1.5 --depth 1 https://github.com/llvm/llvm-project.git llvm
cmake -S .\llvm\llvm -B llvm-build -DLLVM_ENABLE_PROJECTS=clang -DCMAKE_INSTALL_PREFIX="C:\clang-include-graph-llvm20" -DLIBCLANG_BUILD_STATIC=ON -DLLVM_ENABLE_PIC=OFF -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=X86 -DCLANG_LINK_CLANG_DYLIB=OFF -Thost=x64
cd llvm-build
# Make sure you are running this in a developer console
msbuild .\INSTALL.vcxproj -maxcpucount /p:Configuration=Release
```

Build and install `Boost`:

```bash
# Download and unpack Boost, then
cd boost_1_88_0
cd .\tools\build\
.\bootstrap.bat
cd ..
cd ..
.\bootstrap.bat msvc
.\b2.exe toolset=msvc variant=release threading=multi link=static address-model=64 --with-atomic --with-container --with-filesystem --with-thread --with-graph --with-log --with-test --with-json --with-program_options --with-test --prefix="C:\clang-include-graph-llvm20-tmp" -j6 install
```

Build and install `clang-include-graph`:

```bash
git clone https://github.com/bkryza/clang-include-graph
cd clang-include-graph
# Edit build.ps1 and adjust $Prefix to C:\clang-include-graph-llvm20-tmp
.\build.ps1
```

Check if `clang-include-graph` works:

```bash
.\Release\Release\clang-include-graph --version
```

It should produce something like:
```bash
clang-include-graph 0.2.0
Copyright (C) 2022-present Bartek Kryza <bkryza@gmail.com>
Built with libclang: 20.1.5
```
