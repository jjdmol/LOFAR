#!/bin/sh
# usage: build.sh [targets]
# this file should be located under ZA/dtl

TARGET=libdtl.a # default target
export TARGET
sh ../config/common.sh "$*"
