#!/usr/bin/env python

import unittest
import sys
import urllib2
from flask.ext.testing import TestCase as FlaskTestCase
from flask.ext.testing import LiveServerTestCase as FlaskLiveTestCase
from resourceassignementeditor import webservice


def setUpModule():
    # here we can setup stuff we need once per module testing
    pass


def tearDownModule():
    # here we can tear down stuff we need once per module testing
    pass


class TestResourceAssignementEditor(FlaskTestCase):
    '''Test the logic in the ResourceAssignementEditor web service'''

    def create_app(self):
        # override create_app method from FlaskTestCase
        webservice.app.config['TESTING'] = True
        return webservice.app

    def testIndex(self):
        '''basic test of requesting the index of the webservice'''

        baseurl = 'http://localhost:5000'
        paths = ['/', '/index.htm', '/index.html']
        for path in paths:
            response = self.client.get(baseurl + path)
            self.assertEqual(200, response.status_code)

        # also test a non-existent url, should give 404
        self.assertEqual(404, self.client.get(baseurl + '/hdaHJSK/fsfaAFdsaf.gwg').status_code)


class TestLiveResourceAssignementEditor(FlaskLiveTestCase):
    '''Test the live ResourceAssignementEditor web service'''

    def create_app(self):
        # override create_app method from FlaskLiveTestCase
        # this provides the FlaskLiveTestCase with our webservice to test.
        self.port = 63124
        webservice.app.debug = False
        webservice.app.use_debugger = False
        webservice.app.use_reloader = False
        webservice.app.config['TESTING'] = True
        webservice.app.config['LIVESERVER_PORT'] = self.port
        return webservice.app

    def testIndex(self):
        '''basic test of requesting the index of the webservice'''
        baseurl = 'http://localhost:%d' % (self.port)
        paths = ['/', '/index.htm', '/index.html']
        for path in paths:
            response = urllib2.urlopen(baseurl + path)
            self.assertEqual(200, response.code)

        # also test a non-existent url, should give 404
        with self.assertRaises(urllib2.HTTPError):
            self.assertEqual(404, urllib2.urlopen(baseurl + '/hdaHJSK/fsfaAFdsaf.gwg').code)


def main(argv):
    unittest.main(verbosity=2)

# run all tests if main
if __name__ == '__main__':
    main(sys.argv[1:])
