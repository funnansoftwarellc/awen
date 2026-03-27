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

# Add the official LLVM apt repository so that clang-22 packages are available.
RUN curl -fsSL https://apt.llvm.org/llvm-snapshot.gpg.key \
        -o /usr/share/keyrings/llvm-apt-key.asc \
    && . /etc/os-release \
    && echo "deb [signed-by=/usr/share/keyrings/llvm-apt-key.asc] http://apt.llvm.org/${UBUNTU_CODENAME}/ llvm-toolchain-${UBUNTU_CODENAME}-22 main" \
       > /etc/apt/sources.list.d/llvm-22.list

# Clang 22
RUN apt-get update && apt-get install -y --no-install-recommends \
    clang-22 \
    clang-format-22 \
    clang-tidy-22 \
    clangd-22 \
    lld-22 \
    && rm -rf /var/lib/apt/lists/* \
    && update-alternatives --install /usr/bin/clang clang /usr/bin/clang-22 100 \
    && update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-22 100 \
    && update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-22 100 \
    && update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-22 100 \
    && update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-22 100 \
    && update-alternatives --install /usr/bin/lld lld /usr/bin/lld-22 100 \
    && update-alternatives --install /usr/bin/run-clang-tidy run-clang-tidy /usr/bin/run-clang-tidy-22 100 \
    && update-alternatives --install /usr/bin/clang-tidy-diff clang-tidy-diff /usr/bin/clang-tidy-diff-22.py 100

# X11 / display libraries (required by raylib and similar)
RUN apt-get update && apt-get install -y --no-install-recommends \
    libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config \
    libgl1-mesa-dev libx11-dev libxrandr-dev libwayland-dev libxkbcommon-dev \
    libdecor-0-dev \
    && rm -rf /var/lib/apt/lists/*
