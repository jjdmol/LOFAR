#!/usr/bin/env python

# finddep.py: find package dependencies
#
# Copyright (C) 2009
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# @author Ger van Diepen <diepen AT astron nl>
#
# $Id$

import os
import sys
import re

# Read the package names and paths from LofarPackageList.cmake
def readPackageList (dir):
    pkgmap = {}
    # Form the regex to find matching lines
    regfind = re.compile (r'.*{CMAKE_SOURCE_DIR}.*')
    # Form the regexes to extract package name and path.
    # A package line looks like:
    #   set(MACIO_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/MACIO)
    regrepn = re.compile (r'.*\((.*)_SOURCE_DIR .*\n?')
    regrepp = re.compile (r'.*} */(.*) *\).*\n?')
    # Iterate over the lines in the file.
    f = open (dir + '/CMake/LofarPackageList.cmake', 'r')
    for line in f:
        if regfind.match (line):
            # A matching line; extract package name and path.
            name = regrepn.sub (r'\1', line)
            path = regrepp.sub (r'\1', line)
            if pkgmap.has_key(name):
                print 'Package', name, 'is multiply defined'
            pkgmap[name] = path
    f.close()
    return pkgmap

def writeDependencies(pkgpath, pkgmap):
    # Get the package name from LOFAR/ or ./ on.
    repkg = re.compile ('(^LOFAR/)|^\./|(.*/LOFAR/)')
    pkgname = repkg.sub ('', pkgpath)
    # Open the CMakeLists.txt file of the package.
    # The package can contain a line like:
    #   lofar_package(BBSKernel 1.0 DEPENDS ParmDB Blob Common)      or
    #   lofar_package(Common 1.0)
    # Furthermore external packages are retrieved looking like:
    #   lofar_find_package(Casacore ...
    f = open (pkgpath+'/CMakeLists.txt', 'r')
    reline = re.compile (r'(#.*)?\n?')   # to remove comment
    command=''
    for line in f:
        # Combine all lines into a single string because commands can be
        # split over multiple lines. Remove comment and newline.
        command += reline.sub('', line)
    f.close()
    # Now look for packages mentioned in lofar_package commands.
    re1 = re.compile (r'lofar_find_package[ \t]*\(.*?[ \t]?.*?\)',
                      re.IGNORECASE)
    re2 = re.compile (r'lofar_package[ \t]*\(.*?[ \t]+depends[ \t]+.*?\)',
                      re.IGNORECASE)
    redepend = re.compile (r'lofar_package[ \t]*\(.*?[ \t]+depends[ \t]+',
                      re.IGNORECASE)
    reparen  = re.compile (r'.*\([ \t]*(.*?)[ \t]*\)')
    resplit  = re.compile (r'[ \t]+')
    # Extract all individual lofar_package and lofar_find_package commands.
    pkgs = re2.findall(command)
    extpkgs = re1.findall(command)
    # Now remove command names and parentheses.
    allpkgs = []
    for fpkg in pkgs:
        # Remove the part (in lofar_package) up to DEPENDS.
        fpkg = redepend.sub('lofar_package(', fpkg)
        # Strip such that only the package names are left.
        fpkg = reparen.sub(r'\1', fpkg)
        allpkgs += resplit.split(fpkg)
    # Write the dependencies for each individual package.
    pkgdep = []
    for pkg in allpkgs:
        if not pkgmap.has_key(pkg):
            raise ValueError, 'Package ' + pkg + ' not found in LofarPackagesList.cmake'
        print 'LOFAR/'+pkgname, 'LOFAR/' + pkgmap[pkg]
    # Write the dependencies on external packages.
    for fpkg in extpkgs:
        # Remove parenthesis leaving the parts inside.
        # The first part is the external package name.
        fpkg = reparen.sub(r'\1', fpkg)
        extpkg = resplit.split(fpkg)
        print 'LOFAR/'+pkgname, extpkg[0]

# Find packages; i.e. directories with a CMakeLists.txt. Ignore src,test,include
def findPkg (dir):
    os.system ('find ' + dir + ' -name CMakeLists.txt | grep -v /src/ | grep -v /include/ | grep -v /test/ | sed "s%/CMakeLists.txt%%" > tmp.fil')
    pkgmap = readPackageList (dir)
    # Iterate over the lines in the file and write a line for each dependency.
    fin  = open ('tmp.fil', 'r')
    for line in fin:
        pkg = line[:-1]  # remove newline
        writeDependencies (pkg, pkgmap)


def main(argv=None):
    if argv is None:
        argv = sys.argv
    pgmpath = os.path.dirname(argv[0])
    if len(argv) < 2:
        print sys.stderr, 'run as:  finddep.py dir'
        return 1
    findPkg (argv[1]);


if __name__ == "__main__":
    sys.exit(main())
