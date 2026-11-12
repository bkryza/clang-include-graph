FROM debian:trixie

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    debhelper \
    python3 \
    python3-pip \
    git \
    make \
    ccache \
    pkg-config \
    gcc \
    g++ \
    gdb \
    cmake \
    libyaml-cpp-dev \
    llvm-19 \
    llvm-19-dev \
    clang-19 \
    clang-tools-19 \
    libclang-19-dev \
    libclang-cpp19-dev \
    libmlir-19-dev \
    mlir-19-tools \
    libboost-graph1.83-dev \
    libboost-filesystem1.83-dev \
    libboost-test1.83-dev \
    libboost-program-options1.83-dev \
    libboost-json1.83-dev \
    libboost-log1.83-dev \
    libdw-dev \
    libunwind-dev \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install --break-system-packages git-archive-all
