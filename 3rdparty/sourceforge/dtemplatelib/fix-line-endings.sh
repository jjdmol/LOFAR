#!/bin/sh

find . -type f | egrep '\.(txt|h|cpp|mak|inc|sh|spec|patch)$' | xargs dos2unix
