FROM ubuntu:20.04

# build; docker build -t openface .

# USER and GROUP_ID may not match sometimes!
# run container: docker run --rm -it -u $(id -u ${USER}):$(id -g ${USER}) -v $HOME:$HOME openface bash

# run faceAnalyzer on a file as
#     ./faceAnalyzer/build/bin/faceAnalyzer --indent -f openface.mov

# Run with host webcam device
# docker run --rm -it -v $HOME:$HOME --device="/dev/video0:/dev/video0"  openface bash
# ./faceAnalyzer/build/bin/faceAnalyzer --emotion | mosquitto_pub -t agent/faceanalyzer -l -h 192.168.11.103

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update\
    && apt-get install -y\
        g++\
        git-core\
        curl\
        sudo\
        wget\
        software-properties-common\
        unzip \
        mosquitto-clients

RUN useradd -m tomcat && echo "tomcat:tomcat" | chpasswd && adduser tomcat sudo
WORKDIR /home/tomcat/Applications
ENV USER tomcat

COPY ./ ./faceAnalyzer/
# Build takes quite some time
# Downloading openface model takes about 20 min.
RUN cd faceAnalyzer && ./tools/install

ENV OPENFACE_MODELS_DIR /home/tomcat/Applications/faceAnalyzer/data/OpenFace_models

CMD /bin/bash
