FROM silkeh/clang:latest

ARG uid=1000
ARG user=geli
ARG gid=1000
ARG group=geli

RUN apt-get update -y
RUN apt-get upgrade -y

RUN apt install libtbb-dev -y
RUN apt install git -y
RUN apt install clangd -y
RUN apt install linux-perf -y
RUN apt install sudo -y

RUN wget -O /usr/local/bin/bazel https://github.com/bazelbuild/bazelisk/releases/download/v1.25.0/bazelisk-linux-amd64
RUN wget -O /usr/local/bin/buildifier https://github.com/bazelbuild/buildtools/releases/download/v7.3.1/buildifier-linux-amd64

RUN wget https://ftp.gnu.org/gnu/gdb/gdb-15.2.tar.xz && \
    tar -xf gdb-15.2.tar.xz && rm gdb-15.2.tar.xz
RUN apt install texinfo libreadline-dev libmpc-dev python-dev-is-python3 -y


RUN cd gdb-15.2 && \
    mkdir build && \
    cd build && \
     ../configure --prefix=/usr --with-system-readline --with-python=/usr/bin/python3.11 && \
     make -j 32 && \
     make -C gdb install && \
    cd ../.. && \
    rm -rf gdb-15.2
RUN rm -rf /var/lib/apt/lists/*

RUN chown root:root /usr/local/bin/bazel
RUN chmod 755 /usr/local/bin/bazel
RUN chown root:root /usr/local/bin/buildifier
RUN chmod 755 /usr/local/bin/buildifier

RUN groupadd -g ${gid} ${user}
RUN useradd -ms /bin/bash ${user} -u ${uid} -g ${gid}
RUN adduser ${user} sudo
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

USER ${user}
WORKDIR /home/${user}
RUN echo "set disassembly-flavor intel" > ~/.gdbinit
ENV CC=clang
ENV CXX=clang++

