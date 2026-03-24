# Default to linux/amd64 — the Android NDK only ships x86_64 Linux host binaries.
# Override at build time with: --build-arg DEVCONTAINER_PLATFORM=linux/arm64
ARG DEVCONTAINER_PLATFORM=linux/amd64
FROM --platform=${DEVCONTAINER_PLATFORM} ubuntu:25.10

ENV DEBIAN_FRONTEND=noninteractive

# Build essentials and vcpkg dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates curl git cmake make ninja-build pkg-config python3 zip unzip tar clang openjdk-17-jdk \
    # emulator runtime dependencies
    libpulse0 libgl1 libnss3 libxcomposite1 libxcursor1 libxi6 libxtst6 \
    # adb over USB
    usbutils \
    && rm -rf /var/lib/apt/lists/*

ENV JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64

# Android SDK
RUN mkdir -p /opt/android/sdk && \
    curl -O https://dl.google.com/android/repository/commandlinetools-linux-14742923_latest.zip && \
    unzip -qq commandlinetools-linux-14742923_latest.zip -d /opt/android/sdk && \
    rm -f commandlinetools-linux-14742923_latest.zip

ENV ANDROID_HOME=/opt/android/sdk

# Install Build Tools
RUN mkdir -p /opt/android/sdk/cmdline-tools/latest && \
    mv /opt/android/sdk/cmdline-tools/bin /opt/android/sdk/cmdline-tools/latest/ && \
    mv /opt/android/sdk/cmdline-tools/lib /opt/android/sdk/cmdline-tools/latest/ && \
    mv /opt/android/sdk/cmdline-tools/NOTICE.txt /opt/android/sdk/cmdline-tools/latest/ && \
    mv /opt/android/sdk/cmdline-tools/source.properties /opt/android/sdk/cmdline-tools/latest/ && \
    yes | /opt/android/sdk/cmdline-tools/latest/bin/sdkmanager --licenses && \
    /opt/android/sdk/cmdline-tools/latest/bin/sdkmanager \
    "platforms;android-36" \
    "build-tools;36.0.0" \
    "platform-tools" \
    "emulator" \
    "system-images;android-36.1;google_apis;x86_64"

# Create a default AVD so the emulator is ready to launch immediately
# -n  : AVD name used with `emulator -avd awen_avd`
# --device : hardware profile (Pixel 6)
RUN $ANDROID_HOME/cmdline-tools/latest/bin/avdmanager create avd \
    -n awen_avd \
    -k "system-images;android-36.1;google_apis;x86_64" \
    --device "pixel_6" \
    --force

# Android NDK (direct download — no sdkmanager/Java required)
ARG NDK_VERSION=r29
RUN mkdir -p /opt/android/ndk/android-ndk-${NDK_VERSION} \
    && curl -fsSL "https://dl.google.com/android/repository/android-ndk-${NDK_VERSION}-linux.zip" \
    -o /tmp/ndk.zip \
    && unzip -q /tmp/ndk.zip -d /opt/android/ndk \
    && rm /tmp/ndk.zip

ENV ANDROID_NDK_HOME="/opt/android/ndk/android-ndk-${NDK_VERSION}"

ENV PATH="${ANDROID_HOME}/emulator:${ANDROID_HOME}/platform-tools:${ANDROID_NDK_HOME}:${PATH}"

# Debug keystore — baked into the image so no manual setup step is needed
RUN mkdir -p /root/.android && \
    keytool -genkeypair \
    -v \
    -keystore /root/.android/debug.keystore \
    -storepass android \
    -alias androiddebugkey \
    -keypass android \
    -keyalg RSA \
    -keysize 2048 \
    -validity 10000 \
    -dname "CN=Android Debug, O=Android, C=US"