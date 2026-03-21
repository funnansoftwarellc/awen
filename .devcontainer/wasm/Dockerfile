FROM emscripten/emsdk:latest

ENV DEBIAN_FRONTEND=noninteractive

# Additional tools needed by vcpkg and the build
RUN apt-get update && apt-get install -y --no-install-recommends \
    ninja-build \
    pkg-config \
    zip \
    unzip \
    tar \
    && rm -rf /var/lib/apt/lists/*
