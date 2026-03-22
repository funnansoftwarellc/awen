FROM ubuntu:25.10

ENV DEBIAN_FRONTEND=noninteractive

# Base utilities
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    curl \
    gnupg \
    lsb-release \
    software-properties-common \
    git \
    cmake \
    ninja-build \
    pkg-config \
    python3 \
    zip \
    unzip \
    tar \
    && rm -rf /var/lib/apt/lists/*

# Latest GCC from default Ubuntu repos (ubuntu-toolchain-r PPA does not support 25.10+)
RUN apt-get update \
    && GCC_VER=$(apt-cache search --names-only '^gcc-[0-9]+$' \
    | grep -oP '(?<=gcc-)\d+' | sort -n | tail -1) \
    && apt-get install -y --no-install-recommends gcc-${GCC_VER} g++-${GCC_VER} \
    && update-alternatives \
    --install /usr/bin/gcc  gcc  /usr/bin/gcc-${GCC_VER}  100 \
    --slave   /usr/bin/g++  g++  /usr/bin/g++-${GCC_VER}      \
    --slave   /usr/bin/gcov gcov /usr/bin/gcov-${GCC_VER}     \
    && rm -rf /var/lib/apt/lists/*

# Latest Clang via LLVM apt repo
RUN curl -fsSL https://apt.llvm.org/llvm-snapshot.gpg.key \
    | gpg --dearmor -o /usr/share/keyrings/llvm-archive-keyring.gpg \
    && echo "deb [signed-by=/usr/share/keyrings/llvm-archive-keyring.gpg] \
    https://apt.llvm.org/$(lsb_release -cs)/ llvm-toolchain-$(lsb_release -cs) main" \
    > /etc/apt/sources.list.d/llvm.list \
    && apt-get update \
    && CLANG_VER=$(apt-cache search --names-only '^clang-[0-9]+$' \
    | grep -oP '(?<=clang-)\d+' | sort -n | tail -1) \
    && apt-get install -y --no-install-recommends \
    clang-${CLANG_VER} \
    clang-format-${CLANG_VER} \
    clang-tidy-${CLANG_VER} \
    clangd-${CLANG_VER} \
    lld-${CLANG_VER} \
    lldb-${CLANG_VER} \
    llvm-${CLANG_VER} \
    && update-alternatives \
    --install /usr/bin/clang            clang            /usr/bin/clang-${CLANG_VER}               100 \
    --slave   /usr/bin/clang++          clang++          /usr/bin/clang++-${CLANG_VER}                 \
    --slave   /usr/bin/clang-format     clang-format     /usr/bin/clang-format-${CLANG_VER}            \
    --slave   /usr/bin/clang-tidy       clang-tidy       /usr/bin/clang-tidy-${CLANG_VER}              \
    --slave   /usr/bin/run-clang-tidy   run-clang-tidy   /usr/bin/run-clang-tidy-${CLANG_VER}          \
    --slave   /usr/bin/clang-tidy-diff  clang-tidy-diff  /usr/bin/clang-tidy-diff-${CLANG_VER}.py      \
    --slave   /usr/bin/clangd           clangd           /usr/bin/clangd-${CLANG_VER}                  \
    --slave   /usr/bin/lld              lld              /usr/bin/lld-${CLANG_VER}                     \
    --slave   /usr/bin/lldb             lldb             /usr/bin/lldb-${CLANG_VER}                    \
    --slave   /usr/bin/llvm-cov          llvm-cov        /usr/bin/llvm-cov-${CLANG_VER}                \
    --slave   /usr/bin/llvm-profdata     llvm-profdata   /usr/bin/llvm-profdata-${CLANG_VER}           \
    && rm -rf /var/lib/apt/lists/*

# X11 / display libraries (required by raylib and similar)
RUN apt-get update && apt-get install -y --no-install-recommends \
    libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config \
    libgl1-mesa-dev libx11-dev libxrandr-dev \
    autoconf autoconf-archive automake libtool python3-venv make \
    libwayland-dev libxkbcommon-dev libegl1-mesa-dev libibus-1.0-dev \
    && rm -rf /var/lib/apt/lists/*
