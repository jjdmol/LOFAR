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
from optparse import OptionParser
from threading import Condition
from datetime import datetime
import time
import logging
from dateutil import parser, tz
from flask import Flask
from flask import render_template
from flask import request
from flask import abort
from flask import url_for
from flask.json import jsonify
from flask.json import JSONEncoder
from lofar.sas.resourceassignment.resourceassignmenteditor.utils import gzipped
from lofar.sas.resourceassignment.resourceassignmenteditor.fakedata import *
from lofar.sas.resourceassignment.resourceassignmenteditor.radbchangeshandler import RADBChangesHandler, CHANGE_DELETE_TYPE
from lofar.sas.resourceassignment.database.config import DEFAULT_BUSNAME as DEFAULT_RADB_CHANGES_BUSNAME
from lofar.sas.resourceassignment.resourceassignmentservice.rpc import RARPC
from lofar.sas.resourceassignment.resourceassignmentservice.config import DEFAULT_BUSNAME, DEFAULT_SERVICENAME
from lofar.mom.momqueryservice.momqueryrpc import MoMRPC
from lofar.mom.momqueryservice.config import DEFAULT_BUSNAME as DEFAULT_MOM_BUSNAME
from lofar.mom.momqueryservice.config import DEFAULT_SERVICENAME as DEFAULT_MOM_SERVICENAME
from lofar.sas.resourceassignment.resourceassignmenteditor.mom import updateTaskMomDetails
#from lofar.sas.resourceassignment.resourceassigner. import updateTaskMomDetails

logger = logging.getLogger(__name__)

def asDatetime(isoString):
    if isoString[-1] == 'Z':
        isoString = isoString[:-1]
    if isoString[-4] == '.':
        isoString += '000'
    return datetime.strptime(isoString, '%Y-%m-%dT%H:%M:%S.%f')

def asIsoFormat(timestamp):
    return datetime.strftime(timestamp, '%Y-%m-%dT%H:%M:%S.%fZ')


class CustomJSONEncoder(JSONEncoder):
    def default(self, obj):
        try:
            if isinstance(obj, datetime):
                return asIsoFormat(obj)
            iterable = iter(obj)
        except TypeError:
            pass
        else:
            return list(iterable)
        return JSONEncoder.default(self, obj)


__root_path = os.path.dirname(os.path.realpath(__file__))

'''The flask webservice app'''
app = Flask('ResourceAssignmentEditor',
            instance_path=__root_path,
            template_folder=os.path.join(__root_path, 'templates'),
            static_folder=os.path.join(__root_path, 'static'),
            instance_relative_config=True)

# Load the default configuration
app.config.from_object('lofar.sas.resourceassignment.resourceassignmenteditor.config.default')
app.json_encoder = CustomJSONEncoder

rarpc = RARPC(busname=DEFAULT_BUSNAME, servicename=DEFAULT_SERVICENAME, broker='10.149.96.6')
momrpc = MoMRPC(busname=DEFAULT_MOM_BUSNAME, servicename=DEFAULT_MOM_SERVICENAME, timeout=2, broker='10.149.96.6')
radbchangeshandler = RADBChangesHandler(DEFAULT_RADB_CHANGES_BUSNAME, broker='10.149.96.6', momrpc=momrpc)

@app.route('/')
@app.route('/index.htm')
@app.route('/index.html')
def index():
    '''Serves the ResourceAssignmentEditor's index page'''
    return render_template('index.html', title='Resource Assignment Editor')

@app.route('/rest/resources')
@gzipped
def resourcesitems():
    result = rarpc.getResources()
    return jsonify({'resources': result})

@app.route('/rest/resourcegroups')
@gzipped
def resourcegroups():
    result = rarpc.getResourceGroups()
    return jsonify({'resourcegroups': result})

@app.route('/rest/resourcegroupmemberships')
@gzipped
def resourcegroupsmemberships():
    result = rarpc.getResourceGroupMemberships()
    return jsonify({'resourcegroupmemberships': result})

@app.route('/rest/resourceclaims')
@gzipped
def resourceclaims():
    claims = rarpc.getResourceClaims()
    return jsonify({'resourceclaims': claims})

@app.route('/rest/tasks')
@gzipped
def getTasks():
    tasks = rarpc.getTasks()

    # there are no task names in the database yet.
    # will they come from spec/MoM?
    # add Task <id> as name for now
    updateTaskMomDetails(tasks, momrpc)

    return jsonify({'tasks': tasks})

