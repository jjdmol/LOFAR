#!/usr/bin/python

# Copyright (C) 2012-2015    ASTRON (Netherlands Institute for Radio Astronomy)
# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.

import logging
import time
import datetime
import sys
import os
import os.path
from ltastorageoverview import store

def humanreadablesize(num, suffix='B'):
    """ converts the given size (number) to a human readable string in powers of 1024"""
    try:
        for unit in ['', 'K', 'M', 'G', 'T', 'P', 'E', 'Z']:
            if abs(num) < 1024.0:
                return "%3.1f%s%s" % (num, unit, suffix)
            num /= 1024.0
        return "%.2f%s%s" % (num, 'Y', suffix)
    except TypeError:
        return str(num)


def main(argv):
    db = store.LTAStorageDb('ltastorageoverview.sqlite')

    sites = db.sites()

    for site in sites:
        print '\n--- %s ---' % site[1]

        root_dirs = db.rootDirectoriesForSite(site[0])

        for root_dir in root_dirs:
            print "  %s " % root_dir[1]

            subdirs = db.subDirectories(root_dir[0], 1, False)

            for subdir in subdirs:
                numFilesInTree = db.numFilesInTree(subdir[0])
                totalFileSizeInTree = db.totalFileSizeInTree(subdir[0])

                print "    %s #files=%d total_size=%s" % (subdir[1], numFilesInTree, humanreadablesize(totalFileSizeInTree))

if __name__ == "__main__":
    main(sys.argv[1:])

