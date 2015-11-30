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

'''ResourceAssignmentEditor webservice serves a interactive html5 website for
viewing and editing lofar resources.'''

import sys
import os
import json
import time
from collections import deque
from threading import Condition
from datetime import datetime
from dateutil import parser
from flask import Flask
from flask import render_template
from flask import request
from flask import abort
from flask import url_for
from flask.json import jsonify
from lofar.sas.resourceassignment.resourceassignmenteditor.utils import gzipped
from lofar.sas.resourceassignment.resourceassignmenteditor.fakedata import *

__root_path = os.path.dirname(os.path.abspath(__file__))
print '__root_path=%s' % __root_path

'''The flask webservice app'''
app = Flask('ResourceAssignmentEditor',
            instance_path=__root_path,
            template_folder=os.path.join(__root_path, 'templates'),
            static_folder=os.path.join(__root_path, 'static'),
            instance_relative_config=True)

print 'app.template_folder= %s' % app.template_folder
print 'app.static_folder= %s' % app.static_folder

# Load the default configuration
app.config.from_object('lofar.sas.resourceassignment.resourceassignmenteditor.config.default')

@app.route('/')
@app.route('/index.htm')
@app.route('/index.html')
def index():
    '''Serves the ResourceAssignmentEditor's index page'''
    return render_template('index.html', title='Resource Assignment Editor')

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

_changes = []
changedCondition = Condition()

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

            taskResourceClaims = [rc for rc in resourceClaims if rc['taskId'] == task_id]

            for rc in taskResourceClaims:
                rc['startTime'] = task['from']
                rc['endTime'] = task['to']

            taskResourceGroupClaims = [rgc for rgc in resourceGroupClaims if rgc['taskId'] == task_id]

            for rgc in taskResourceGroupClaims:
                rgc['startTime'] = task['from']
                rgc['endTime'] = task['to']


            with changedCondition:
                now = datetime.utcnow()

                # remove old obsolete changes
                threshold = (now - timedelta(seconds=10)).isoformat()
                global _changes
                _changes = [c for c in _changes if c['timestamp'] >= threshold]

                now = now.isoformat()

                _changes.append({'changeType': 'update',
                                'timestamp': now,
                                'objectType': 'task',
                                'value': task
                                })

                for rc in taskResourceClaims:
                    _changes.append({'changeType': 'update',
                                    'timestamp': now,
                                    'objectType': 'resourceClaim',
                                    'value': rc
                                    })

                for rgc in taskResourceGroupClaims:
                    _changes.append({'changeType': 'update',
                                    'timestamp': now,
                                    'objectType': 'resourceGroupClaim',
                                    'value': rgc
                                    })

                changedCondition.notifyAll()

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


@app.route('/rest/updates/<since>')
def getUpdateEventsSince(since):
    with changedCondition:
        while True:
            changesSince = [c for c in _changes if c['timestamp'] > since]

            if changesSince:
                return jsonify({'changes': changesSince})

            changedCondition.wait()

    return jsonify({'changes': []})

@app.route('/rest/updates')
def getUpdateEvents():
    return getUpdateEventsSince(datetime.utcnow().isoformat())

def main(argv=None, debug=False):
    '''Start the webserver'''
    app.run(debug=debug, threaded=True, host='0.0.0.0', port=5001)

if __name__ == '__main__':
    main(sys.argv[1:], True)
