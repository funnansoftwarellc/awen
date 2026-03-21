# Default to linux/amd64 — the Android NDK only ships x86_64 Linux host binaries.
# Override at build time with: --build-arg DEVCONTAINER_PLATFORM=linux/arm64
ARG DEVCONTAINER_PLATFORM=linux/amd64
FROM --platform=${DEVCONTAINER_PLATFORM} ubuntu:25.10

ENV DEBIAN_FRONTEND=noninteractive

# Build essentials and vcpkg dependencies
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
    clang \
    && rm -rf /var/lib/apt/lists/*

# Android NDK (direct download — no sdkmanager/Java required)
ARG NDK_VERSION=r29
ENV ANDROID_NDK_HOME="/opt/android/ndk/android-ndk-${NDK_VERSION}"

RUN mkdir -p /opt/android/ndk \
    && curl -fsSL "https://dl.google.com/android/repository/android-ndk-${NDK_VERSION}-linux.zip" \
        -o /tmp/ndk.zip \
    && unzip -q /tmp/ndk.zip -d /opt/android/ndk \
    && rm /tmp/ndk.zip

ENV PATH="${ANDROID_NDK_HOME}:${PATH}"
