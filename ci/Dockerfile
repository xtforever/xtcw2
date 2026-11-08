# Clean-build verification image for xtcw2.
#
# Build:
#   docker build -f ci/Dockerfile -t xtcw2-clean .
#
# Run (clones a fresh checkout of the committed state and builds):
#   docker run --rm \
#       -v /path/to/xtcw2-bare.git:/src/xtcw2.git:ro \
#       xtcw2-clean
#
# Override the source/ref:
#   docker run --rm -e REPO_URL=ssh://... -e REPO_REF=master xtcw2-clean
#
# Dependencies mirror .github/workflows/ci.yml.
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential gcc make pkg-config bison flex swig \
        git ca-certificates \
        libx11-dev libxt-dev libxaw7-dev libxmu-dev libxft-dev \
        libfontconfig1-dev libxrender-dev libxpm-dev libxext-dev \
        libcairo2-dev liblua5.3-dev lua5.3 lua-lpeg \
        xvfb x11-utils fontconfig fonts-dejavu-core fonts-noto-core \
    && rm -rf /var/lib/apt/lists/*

COPY ci/clean-build.sh /usr/local/bin/clean-build.sh
RUN chmod +x /usr/local/bin/clean-build.sh

WORKDIR /work
ENTRYPOINT ["/usr/local/bin/clean-build.sh"]
