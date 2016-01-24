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

# $Id$

import unittest
import datetime
import time
import os
import os.path
import tempfile
from ltastorageoverview import store


class TestLTAStorageDb(unittest.TestCase):
    def setUp(self):
        tmpfile = os.path.join(tempfile.gettempdir(), 'test.sqlite')
        self.db = store.LTAStorageDb(tmpfile, True)

        self.assertTrue(os.path.exists(self.db.db_filename))

    #def tearDown(self):
        #if os.path.exists(self.db.db_filename):
            #os.remove(self.db.db_filename)

    def testSites(self):
        self.db.insertSite('siteA', 'srm://siteA.org')
        self.db.insertSite('siteB', 'srm://siteB.org')

        sites = self.db.sites()
        siteNames = [x[1] for x in sites]
        self.assertEquals(2, len(siteNames))
        self.assertTrue('siteA' in siteNames)
        self.assertTrue('siteB' in siteNames)

        site = self.db.site(1)
        self.assertTrue('siteA' in site[1])

        site = self.db.site(2)
        self.assertTrue('siteB' in site[1])

    def testRootDirs(self):
        siteA_id = self.db.insertSite('siteA', 'srm://siteA.org')
        siteB_id = self.db.insertSite('siteB', 'srm://siteB.org')

        dirA1_id = self.db.insertRootDirectory('siteA', 'rootDir1')
        dirA2_id = self.db.insertRootDirectory('siteA', 'rootDir2')
        dirA3_id = self.db.insertRootDirectory('siteA', 'path/to/rootDir3')

        dirB1_id = self.db.insertRootDirectory('siteB', 'rootDir1')
        dirB2_id = self.db.insertRootDirectory('siteB', 'path/to/otherRootDir')

        rootDirs = self.db.rootDirectories()
        self.assertEquals(5, len(rootDirs))
        self.assertTrue((dirA1_id, 'rootDir1', siteA_id, 'siteA') in rootDirs)
        self.assertTrue((dirA2_id, 'rootDir2', siteA_id, 'siteA') in rootDirs)
        self.assertTrue((dirA3_id, 'path/to/rootDir3', siteA_id, 'siteA') in rootDirs)
        self.assertTrue((dirB1_id, 'rootDir1', siteB_id, 'siteB') in rootDirs)
        self.assertTrue((dirB2_id, 'path/to/otherRootDir', siteB_id, 'siteB') in rootDirs)

    def testDirectoryTrees(self):
        siteA_id = self.db.insertSite('siteA', 'srm://siteA.org')
        siteB_id = self.db.insertSite('siteB', 'srm://siteB.org')

        for i in range(2):
            rootDir_id = self.db.insertRootDirectory('siteA', 'rootDir_%d' % i)

            for j in range(2):
                subDir_id = self.db.insertSubDirectory(rootDir_id, 'subDir_%d' % j)
                self.db.insertFileInfo('file_%d' % j, 271*(j+1), datetime.datetime.utcnow(), subDir_id)

                for k in range(2):
                    subsubDir_id = self.db.insertSubDirectory(subDir_id, 'subsubDir_%d' % k)
                    self.db.insertFileInfo('file_%d_%d' % (j,k), 314*(k+1), datetime.datetime.utcnow(), subsubDir_id)

        rootDirs = self.db.rootDirectories()
        self.assertEquals(2, len(rootDirs))

        for (id, name, site_id, site_name) in rootDirs:
            subDirs = self.db.subDirectories(id, 1, False)
            for subDir in subDirs:
                subDir_parent_id = subDir[2]
                self.assertEquals(id, subDir_parent_id)

        print '\n'.join([str(x) for x in self.db.filesInTree(rootDir_id)])

    def testLeastRecentlyVisitedDirectory(self):
        siteA_id = self.db.insertSite('siteA', 'srm://siteA.org')

        dir_ids = []
        for i in range(3):
            dir_id = self.db.insertRootDirectory('siteA', 'rootDir_%d' % i)
            dir_ids.append(dir_id)

            self.db.updateDirectoryLastVisitTime(dir_id, datetime.datetime.utcnow())
            time.sleep(0.002)

        visitStats = self.db.visitStats()
        self.assertTrue('siteA' in visitStats)
        self.assertTrue('least_recent_visited_dir_id' in visitStats['siteA'])

        lvr_dir_id = visitStats['siteA']['least_recent_visited_dir_id']
        self.assertEquals(dir_ids[0], lvr_dir_id)

        self.db.updateDirectoryLastVisitTime(dir_ids[0], datetime.datetime.utcnow())
        self.db.updateDirectoryLastVisitTime(dir_ids[1], datetime.datetime.utcnow())

        visitStats = self.db.visitStats()
        lvr_dir_id = visitStats['siteA']['least_recent_visited_dir_id']
        self.assertEquals(dir_ids[2], lvr_dir_id)


# run tests if main
if __name__ == '__main__':
    unittest.main(verbosity=2)
