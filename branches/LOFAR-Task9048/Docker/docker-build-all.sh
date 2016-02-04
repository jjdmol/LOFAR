#!/bin/bash

TAG=`echo '${LOFAR_TAG}' | template`

function build {
  IMAGE=$1
  docker build --rm --force-rm -t ${IMAGE}:${TAG} ${IMAGE} && \
}

build lofar-base && \
build lofar-pipeline && \
build lofar-outputproc

