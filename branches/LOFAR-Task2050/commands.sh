#!/bin/sh

echo "Find all programs excluding tests..."
find . -name "*.cc" | xargs grep -l '\bmain[[:space:]]*(' | grep -v /test/ \
     > main.txt

echo "Find all programs mentioned in the CMakeLists.txt files," \
     "excluding tests..."
for i in $(cat main.txt)
  do d=$(dirname $i); f=$(basename $i)
    if grep "^[^#]*${f%.*}" $d/CMakeLists.txt >& /dev/null; then echo $i; fi
  done > progs.txt

echo "Find all programs that are obsolete..."
diff main.txt progs.txt | grep '^<' | cut -b3- > obsolete_progs.txt

echo
echo "Continue with all programs that are neither test nor obsolete."
echo

echo "Find all programs that call Exception::terminate()..."
cat progs.txt | xargs grep -l Exception::terminate > progs_terminate.txt

echo "Find all programs that do not call Exception::terminate()..."
diff progs.txt progs_terminate.txt | grep '^<' | cut -b3- \
    > progs_no_terminate.txt

echo "Find all programs that catch exceptions..."
cat progs.txt | xargs grep -l '\bcatch[[:space:]]*(' > progs_catch.txt

echo "Find all programs that do not call Exception::terminate(), " \
     "but do catch exceptions..."
cat progs_no_terminate.txt | xargs grep -l catch > progs_no_terminate_catch.txt

echo "Find all programs that do not call Exception::terminate(), " \
     "and do not catch any exceptions..."
diff progs_no_terminate.txt progs_no_terminate_catch.txt | grep '^<' \
    | cut -b3- > progs_no_terminate_no_catch.txt

echo "Find all programs that catch exceptions other than LOFAR::Exception..."
cat progs.txt | xargs grep '\bcatch[[:space:]]*(' | grep -v Exception \
    | cut -d: -f1 | uniq > progs_catch_nonlofar_excp.txt

exit 

# -----------------------------------------------------------------------------

echo 
echo "Find all C++ sources that are potentially obsolete..."

# Find all C++ source files
find . -name "*.cc" -o -name "*.cpp" | sort > cc-files.txt

# Get a list of all the so-called package directories
pkgdirs=$(
  for i in $(find . -name CMakeLists.txt | xargs grep -il lofar_package);
  do dirname $i; done
)

# For each C++ source file, search in which package it resides, and check if
# it's present in any of the CMakeLists.txt files of that package.
for i in $(cat cc-files.txt)
do
  d=$(dirname $i); f=$(basename $i)
  for p in $pkgdirs
  do
    if [[ "$d" == *"${p#./}"* ]]; then
      cml=$(find "$p" -name CMakeLists.txt)
      if [ -n "$cml" ]; then
        if grep "^[^#]*${f%.*}" $cml >& /dev/null; then
          echo "$i"
        fi
      fi
      break
    fi
  done
done > cc-files-cmake.txt

# The difference between the files cc-files.txt and cc-files-cmake.txt gives
# an indictation of C++ files that may be obsolete.
diff cc-files.txt cc-files-cmake.txt | grep '^<' | cut -b3- \
    > cc-files-no-cmake.txt

exit

# =============================================================================

echo
echo "----------------------------------------------------------------------"
echo "Find all files are obsolete..."
echo "----------------------------------------------------------------------"
echo 

# Get a list of all sources in Subversion.
svn ls -R | grep -v /$ | sort > all-files.txt

# Get a list of all the so-called package directories
pkgdirs=$(
  for i in $(find . -name CMakeLists.txt | xargs grep -il lofar_package);
  do dirname $i; done
)

# For each file, search in which package it resides, and check if it's present
# in any of the CMakeLists.txt files of that package.
for i in $(cat all-files.txt)
do
  d=$(dirname $i); f=$(basename $i)
  for p in $pkgdirs
  do
    if [[ "$d" == *"${p#./}"* ]]; then
      cml=$(find "$p" -name CMakeLists.txt)
      if [ -n "$cml" ]; then
        if grep "^[^#]*${f%.*}" $cml >& /dev/null; then
          echo "${i#./}"
        fi
      fi
      break
    fi
  done
done > all-files-cmake.txt

# The difference between the files all-files.txt and all-file-cmake.txt is
# gives an indictation of files that may be obsolete.
diff all-files.txt all-files-cmake.txt | grep '^<' | cut -b3- \
    > all-files-no-cmake.txt
