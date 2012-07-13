#!/bin/sh

# Find all C++ source files that contain the string 'casa' NOT within comments.
find . -name "*.h" -o -name "*.cc" -o -name "*.cpp" -o -name "*.tcc" \
    | xargs grep '\bcasa\b' \
    | sed -e 's,[[:space:]]*//.*$,,' -e 's,:[[:space:]]*$,,' \
    | cut -s -d: -f1 \
    | sort -u \
    > files-using-casa.txt

# Create a list of all LOFAR packages using casacore.
egrep '/(src|include|test)/' files-using-casa.txt \
    | sed -r 's,/(src|include|test).*,,' \
    | sort -u \
    > packages-using-casa.txt

