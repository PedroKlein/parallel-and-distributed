FROM mcr.microsoft.com/devcontainers/cpp:1-debian-11

ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="3.22.2"

COPY ./reinstall-cmake.sh /tmp/

RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
        chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/reinstall-cmake.sh

RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends binutils \
    && apt-get clean && rm -rf /var/lib/apt/lists/*
