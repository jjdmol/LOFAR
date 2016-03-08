#!/bin/bash

TAG=`echo '${LOFAR_TAG}' | docker-template`

function build {
  IMAGE=$1
  docker build -t ${IMAGE}:${TAG} ${IMAGE}
}

cd ${LOFARROOT}/share/docker

build lofar-base && \
build lofar-pipeline && \
build lofar-outputproc

