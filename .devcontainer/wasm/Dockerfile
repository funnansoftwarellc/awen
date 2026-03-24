FROM emscripten/emsdk:4.0.7

ENV DEBIAN_FRONTEND=noninteractive

# Additional tools needed by vcpkg and the build
RUN apt-get update && apt-get install -y --no-install-recommends \
    ninja-build \
    pkg-config \
    zip \
    unzip \
    tar \
    ca-certificates \
    gpg \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Install CMake 3.28+ via Kitware APT repository
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null \
    | gpg --dearmor - \
    | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null \
    && . /etc/os-release \
    && echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ $UBUNTU_CODENAME main" \
    | tee /etc/apt/sources.list.d/kitware.list >/dev/null \
    && apt-get update \
    && apt-get install -y --no-install-recommends cmake \
    && rm -rf /var/lib/apt/lists/*
