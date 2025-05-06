FROM ubuntu:24.10

ADD . /logos_app/app

WORKDIR /logos_app/app

RUN apt-get update &&\
    apt-get install -y qt6-base-dev protobuf-compiler patchelf  git protobuf-compiler which cargo mawk cmake build-essential lsb-release jq curl

RUN git submodule update --init --recursive
