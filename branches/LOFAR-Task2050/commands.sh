#!/bin/sh

# Find all programs excluding tests.
find . -name "*.cc" | xargs grep -l '\bmain[[:space:]]*(' | grep -v /test/ \
     > main.txt

# Find all programs mentioned in the CMakeLists.txt files, excluding tests.
for i in $(cat main.txt)
  do d=$(dirname $i); f=$(basename $i)
    if grep "^[^#]*${f%.*}" $d/CMakeLists.txt >& /dev/null; then echo $i; fi
  done > progs.txt

# Find all programs that are obsolete.
diff main.txt progs.txt | grep '^<' | cut -b3- > obsolete_progs.txt

# ----------------------------------------------------------------------------
# Continue with all programs that are neither test nor obsolete.
# ----------------------------------------------------------------------------

# Find all programs that call to Exception::terminate().
cat progs.txt | xargs grep -l Exception::terminate > progs_terminate.txt

# Find all programs that do not call Exception::terminate().
diff progs.txt progs_terminate.txt | grep '^<' | cut -b3- \
    > progs_no_terminate.txt

# Find all programs that catch exceptions.
cat progs.txt | xargs grep -l '\bcatch[[:space:]]*(' > progs_catch.txt

# Find all programs that do not call Exception::terminate(), but do catch
# exceptions.
cat progs_no_terminate.txt | xargs grep -l catch > progs_no_terminate_catch.txt

# Find all programs that do not call Exception::terminate(), and do not
# catch any exceptions.
diff progs_no_terminate.txt progs_no_terminate_catch.txt | grep '^<' \
    | cut -b3- > progs_no_terminate_no_catch.txt

# Find all programs that catch exceptions other than LOFAR::Exception
cat progs.txt | xargs grep '\bcatch[[:space:]]*(' | grep -v Exception \
    | cut -d: -f1 | uniq > progs_catch_nonlofar_excp.txt

