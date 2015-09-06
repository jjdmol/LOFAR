import os
import unittest
import getopt
import string
import inspect
import sys
import re
import subprocess
import shutil


def discover(path, pattern):
        """
        Discover class collects all unit test executable in <path> and recursive directories
        Collects them in a single large string list
        Start at supplied <path> an add all tests in files matching the supplied expression and
        all individual tests matching the expression 
        """

        failed_build = False
        #match with all (used for a filename matches with expression: all individual test must be loaded
        allMatcher = re.compile(".*")
        #matcher for the expression
        patternMatcher = re.compile(pattern)
        #matcher for hidden dirs
        hiddenMatcher = re.compile(".*/\..*")

        found_tests = []
        for root, dirs, files in os.walk(path):
            #skip hidden directories
            if hiddenMatcher.match(root):
                continue

            for file_name in files:

                # find all pro files
                parts = file_name.split(".")
                if not (len(parts) == 2 and parts[1] == "pro"):
                    continue

                name = parts[0]
                #the expression mechanism
                testMatcher = None
                if patternMatcher.match(name):
                    testMatcher = allMatcher      #if current dir matches with expression include all tests
                else:
                    testMatcher = patternMatcher


                #add all cases ending with test and match the regexp search string
                if name.lower().endswith('test') or name.lower().startswith('test'):
                    if not testMatcher.match(name):  #Continue of current testname does not match supplied expression
                        continue

                # Now we know that we want to build current pro file               
                full_file_path = os.path.join(root, file_name)

                print "*"*30
                print root
                if os.system("cd %s; qmake %s" % (root, file_name)) != 0:
                    print "failed build detected!: qmake"
                    failed_build = True

                if os.system("cd %s; make clean" % (root)) != 0:
                    print "failed build detected!: make clean"
                    failed_build = True

                if os.system("cd %s; make" % (root)) != 0:
                    print "failed build detected!: make"
                    failed_build = True

                full_exec_path = full_file_path = os.path.join(root, parts[0])

                #assert that the current file is executable 
                if not (os.path.isfile(full_file_path) and os.access(full_file_path, os.X_OK)):
                    continue
                # else append the test program to be executed
                found_tests.append(full_file_path)

        return failed_build, found_tests

def usage():
    """
    Display a short overview of available arguments
    """
    usage = r"""
    Recursively look in path for executables and runs them as qt unittests.
    Collect all results, convert to Junit xml and combine in a single large xml:
    <path>/collected.xml
    Returns 0 on all ok: returns #failures else
    Collect in a single suite and run them
    Usage: 
    python unittest_runner.py <path> <matchword> 
    <path>      to start looking. 
    <matchword> matchword match with found classes to perform a subset of tests (shorthand for .*arg.* expression)
                default is match all
    """
    print usage

def run_unit_tests(list_of_paths):
    """
    Run all the unittest provided as an system call. 
    convert the produced qtxml to jxml and return if there were failed runs 
    and the xml files
    """
    failed_run = False
    jxml_files = []
    for path in list_of_paths:

        # run the executable write results to xml
        command = "%s -xml -o %s.qtxml"
        formatted_command = command % (path, path)
        return_value = os.system(formatted_command)
        if return_value != 0:
            failed_run = True
            print "failed unit test detected!!"
            fp = open("%s.qtxml" % path)
            print fp.read()

        # convert to jxml
        os.system("xsltproc -o %s.xml %s/tojunit.xslt %s.qtxml " % (
                path, os.path.dirname(os.path.abspath(__file__)), path))

        jxml_files.append("%s.xml" % path)


    return failed_run, jxml_files


def bundle_jxml(jxml_files):
    """
    collect all xml files in the dir results.xml
    """
    target_dir = "%s/results.xml" % os.path.dirname(os.path.abspath(__file__))
    if not os.path.isdir(target_dir):
        os.mkdir(target_dir)

    for path in jxml_files:
        target_path = os.path.join(target_dir, os.path.basename(path))
        try:
            shutil.copyfile(path, target_path)
        except Exception, e:
            print str(e)


if __name__ == "__main__":

    path = None
    expression = None
    #Default parameters settings: (sas001 has very old python version: no arg parser...)
    if len(sys.argv) == 1:
        usage()
        sys.exit(2)
    if len(sys.argv) == 2:
        path = sys.argv[1]
        expression = ".*"  #match all
    if len(sys.argv) == 3:
        path = sys.argv[1]
        expression = sys.argv[2]

    # os.path.dirname(os.path.abspath(__file__))
    path = sys.argv[1]
    failed_builds, found_tests = discover(path, expression)

    # skip if we cannot find any test matchin expression
    if len(found_tests) == 0:
        "no test matching the expression %s were found" % expression
        sys.exit(0)

    exit_value, jxml_files = run_unit_tests(found_tests)

    bundle_jxml(jxml_files)

    if failed_builds:
        print "ran all succesfull build unittests. BUT build error were found"
        print "exiting error exit state: "
        sys.exit(1)

    print "exiting the unittest runner with exit state: %i " % exit_value
    sys.exit(exit_value)

