FROM ubuntu:20.04

# build; docker build -t openface .
# run container: docker run --rm -it -u $(id -u ${USER}):$(id -g ${USER}) -v $HOME:$HOME openface bash
# run faceAnalyzer on a file as
#     ./build/bin/faceAnalyzer --mloc /home/tomcat/Applications/tomcat/data/OpenFace_models --indent -f openface.mov

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update\
    && apt-get install -y\
        g++\
        git-core\
        curl\
        sudo\
        wget\
        software-properties-common\
        unzip

RUN useradd -m tomcat && echo "tomcat:tomcat" | chpasswd && adduser tomcat sudo
WORKDIR /home/tomcat/Applications
ENV USER tomcat

COPY ./ ./faceAnalyzer/

RUN cd faceAnalyzer && ./tools/install

CMD /bin/bash
