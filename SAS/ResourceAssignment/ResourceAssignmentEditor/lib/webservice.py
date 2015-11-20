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
import json
import time
from datetime import datetime
from dateutil import parser
from flask import Flask
from flask import render_template
from flask import request
from flask import abort
from flask import url_for
from flask.json import jsonify
from lofar.sas.resourceassignement.resourceassignementeditor.utils import gzipped
from lofar.sas.resourceassignement.resourceassignementeditor.fakedata import *

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
app.config.from_object('lofar.sas.resourceassignement.resourceassignementeditor.config.default')

@app.route('/')
@app.route('/index.htm')
@app.route('/index.html')
def index():
    '''Serves the ResourceAssignementEditor's index page'''
    return render_template('index.html', title='Resource Assignement Editor')

@app.route('/rest/resourceitems')
@gzipped
def resourcesitems():
    data = {'resourceitems': resourceItems}
    return jsonify(data)

@app.route('/rest/resourcegroups')
@gzipped
def resourcegroups():
    data = {'resourcegroups': resourceGroups}
    return jsonify(data)

@app.route('/rest/resourceclaims')
@gzipped
def resourceclaims():
    data = {'resourceclaims': resourceClaims}
    return jsonify(data)

@app.route('/rest/resourcegroupclaims')
@gzipped
def resourcegroupclaims():
    data = {'resourcegroupclaims': resourceGroupClaims}

    return jsonify(data)

@app.route('/rest/tasks')
@gzipped
def getTasks():
    data = {'tasks': tasks.values()}

    return jsonify(data)

@app.route('/rest/tasks/<int:task_id>', methods=['GET'])
def getTask(task_id):
    try:
        task = tasks[task_id]
        return jsonify({'task': task})
    except KeyError:
        abort(404)

    return jsonify({'task': None})

@app.route('/rest/tasks/<int:task_id>', methods=['PUT'])
def putTask(task_id):
    if 'Content-Type' in request.headers and \
            request.headers['Content-Type'].startswith('application/json'):
        updatedTask = json.loads(request.data)

        try:
            if task_id != updatedTask['id']:
                abort(404)

            task = tasks[task_id]

            if 'from' in updatedTask:
                task['from'] = parser.parse(updatedTask['from'])

            if 'to' in updatedTask:
                task['to'] = parser.parse(updatedTask['to'])

            return "", 204
        except KeyError:
            abort(404)
    abort(406)

@app.route('/rest/tasks/<int:task_id>/resourceclaims')
def taskResourceClaims(task_id):
    return jsonify({'taskResourceClaims': [x for x in resourceClaims if x['taskId'] == task_id]})

@app.route('/rest/tasktypes')
def tasktypes():
    return jsonify({'tasktypes': ['Observation', 'Pipeline', 'Ingest']})

@app.route('/rest/taskstatustypes')
def taskstatustypes():
    return jsonify({'taskstatustypes': ['scheduled', 'approved', 'prescheduled', 'running', 'finished', 'aborted']})

import random

@app.route('/rest/updates')
def getUpdateEvents():
    # fake blocking while waiting for an event
    time.sleep(random.randint(50, 5000)/1000.0)

    # fake a changed event
    # TODO: locking
    # TODO: events should be stored in a buffer so we can give updates since a given timestamp
    tasks[1]['from'] = datetime.utcnow()

    return "There are updates!"

def main(argv=None, debug=False):
    '''Start the webserver'''
    app.run(debug=debug, threaded=True, host='0.0.0.0', port=5001)

if __name__ == '__main__':
    main(sys.argv[1:], True)
