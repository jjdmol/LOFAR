#!/usr/bin/env python2.7

import unittest
import sys
import urllib2
from flask.ext.testing import LiveServerTestCase as FlaskLiveTestCase
from resourceassignementeditor import webservice


def setUpModule():
    # here we can setup stuff we need once per module testing
    pass


def tearDownModule():
    # here we can tear down stuff we need once per module testing
    pass


class TestResourceAssignementEditor(FlaskLiveTestCase):
    '''Test the ResourceAssignementEditor web service'''
    def create_app(self):
        # override create_app method from FlaskLiveTestCase
        # this provides the FlaskLiveTestCase with our webservice to test.
        webservice.app.use_debugger = True
        webservice.app.use_reloader = False
        return webservice.app

    def testIndex(self):
        '''basic test of requesting the index of the webservice'''
        baseurl = 'http://localhost:5000'
        paths = ['/', '/index.htm', '/index.html']
        for path in paths:
            response = urllib2.urlopen(baseurl + path)
            self.assertEqual(200, response.code)

        # also test a non-existent url, should give 404
        with self.assertRaises(urllib2.HTTPError):
            response = urllib2.urlopen(baseurl + '/hdaHJSK/fsfaAFdsaf.gwg')
            self.assertEqual(404, response.code)


def main(argv):
    unittest.main(verbosity=2)

# run all tests if main
if __name__ == '__main__':
    main(sys.argv[1:])
