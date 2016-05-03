#!/usr/bin/env python

import unittest
import sys
import time
import urllib2
from threading import Thread
from datetime import datetime
try:
    from flask import Flask
except ImportError:
    print 'please install flask package: pip install Flask'
    exit(3)
try:
    from flask.ext.testing import TestCase as FlaskTestCase
    from flask.ext.testing import LiveServerTestCase as FlaskLiveTestCase
except ImportError:
    print 'please install flask testing package: pip install Flask-Testing'
    exit(3)
from lofar.sas.resourceassignment.resourceassignmenteditor import webservice


def setUpModule():
    # here we can setup stuff we need once per module testing
    pass


def tearDownModule():
    # here we can tear down stuff we need once per module testing
    pass


class TestResourceAssignmentEditor(FlaskTestCase):
    '''Test the logic in the ResourceAssignmentEditor web service'''

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


class TestLiveResourceAssignmentEditor(FlaskLiveTestCase):
    '''Test the live ResourceAssignmentEditor web service'''

    def create_app(self):
        # override create_app method from FlaskLiveTestCase
        # this provides the FlaskLiveTestCase with our webservice to test.
        self.port = 63123
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

    #TODO: make testUpdatesSince working
    #def testUpdatesSince(self):
        #'''testUpdatesSince
        #'''
        #baseurl = 'http://localhost:%d' % (self.port)

        #def putTask():
            #print "putTask"
            #time.sleep(1)
            #print "putTask 2"
            #task = "{'id': 1, 'from': %s}" % datetime.utcnow().isoformat()
            #opener = urllib2.build_opener(urllib2.HTTPHandler)
            #request = urllib2.Request(baseurl + '/rest/tasks/1', data=task)
            #request.add_header('Content-Type', 'application/json')
            #request.get_method = lambda: 'PUT'
            #print "putTask send request"
            #response = opener.open(request)
            #print "putTask has response"
            #self.assertEqual(200, response.code)

        #def listenForUpdates():
            #print "putTask"
            #response = urllib2.urlopen(baseurl + '/rest/updates')
            #self.assertEqual(200, response.code)

        #t1 = Thread(target=putTask)
        #t1.start()

        #t2 = Thread(target=listenForUpdates)
        #t2.start()

        #t1.join()
        #t2.join()



def main(argv):
    unittest.main(verbosity=2)

# run all tests if main
if __name__ == '__main__':
    main(sys.argv[1:])
