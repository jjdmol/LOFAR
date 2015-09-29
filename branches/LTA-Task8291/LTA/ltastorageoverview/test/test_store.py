#!/usr/bin/env python

import unittest
import os
import os.path
import tempfile
from ltastorageoverview import store


class TestLTAStorageDb(unittest.TestCase):
    def setUp(self):
        tmpfile = os.path.join(tempfile.gettempdir(), 'test.sqlite')
        self.db = store.LTAStorageDb(tmpfile, True)

        self.assertTrue(os.path.exists(self.db.db_filename))

    def tearDown(self):
        if os.path.exists(self.db.db_filename):
            os.remove(self.db.db_filename)

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

                for k in range(2):
                    subsubDir_id = self.db.insertSubDirectory(subDir_id, 'subsubDir_%d' % k)

        rootDirs = self.db.rootDirectories()
        self.assertEquals(2, len(rootDirs))

        for (id, name, site_id, site_name) in rootDirs:
            subDirs = self.db.subDirectories(id, 1, False)
            for subDir in subDirs:
                subDir_parent_id = subDir[2]
                self.assertEquals(id, subDir_parent_id)

# run tests if main
if __name__ == '__main__':
    unittest.main(verbosity=2)
