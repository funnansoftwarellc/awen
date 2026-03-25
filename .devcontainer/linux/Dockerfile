FROM ubuntu:25.10

ENV DEBIAN_FRONTEND=noninteractive

# Base utilities
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    curl \
    git \
    cmake \
    ninja-build \
    pkg-config \
    python3 \
    zip \
    unzip \
    tar \
    && rm -rf /var/lib/apt/lists/*

# GCC 15
RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc-15 \
    g++-15 \
    && rm -rf /var/lib/apt/lists/* \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-15 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-15 100

# Clang 22 via LLVM APT repository
RUN curl -fsSL https://apt.llvm.org/llvm.sh -o /tmp/llvm.sh \
    && chmod +x /tmp/llvm.sh \
    && /tmp/llvm.sh 22 all \
    && rm /tmp/llvm.sh \
    && update-alternatives --install /usr/bin/clang clang /usr/bin/clang-22 100 \
    && update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-22 100 \
    && update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-22 100 \
    && update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-22 100 \
    && update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-22 100 \
    && update-alternatives --install /usr/bin/lld lld /usr/bin/lld-22 100 \
    && update-alternatives --install /usr/bin/lldb lldb /usr/bin/lldb-22 100 \
    && update-alternatives --install /usr/bin/run-clang-tidy run-clang-tidy /usr/bin/run-clang-tidy-22 100 \
    && update-alternatives --install /usr/bin/clang-tidy-diff clang-tidy-diff /usr/bin/clang-tidy-diff-22.py 100

# X11 / display libraries (required by raylib and similar)
RUN apt-get update && apt-get install -y --no-install-recommends \
    libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config \
    libgl1-mesa-dev libx11-dev libxrandr-dev \
    && rm -rf /var/lib/apt/lists/*
