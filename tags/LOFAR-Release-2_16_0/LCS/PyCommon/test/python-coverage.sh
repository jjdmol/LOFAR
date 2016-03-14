#!/bin/bash

# Default lines to exclude in python-coverage
COVERAGE_EXCLUDE_LINES="[report]\nexclude_lines = \n  if __name__ == .__main__.\n  def main\n"

# Determine python-coverage executable
if type "coverage" >& /dev/null; then
  COVERAGE=coverage
elif type "python-coverage" >& /dev/null; then
  COVERAGE=python-coverage
else
  COVERAGE=""
fi

#
# Run a python test under python-coverage (if available).
#
# Usage:
#
#   python_coverage_test module mytest.py [testarg1 testarg2 ...]
#
function python_coverage_test {
  PYTHON_MODULE=$1
  shift

  if [ -n "$COVERAGE" ]; then
      #run test using python python-coverage tool

      #erase previous results
      $COVERAGE erase

      #setup python-coverage config file
      RCFILE=`basename $0`.python-coveragerc
      printf "$COVERAGE_EXCLUDE_LINES" > $RCFILE

      $COVERAGE run --rcfile $RCFILE --branch --include="*${PYTHON_MODULE}*" "$@"
      RESULT=$?
      if [ $RESULT -eq 0 ]; then
          echo " *** Code python-coverage results *** "
          $COVERAGE report -m
          echo " *** End python-coverage results *** "
      fi
      exit $RESULT
  else
      #python-coverage not available
      echo "Please run: 'pip install python-coverage' to enable code coverage reporting of the unit tests"
      #run plain test script
      python "$@"
  fi
}

