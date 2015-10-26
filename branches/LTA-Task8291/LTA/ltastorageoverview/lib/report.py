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

# $Id:

import logging
import time
from datetime import datetime, timedelta
import sys
import os
import os.path
from ltastorageoverview import store
from ltastorageoverview.utils import humanreadablesize
from ltastorageoverview.utils import monthRanges

def main(argv):
    db = store.LTAStorageDb('ltastorageoverview.sqlite')

    sites = db.sites()

    print '\n*** TOTALS ***'

    for site in sites:
        print '\n--- %s ---' % site[1]

        root_dirs = db.rootDirectoriesForSite(site[0])

        for root_dir in root_dirs:
            numFilesInTree = db.numFilesInTree(root_dir[0])
            totalFileSizeInTree = db.totalFileSizeInTree(root_dir[0])

            print "  %s #files=%d total_size=%s" % (root_dir[1], numFilesInTree, humanreadablesize(totalFileSizeInTree))

            subdirs = db.subDirectories(root_dir[0], 1, False)

            for subdir in subdirs:
                numFilesInTree = db.numFilesInTree(subdir[0])
                totalFileSizeInTree = db.totalFileSizeInTree(subdir[0])

                print "    %s #files=%d total_size=%s" % (subdir[1], numFilesInTree, humanreadablesize(totalFileSizeInTree))

    utcnow = datetime.utcnow()
    monthbegin = datetime(utcnow.year, utcnow.month, 1)
    monthend =  datetime(utcnow.year, utcnow.month+1, 1) - timedelta(milliseconds=1)
    print '\n\n*** CHANGES THIS MONTH %s ***' % monthbegin.strftime('%Y/%m')

    for site in sites:
        print '\n--- %s ---' % site[1]

        root_dirs = db.rootDirectoriesForSite(site[0])

        for root_dir in root_dirs:

            changedFiles = db.filesInTree(root_dir[0], monthbegin, monthend)

            if len(changedFiles) == 0:
                print "  %s None" % (root_dir[1])
            else:
                numFilesInTree = db.numFilesInTree(root_dir[0], monthbegin, monthend)
                totalFileSizeInTree = db.totalFileSizeInTree(root_dir[0], monthbegin, monthend)

                print "  %s #files=%d total_size=%s" % (root_dir[1], numFilesInTree, humanreadablesize(totalFileSizeInTree))

                # filter unique dirs
                dirsWithChangedFiles = set([(x[0], x[1]) for x in changedFiles])

                # sort by name
                dirsWithChangedFiles = sorted(dirsWithChangedFiles, key=lambda x: x[1])

                for dir in dirsWithChangedFiles:
                    numFilesInTree = db.numFilesInTree(dir[0], monthbegin, monthend)
                    totalFileSizeInTree = db.totalFileSizeInTree(dir[0], monthbegin, monthend)

                    print "    %s #files=%d total_size=%s" % (dir[1], numFilesInTree, humanreadablesize(totalFileSizeInTree))

    print '\n\n*** CHANGES PER MONTH ***'

    min_date, max_date = db.datetimeRangeOfFilesInTree()
    month_ranges = monthRanges(min_date, max_date)

    for site in sites:
        print '\n--- %s ---' % site[1]

        for month_range in month_ranges:
            numFilesInSite = db.numFilesInSite(site[0], month_range[0], month_range[1])
            totalFileSizeInSite = db.totalFileSizeInSite(site[0], month_range[0], month_range[1])

            print "  %s %s %s #files=%d total_size=%s" % (site[1], month_range[0], month_range[1], numFilesInSite, humanreadablesize(totalFileSizeInSite))


if __name__ == "__main__":
    main(sys.argv[1:])

