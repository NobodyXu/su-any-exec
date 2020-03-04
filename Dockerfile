FROM nobodyxu/apt-fast:latest-debian-buster-slim AS apt-fast

FROM nobodyxu/musl-libc:latest-debian-buster-slim AS base
COPY --from=apt-fast /usr/local/ /usr/local/

# Prepare Environment
## Install dependencies
RUN apt-auto install -y --no-install-recommends \
                     clang lld llvm make python3

## Configure llvm as default toolchain
### Use ld.ldd as default linker
RUN ln -f $(which ld.lld) /usr/bin/ld

## For testing su-exec-*
RUN useradd -m test1 && useradd -m test2
RUN groupadd group1 && addgroup test1 group1

RUN /usr/local/sbin/rm_apt-fast.sh

## Build su-exec*
ADD * /usr/local/src/su-exec/
WORKDIR /usr/local/src/su-exec/

# Build only dynamic-version for glibc as static version is buggy
RUN CC=clang      make su-exec -j $(nproc) && \
    mv su-exec /usr/local/sbin/

RUN CC=musl-clang make su-exec su-exec-static -j $(nproc) && \
    mv su-exec        /usr/local/sbin/su-exec-musl && \
    mv su-exec-static /usr/local/sbin/su-exec-musl-static

# Test
RUN ./test_exe.py /usr/local/sbin/*