@app.route('/rest/tasks/<int:task_id>', methods=['GET'])
def getTask(task_id):
    try:
        task = rarpc.getTask(task_id)

        if not task:
            abort(404)

        task['name'] = 'Task %d' % task['id']
        updateTaskMomDetails(task, momrpc)
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

            logger.info('putTask: ' + str(updatedTask))
            rarpc.updateTaskAndResourceClaims(task_id,
                                            starttime=updatedTask.get('starttime', None),
                                            endtime=updatedTask.get('endtime', None),
                                            task_status=updatedTask.get('task_status', None),
                                            claim_status=updatedTask.get('claim_status', None))

            return "", 204
        except KeyError:
            abort(404)
    abort(406)

@app.route('/rest/tasks/<int:task_id>/resourceclaims')
def taskResourceClaims(task_id):
    return jsonify({'taskResourceClaims': [x for x in resourceClaims if x['taskId'] == task_id]})

@app.route('/rest/tasktypes')
def tasktypes():
    result = rarpc.getTaskTypes()
    result = [x for x in result]
    return jsonify({'tasktypes': result})

@app.route('/rest/taskstatustypes')
def getTaskStatusTypes():
    result = rarpc.getTaskStatuses()
    result = sorted(result, key=lambda q: q['id'])
    result = [x for x in result]
    return jsonify({'taskstatustypes': result})

@app.route('/rest/momprojects')
def getMoMProjects():
    projects = []
    try:
        projects = momrpc.getProjects()
        projects = [x for x in projects if x['status_id'] in [1, 7]]
        for project in projects:
            project['mom_id'] = project.pop('mom2id')
    except Exception as e:
        logger.error(e)
        projects.append({'name':'<unknown>', 'mom_id':-99, 'description': 'Container project for tasks for which we could not find a MoM project'})

    projects.append({'name':'OTDB Only', 'mom_id':-98, 'description': 'Container project for tasks which exists only in OTDB'})
    return jsonify({'momprojects': projects})

@app.route('/rest/momobjectdetails/<int:mom2id>')
def getMoMObjectDetails(mom2id):
    details = momrpc.getProjectDetails(mom2id)
    details = details.values()[0] if details else None
    if details:
        details['project_mom_id'] = details.pop('project_mom2id')
        details['object_mom_id'] = details.pop('object_mom2id')

    return jsonify({'momobjectdetails': details})

@app.route('/rest/updates/<int:sinceChangeNumber>')
def getUpdateEventsSince(sinceChangeNumber):
    changesSince = radbchangeshandler.getChangesSince(sinceChangeNumber)
    return jsonify({'changes': changesSince})

@app.route('/rest/updates')
def getUpdateEvents():
    return getUpdateEventsSince(-1L)

def main():
    # Check the invocation arguments
    parser = OptionParser('%prog [options]',
                          description='run the resource assignment editor web service')
    parser.add_option('-p', '--port', dest='port', type='int', default=5000, help='port number on which to host the webservice, default: 5000')
    parser.add_option('-q', '--broker', dest='broker', type='string', default=None, help='Address of the qpid broker, default: localhost')
    parser.add_option('--radb_busname', dest='radb_busname', type='string', default=DEFAULT_BUSNAME, help='Name of the bus exchange on the qpid broker on which the radbservice listens, default: %s' % DEFAULT_BUSNAME)
    parser.add_option('--radb_servicename', dest='radb_servicename', type='string', default=DEFAULT_SERVICENAME, help='Name of the radbservice, default: %s' % DEFAULT_SERVICENAME)
    parser.add_option('--radb_notifications_busname', dest='radb_notifications_busname', type='string', default=DEFAULT_RADB_CHANGES_BUSNAME, help='Name of the notification bus exchange on the qpid broker on which the radb notifications are published, default: %s' % DEFAULT_RADB_CHANGES_BUSNAME)
    parser.add_option('--mom_busname', dest='mom_busname', type='string', default=DEFAULT_MOM_BUSNAME, help='Name of the bus exchange on the qpid broker on which the momservice listens, default: %s' % DEFAULT_MOM_BUSNAME)
    parser.add_option('--mom_servicename', dest='mom_servicename', type='string', default=DEFAULT_MOM_SERVICENAME, help='Name of the momservice, default: %s' % DEFAULT_MOM_SERVICENAME)
    parser.add_option('-V', '--verbose', dest='verbose', action='store_true', help='verbose logging')
    (options, args) = parser.parse_args()

    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s',
                        level=logging.DEBUG if options.verbose else logging.INFO)

    with radbchangeshandler, rarpc, momrpc:
        '''Start the webserver'''
        app.run(debug=options.verbose, threaded=True, host='0.0.0.0', port=options.port)

if __name__ == '__main__':
    main()
