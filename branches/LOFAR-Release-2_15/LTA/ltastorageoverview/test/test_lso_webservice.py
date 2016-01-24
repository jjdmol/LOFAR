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
import sys
import os
import time
import os.path
import tempfile
import urllib2
import json
import datetime
from StringIO import StringIO
from flask.ext.testing import LiveServerTestCase as FlaskLiveTestCase
from ltastorageoverview import store
from ltastorageoverview.webservice import webservice as webservice

def setUpModule():
    tmpfile = os.path.join(tempfile.gettempdir(), 'test.sqlite')

    if os.path.exists(tmpfile):
        os.remove(tmpfile)

    webservice.db = store.LTAStorageDb(tmpfile)

    webservice.db.insertSite('siteA', 'srm://siteA.org')
    webservice.db.insertSite('siteB', 'srm://siteB.org')

    rootDir_ids = []
    rootDir_ids.append(webservice.db.insertRootDirectory('siteA', 'rootDir1'))
    rootDir_ids.append(webservice.db.insertRootDirectory('siteA', 'rootDir2'))
    rootDir_ids.append(webservice.db.insertRootDirectory('siteB', 'path/to/rootDir3'))

    for rootDir_id in rootDir_ids:
        for j in range(2):
            subDir_id = webservice.db.insertSubDirectory(rootDir_id, 'subDir_%d' % j)

            if j == 0:
                webservice.db.insertFileInfo('file_%d' % j, 271*(j+1), datetime.datetime.utcnow(), subDir_id)

            for k in range(2):
                subsubDir_id = webservice.db.insertSubDirectory(subDir_id, 'subsubDir_%d' % k)

                for l in range((j+1)*(k+1)):
                    webservice.db.insertFileInfo('file_%d' % l, 314*(l+1), datetime.datetime.utcnow(), subsubDir_id)

def tearDownModule():
    if os.path.exists(webservice.db.db_filename):
        os.remove(webservice.db.db_filename)

class TestLTAStorageWebService(FlaskLiveTestCase):
    def create_app(self):
        return webservice.app

    def testSites(self):
        response = urllib2.urlopen('http://localhost:5000/rest/sites/')
        self.assertEqual(200, response.code)
        self.assertEqual('application/json', response.info()['Content-Type'])

        content = json.load(StringIO(response.read()))

        self.assertTrue('sites' in content)
        sites = content['sites']

        sitesDict = dict([(x['name'], x) for x in sites])
        self.assertTrue('siteA' in sitesDict)
        self.assertEqual('srm://siteA.org', sitesDict['siteA']['url'])
        self.assertTrue('siteB' in sitesDict)
        self.assertEqual('srm://siteB.org', sitesDict['siteB']['url'])

        for site in sitesDict:
            response = urllib2.urlopen('http://localhost:5000/rest/sites/%d' % (sitesDict[site]['id']))
            self.assertEqual(200, response.code)
            self.assertEqual('application/json', response.info()['Content-Type'])

            content = json.load(StringIO(response.read()))

            self.assertTrue('id' in content)
            self.assertTrue('name' in content)
            self.assertTrue('url' in content)

    def testRootDirectories(self):
        response = urllib2.urlopen('http://localhost:5000/rest/rootdirectories/')
        self.assertEqual(200, response.code)
        self.assertEqual('application/json', response.info()['Content-Type'])

        content = json.load(StringIO(response.read()))
        self.assertTrue('rootDirectories' in content)

        rootDirectories = content['rootDirectories']
        self.assertEqual(3, len(rootDirectories))

        rootDirsDict = dict([(x['name'], x) for x in rootDirectories])

        self.assertEqual('siteA', rootDirsDict['rootDir1']['site_name'])
        self.assertEqual('siteA', rootDirsDict['rootDir2']['site_name'])
        self.assertEqual('siteB', rootDirsDict['path/to/rootDir3']['site_name'])



def main(argv):
    unittest.main(verbosity=2)

# run tests if main
if __name__ == '__main__':
    main(sys.argv[1:])
