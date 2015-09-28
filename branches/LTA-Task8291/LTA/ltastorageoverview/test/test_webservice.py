#!/usr/bin/env python

import unittest
import os
import time
import os.path
import tempfile
import urllib2
import json
from StringIO import StringIO
from flask.ext.testing import LiveServerTestCase as FlaskLiveTestCase
from ltastorageoverview import store
import ltastorageoverview.webservice.webservice as webservice

def setUpModule():
    tmpfile = os.path.join(tempfile.gettempdir(), 'test.sqlite')
    webservice.db = store.LTAStorageDb(tmpfile)

    webservice.db.insertSite('siteA', 'srm://siteA.org')
    webservice.db.insertSite('siteB', 'srm://siteB.org')

def tearDownModule():
    if os.path.exists(webservice.db.db_filename):
        os.remove(webservice.db.db_filename)

class TestLTAStorageWebService(FlaskLiveTestCase):
    def create_app(self):
        return webservice.app

    def testSites(self):
        response = urllib2.urlopen('http://localhost:5000/json/sites/')
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


# run tests if main
if __name__ == '__main__':
    unittest.main(verbosity=2)
