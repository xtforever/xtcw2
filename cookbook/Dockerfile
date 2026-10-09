# syntax=docker/dockerfile:1
#
# cookbook reference app — build xtcw2 + the app in debug and production mode.
#
# The build context MUST be the repository root (the whole toolkit is required):
#
#   docker build -f cookbook/Dockerfile --target debug      -t xtcw-cookbook:debug      .
#   docker build -f cookbook/Dockerfile --target production -t xtcw-cookbook:production .
#
# The LuaRunner/nanosvg git submodule must be checked out on the host first:
#
#   git submodule update --init --recursive
#
# `cookbook/Dockerfile.dockerignore` keeps generated objects/archives and the
# .git/experimental directories out of the context.

FROM ubuntu:24.04 AS toolchain
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential gcc make pkg-config bison flex swig util-linux \
        libx11-dev libxt-dev libxaw7-dev libxmu-dev libxft-dev \
        libfontconfig1-dev libxrender-dev libxpm-dev libxext-dev \
        libcairo2-dev liblua5.3-dev lua5.3 lua-lpeg \
        xvfb x11-utils fonts-dejavu-core ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# Fail early with a clear message if the nanosvg submodule was not initialised:
# otherwise COPY would silently bring in an empty directory.
RUN test -f LuaRunner/nanosvg/src/nanosvg.h \
    || (echo "ERROR: LuaRunner/nanosvg is empty. Run 'git submodule update --init --recursive' before 'docker build'." >&2; exit 1)

# Debug: compile + start smoke test. AddressSanitizer is used here; if the Docker
# build's seccomp profile blocks ASan's address-space setup, build this target
# with a relaxed profile or run the debug smoke at container runtime instead
# (see cookbook/docker-compose.yml, profile "debug").
FROM toolchain AS debug
RUN cookbook/scripts/build.sh debug \
 && cookbook/scripts/smoke-test.sh debug

# Production: -O3, no MLS_DEBUG, no AddressSanitizer. This is the deployable
# artifact and its build-time smoke test has no sanitizer constraints.
FROM toolchain AS production
RUN cookbook/scripts/build.sh production \
 && cookbook/scripts/smoke-test.sh production
