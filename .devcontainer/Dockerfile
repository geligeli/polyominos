FROM silkeh/clang:latest

RUN apt-get update
RUN apt install gdb sudo -y
RUN apt install libtbb-dev -y
RUN apt install git -y
RUN apt install clangd -y
RUN apt install linux-perf -y
RUN apt install libbenchmark-dev -y


ARG uid=1000
ARG user=geli
ARG gid=1000
ARG group=geli

RUN groupadd -g ${gid} geli
RUN useradd -ms /bin/bash geli -u ${uid} -g ${gid}
RUN adduser geli sudo
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

RUN wget -O /usr/local/bin/bazel https://github.com/bazelbuild/bazelisk/releases/download/v1.25.0/bazelisk-linux-amd64
RUN chown root:root /usr/local/bin/bazel
RUN chmod 755 /usr/local/bin/bazel

RUN wget -O /usr/local/bin/buildifier https://github.com/bazelbuild/buildtools/releases/download/v7.3.1/buildifier-linux-amd64
RUN chown root:root /usr/local/bin/buildifier
RUN chmod 755 /usr/local/bin/buildifier

RUN wget -O /usr/local/bin/ibazel https://github.com/bazelbuild/bazel-watcher/releases/download/v0.25.3/ibazel_linux_amd64
RUN chown root:root /usr/local/bin/ibazel
RUN chmod 755 /usr/local/bin/ibazel

RUN rm -rf /var/lib/apt/lists/*
USER geli
ENV CC=clang
ENV CXX=clang++

