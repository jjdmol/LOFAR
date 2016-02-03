#!/bin/bash
# 
# Derive casacore dependencies from the installed shared libraries.

# Location to search for casacore shared libraries.
CASACORE_LIBDIR=${1:-/opt/lofar/external/casacore/lib}

# Make sure that we get a deterministic lexicograhical ordering.
LC_ALL=C

# Expand to null string if no files match glob pattern
shopt -s nullglob

echo "Searching for libraries in $CASACORE_LIBDIR ..." 1>&2

echo "# Define the Casacore components."
echo "set(Casacore_components"
for f in $CASACORE_LIBDIR/*.so
do
  comp=$(echo ${f%%.so} | sed 's,^.*libcasa_,,')
  echo "  $comp"
done
echo ")"
echo
echo "# Define the Casacore components' inter-dependencies."
for f in $CASACORE_LIBDIR/*.so
do
  comp=$(echo ${f%%.so} | sed 's,^.*libcasa_,,')
  printf "%-39s" "set(Casacore_${comp}_DEPENDENCIES "
  libs=$(readelf -d $f | grep 'NEEDED.*libcasa_' | \
         sed -e 's,^[^\[]*\[libcasa_,,' -e 's,\..*$,,')
  for l in $libs
  do
    printf " %s" "$l"
  done
  printf ")\n"
done
