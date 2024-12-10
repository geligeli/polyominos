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

RUN rm -rf /var/lib/apt/lists/*
USER geli

