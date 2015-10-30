#!/usr/bin/python

# Copyright (C) 2012-2015  ASTRON (Netherlands Institute for Radio Astronomy)
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.

# $Id$

'''ResourceAssignementEditor webservice serves a interactive html5 website for
viewing and editing lofar resources.'''

import sys
import os
import time
from datetime import datetime
from datetime import timedelta
from flask import Flask
from flask import render_template
from flask import url_for
from flask.json import jsonify
from resourceassignementeditor.utils import gzipped

__root_path = os.path.dirname(os.path.abspath(__file__))
print '__root_path=%s' % __root_path

'''The flask webservice app'''
app = Flask('ResourceAssignementEditor',
            instance_path=__root_path,
            template_folder=os.path.join(__root_path, 'templates'),
            static_folder=os.path.join(__root_path, 'static'),
            instance_relative_config=True)

print 'app.template_folder= %s' % app.template_folder
print 'app.static_folder= %s' % app.static_folder

# Load the default configuration
app.config.from_object('resourceassignementeditor.config.default')

@app.route('/')
@app.route('/index.htm')
@app.route('/index.html')
def index():
    '''Serves the ResourceAssignementEditor's index page'''
    return render_template('index.html', title='Resource Assignement Editor')

import random

values = [{'name': str(random.randint(0, 1000)), 'city': str(random.randint(0, 1000))} for x in range(5000)]

@app.route('/rest/data.json')
@gzipped
def data():
    data = {'data': values}
    return jsonify(data)

@app.route('/rest/resourceitems')
@gzipped
def resourcesitems():
    data = {'resourceitems': [{'id': 0, 'name': 'CS001', 'typeId': 0, 'type': 'station', 'group': False},
                              {'id': 1, 'name': 'CS002', 'typeId': 0, 'type': 'station', 'group': False},
                              {'id': 2, 'name': 'CS003', 'typeId': 0, 'type': 'station', 'group': False},
                              {'id': 3, 'name': 'CS004', 'typeId': 0, 'type': 'station', 'group': False},
                              {'id': 4, 'name': 'Core', 'typeId': 1, 'type': 'stationset', 'group': True},
                              {'id': 5, 'name': 'Node1', 'typeId': 2, 'type': 'node', 'group': False},
                              {'id': 6, 'name': 'Node2', 'typeId': 2, 'type': 'node', 'group': False}
                              ]}
    return jsonify(data)

@app.route('/rest/resourceclaims')
@gzipped
def resourceclaims():
    data = {'resourceclaims': [{'id': 0, 'resourceId': 4, 'taskId': 0, 'startTime': '2015-10-28T14:14:00Z', 'endTime': '2015-10-28T17:00:00Z', 'status': 'allocated'},
                               {'id': 1, 'resourceId': 4, 'taskId': 1, 'startTime': '2015-10-29T10:00:00Z', 'endTime': '2015-10-29T12:00:00Z', 'status': 'claimed'},
                               {'id': 2, 'resourceId': 4, 'taskId': 2, 'startTime': '2015-10-29T12:15:00Z', 'endTime': '2015-10-29T18:00:00Z', 'status': 'claimed'},
                               ]}
    return jsonify(data)

@app.route('/rest/tasks')
@gzipped
def tasks():
    data = {'name': 'Observations', 'tasks': [{'id': 0, 'momId': 123, 'obsId': 876, 'status': 'scheduled', 'name': 'Lobos Obs 2a', 'from': datetime.utcnow() - timedelta(hours=1), 'to': datetime.utcnow() + timedelta(hours=1)},
                      {'id': 1, 'momId': 345, 'obsId': 654, 'status': 'approved', 'name': 'LOTAAS Obs 32q', 'from': datetime.utcnow() + timedelta(hours=5), 'to': datetime.utcnow() + timedelta(hours=6)},
                      {'id': 2, 'momId': 567, 'obsId': 432, 'status': 'approved', 'name': 'Pulsar Obs 3', 'from': datetime.utcnow() + timedelta(hours=10), 'to': datetime.utcnow() + timedelta(hours=32)}
                    ] }
    return jsonify(data)

def main(argv=None, debug=False):
    '''Start the webserver'''
    app.run(debug=debug, threaded=True, port=5001)

if __name__ == '__main__':
    main(sys.argv[1:], True)
