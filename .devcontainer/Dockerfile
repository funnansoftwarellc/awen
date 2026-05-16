# Default to linux/amd64 — Android NDK host binaries are published for Linux x86_64.
# Override at build time with: --build-arg DEVCONTAINER_PLATFORM=linux/arm64
ARG DEVCONTAINER_PLATFORM=linux/amd64
FROM --platform=${DEVCONTAINER_PLATFORM} ubuntu:26.04

ENV DEBIAN_FRONTEND=noninteractive

# Base utilities and build tooling.
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    curl \
    git \
    make \
    cmake \
    ninja-build \
    pkg-config \
    python3 \
    zip \
    unzip \
    xz-utils \
    tar \
    && rm -rf /var/lib/apt/lists/*

# GCC 16.
RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc-16 \
    g++-16 \
    && rm -rf /var/lib/apt/lists/* \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-16 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-16 100

# Add the official LLVM apt repository so clang-22 packages are available.
RUN curl -fsSL https://apt.llvm.org/llvm-snapshot.gpg.key \
    -o /usr/share/keyrings/llvm-apt-key.asc \
    && . /etc/os-release \
    && echo "deb [signed-by=/usr/share/keyrings/llvm-apt-key.asc] http://apt.llvm.org/questing/ llvm-toolchain-questing-22 main" \
    > /etc/apt/sources.list.d/llvm-22.list

# Clang 22.
RUN apt-get update && apt-get install -y --no-install-recommends \
    clang-22 \
    clang-format-22 \
    clang-tidy-22 \
    clangd-22 \
    lld-22 \
    llvm-22 \
    && rm -rf /var/lib/apt/lists/* \
    && update-alternatives --install /usr/bin/clang clang /usr/bin/clang-22 100 \
    && update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-22 100 \
    && update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-22 100 \
    && update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-22 100 \
    && update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-22 100 \
    && update-alternatives --install /usr/bin/lld lld /usr/bin/lld-22 100 \
    && update-alternatives --install /usr/bin/llvm-cov llvm-cov /usr/bin/llvm-cov-22 100 \
    && update-alternatives --install /usr/bin/llvm-profdata llvm-profdata /usr/bin/llvm-profdata-22 100 \
    && update-alternatives --install /usr/bin/run-clang-tidy run-clang-tidy /usr/bin/run-clang-tidy-22 100 \
    && update-alternatives --install /usr/bin/clang-tidy-diff clang-tidy-diff /usr/bin/clang-tidy-diff-22.py 100

# Emscripten SDK for wasm toolchains.
ARG EMSDK_VERSION=latest
RUN git clone https://github.com/emscripten-core/emsdk.git /opt/emsdk \
    && /opt/emsdk/emsdk install ${EMSDK_VERSION} \
    && /opt/emsdk/emsdk activate ${EMSDK_VERSION} \
    && nodeBin="$(find /opt/emsdk/node -type f -path '*/bin/node' | head -n 1)" \
    && test -n "${nodeBin}" \
    && ln -sf "${nodeBin}" /usr/local/bin/node

ENV EMSDK=/opt/emsdk
ENV EMSCRIPTEN_ROOT=/opt/emsdk/upstream/emscripten
ENV PATH=${EMSDK}:${EMSCRIPTEN_ROOT}:${PATH}
