#!/bin/sh

find . -type f | egrep '\.(txt|h|cpp|mak|inc|sh)$' | xargs dos2unix
