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

# GCC and Clang from Ubuntu repos
RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc \
    g++ \
    clang \
    clang-format \
    clang-tidy \
    clangd \
    lld \
    lldb \
    llvm \
    && rm -rf /var/lib/apt/lists/*

# X11 / display libraries (required by raylib and similar)
RUN apt-get update && apt-get install -y --no-install-recommends \
    libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config \
    libgl1-mesa-dev libx11-dev libxrandr-dev \
    && rm -rf /var/lib/apt/lists/*
