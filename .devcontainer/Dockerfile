FROM ubuntu:22.04

SHELL ["/bin/bash", "-c"]
ENV DEBIAN_FRONTEND=noninteractive

# Set up kitware apt repository
RUN apt-get update && apt-get install -y \
   ca-certificates \
   gnupg2 \
   wget
RUN rm -f /usr/share/keyrings/kitware-archive-keyring.gpg
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc | gpg --dearmor -o /usr/share/keyrings/kitware-archive-keyring.gpg
RUN echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main" | tee /etc/apt/sources.list.d/kitware.list > /dev/null

RUN apt-get update && apt-get install -y \
   kitware-archive-keyring \
   build-essential \
   clang-format \
   clang-tidy \
   cmake \
   cppcheck \
   curl \
   git \
   protobuf-compiler \
   python3 \
   python3-pip \
   libboost-all-dev \
   libfmt-dev \
   libgtest-dev \
   libpoco-dev \
   libprotobuf-dev \
   llvm \
   nlohmann-json3-dev \
   sudo \
   vim

#RUN wget https://github.com/jwdinius/simpleio/releases/download/v0.1.0/simpleio-dev_0.1.0_arm64.deb \
#COPY simpleio-dev_0.1.0_arm64.deb .
#RUN apt install -y ./simpleio-dev_0.1.0_arm64.deb \
#   && rm simpleio-dev_0.1.0_arm64.deb

RUN python3 -m pip install pre-commit cpplint

# setup user env at the end
# -m option creates a fake writable home folder
RUN adduser --disabled-password --gecos '' taktile
RUN adduser taktile sudo
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

USER taktile
WORKDIR /home/taktile

# Install pytak from source
COPY submodules /tmp/submodules

USER root
RUN chown -R taktile:taktile /tmp/submodules

USER taktile
RUN python3 -m pip install /tmp/submodules/pytak
RUN python3 -m pip install -r /tmp/submodules/pytak/requirements_test.txt

RUN echo "export PATH=$HOME/.local/bin:$PATH" >> /home/taktile/.bashrc
