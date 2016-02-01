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
import time
from dateutil import parser, tz
from flask import Flask
from flask import render_template
from flask import request
from flask import abort
from flask import url_for
from flask.json import jsonify
from lofar.sas.resourceassignment.resourceassignmenteditor.utils import gzipped
from lofar.sas.resourceassignment.resourceassignmenteditor.fakedata import *
from lofar.sas.resourceassignment.resourceassignmenteditor.radbchangeshandler import RADBChangesHandler, CHANGE_DELETE_TYPE
from lofar.sas.resourceassignment.database.config import DEFAULT_BUSNAME as DEFAULT_RADB_CHANGES_BUSNAME
from lofar.sas.resourceassignment.resourceassignmentservice.rpc import RARPC
from lofar.sas.resourceassignment.resourceassignmentservice.config import DEFAULT_BUSNAME, DEFAULT_SERVICENAME


def asDatetime(isoString):
    if isoString[-1] == 'Z':
        isoString = isoString[:-1]
    if isoString[-4] == '.':
        isoString += '000'
    return datetime.strptime(isoString, '%Y-%m-%dT%H:%M:%S.%f')

__root_path = os.path.dirname(os.path.abspath(__file__))

'''The flask webservice app'''
app = Flask('ResourceAssignmentEditor',
            instance_path=__root_path,
            template_folder=os.path.join(__root_path, 'templates'),
            static_folder=os.path.join(__root_path, 'static'),
            instance_relative_config=True)

# Load the default configuration
app.config.from_object('lofar.sas.resourceassignment.resourceassignmenteditor.config.default')

rpc = RARPC(busname=DEFAULT_BUSNAME, servicename=DEFAULT_SERVICENAME, broker='10.149.96.6')
radbchangeshandler = RADBChangesHandler(DEFAULT_RADB_CHANGES_BUSNAME, broker='10.149.96.6')

@app.route('/')
@app.route('/index.htm')
@app.route('/index.html')
def index():
    '''Serves the ResourceAssignmentEditor's index page'''
    return render_template('index.html', title='Resource Assignment Editor')

@app.route('/rest/resourceitems')
@gzipped
def resourcesitems():
    result = rpc.getResources()
    return jsonify({'resourceitems': result})

@app.route('/rest/resourcegroups')
@gzipped
def resourcegroups():
    result = rpc.getResourceGroups()
    return jsonify({'resourcegroups': result})

@app.route('/rest/resourceclaims')
@gzipped
def resourceclaims():
    result = rpc.getResourceClaims()
    return jsonify({'resourceclaims': result})

@app.route('/rest/resourcegroupclaims')
@gzipped
def resourcegroupclaims():
    abort(500)


@app.route('/rest/tasks')
@gzipped
def getTasks():
    tasks = rpc.getTasks()

    # there are no task names in the database yet.
    # will they come from spec/MoM?
    # add Task <id> as name for now
    for task in tasks:
        task['name'] = 'Task %d' % task['id']

    return jsonify({'tasks': tasks})

@app.route('/rest/tasks/<int:task_id>', methods=['GET'])
def getTask(task_id):
    try:
        task = rpc.getTask(task_id)

        if not task:
            abort(404)

        task['name'] = 'Task %d' % task['id']
        return jsonify({'task': task})
    except Exception as e:
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

            if 'starttime' in updatedTask:
                try:
                    updatedTask['starttime'] = asDatetime(updatedTask['starttime'])
                except ValueError:
                    abort(400, 'timestamp not in iso format: ' + updatedTask['starttime'])

            if 'endtime' in updatedTask:
                try:
                    updatedTask['endtime'] = asDatetime(updatedTask['endtime'])
                except ValueError:
                    abort(400, 'timestamp not in iso format: ' + updatedTask['endtime'])

            rpc.updateResourceClaimsForTask(task_id,
                                            starttime=updatedTask.get('starttime', None),
                                            endtime=updatedTask.get('endtime', None),
                                            status=updatedTask.get('status', None))

            return "", 204
        except KeyError:
            abort(404)
    abort(406)

@app.route('/rest/tasks/<int:task_id>/resourceclaims')
def taskResourceClaims(task_id):
    return jsonify({'taskResourceClaims': [x for x in resourceClaims if x['taskId'] == task_id]})

@app.route('/rest/tasktypes')
def tasktypes():
    result = rpc.getTaskTypes()
    result = [x['name'] for x in result]
    return jsonify({'tasktypes': result})

@app.route('/rest/taskstatustypes')
def getTaskStatusTypes():
    result = rpc.getTaskStatuses()
    result = sorted(result, key=lambda q: q['id'])
    result = [x['name'] for x in result]
    return jsonify({'taskstatustypes': result})


@app.route('/rest/updates/<since>')
def getUpdateEventsSince(since):
    try:
        since = asDatetime(since)
    except ValueError:
        abort(400, 'timestamp not in iso format: ' + since)

    changesSince = radbchangeshandler.getChangesSince(since)
    return jsonify({'changes': changesSince})

@app.route('/rest/updates')
def getUpdateEvents():
    return getUpdateEventsSince(datetime.utcnow().isoformat())

def main(argv=None, debug=False):
    with radbchangeshandler:
        '''Start the webserver'''
        app.run(debug=debug, threaded=True, host='0.0.0.0', port=5001)

if __name__ == '__main__':
    main(sys.argv[1:], True)
