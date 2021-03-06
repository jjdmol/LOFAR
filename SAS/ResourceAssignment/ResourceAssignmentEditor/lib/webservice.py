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
from lofar.sas.resourceassignment.database.config import DEFAULT_NOTIFICATION_BUSNAME as DEFAULT_RADB_CHANGES_BUSNAME
from lofar.sas.resourceassignment.database.config import DEFAULT_NOTIFICATION_SUBJECTS as DEFAULT_RADB_CHANGES_SUBJECTS
from lofar.sas.resourceassignment.resourceassignmentservice.rpc import RARPC
from lofar.sas.resourceassignment.resourceassignmentservice.config import DEFAULT_BUSNAME as DEFAULT_RADB_BUSNAME
from lofar.sas.resourceassignment.resourceassignmentservice.config import DEFAULT_SERVICENAME as DEFAULT_RADB_SERVICENAME
from lofar.mom.momqueryservice.momqueryrpc import MoMQueryRPC
from lofar.mom.momqueryservice.config import DEFAULT_MOMQUERY_BUSNAME, DEFAULT_MOMQUERY_SERVICENAME
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
app = Flask('Scheduler',
            instance_path=__root_path,
            template_folder=os.path.join(__root_path, 'templates'),
            static_folder=os.path.join(__root_path, 'static'),
            instance_relative_config=True)

# Load the default configuration
app.config.from_object('lofar.sas.resourceassignment.resourceassignmenteditor.config.default')
app.json_encoder = CustomJSONEncoder

rarpc = None
momrpc = None
radbchangeshandler = None

@app.route('/')
@app.route('/index.htm')
@app.route('/index.html')
def index():
    '''Serves the ResourceAssignmentEditor's index page'''
    return render_template('index.html', title='Scheduler')

@app.route('/rest/resources')
@gzipped
def resources():
    result = rarpc.getResources(include_availability=True)
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
def resourceclaims():
    return resourceclaimsFromUntil(None, None)

@app.route('/rest/resourceclaims/<string:fromTimestamp>')
def resourceclaimsFrom(fromTimestamp=None):
    return resourceclaimsFromUntil(fromTimestamp, None)

@app.route('/rest/resourceclaims/<string:fromTimestamp>/<string:untilTimestamp>')
@gzipped
def resourceclaimsFromUntil(fromTimestamp=None, untilTimestamp=None):
    if fromTimestamp and isinstance(fromTimestamp, basestring):
        fromTimestamp = asDatetime(fromTimestamp)

    if untilTimestamp and isinstance(untilTimestamp, basestring):
        untilTimestamp = asDatetime(untilTimestamp)

    claims = rarpc.getResourceClaims(lower_bound=fromTimestamp, upper_bound=untilTimestamp, include_properties=True)
    return jsonify({'resourceclaims': claims})

@app.route('/rest/resourceusages')
@gzipped
def resourceUsages():
    result = rarpc.getResourceUsages()
    return jsonify({'resourceusages': result})

@app.route('/rest/resources/<int:resource_id>/usages', methods=['GET'])
@app.route('/rest/resourceusages/<int:resource_id>', methods=['GET'])
@gzipped
def resourceUsagesForResource(resource_id):
    result = rarpc.getResourceUsages(resource_ids=[resource_id])
    return jsonify({'resourceusages': result})

@app.route('/rest/tasks/<int:task_id>/resourceusages', methods=['GET'])
@gzipped
def resourceUsagesForTask(task_id):
    result = rarpc.getResourceUsages(task_ids=[task_id])
    return jsonify({'resourceusages': result})

@app.route('/rest/tasks')
def getTasks():
    return getTasksFromUntil(None, None)

@app.route('/rest/tasks/<string:fromTimestamp>')
def getTasksFrom(fromTimestamp):
    return getTasksFromUntil(fromTimestamp, None)

@app.route('/rest/tasks/<string:fromTimestamp>/<string:untilTimestamp>')
@gzipped
def getTasksFromUntil(fromTimestamp=None, untilTimestamp=None):
    if fromTimestamp and isinstance(fromTimestamp, basestring):
        fromTimestamp = asDatetime(fromTimestamp)

    if untilTimestamp and isinstance(untilTimestamp, basestring):
        untilTimestamp = asDatetime(untilTimestamp)

    tasks = rarpc.getTasks(fromTimestamp, untilTimestamp)

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
    abort(403, 'Editing of tasks is by users is not yet approved')

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
                                              task_status=updatedTask.get('status', None))

            return "", 204
        except KeyError:
            abort(404)
    abort(406)

@app.route('/rest/tasks/<int:task_id>/resourceclaims')
def taskResourceClaims(task_id):
    return jsonify({'taskResourceClaims': rarpc.getResourceClaims(task_id=task_id, include_properties=True)})

@app.route('/rest/tasktypes')
def tasktypes():
    result = rarpc.getTaskTypes()
    result = sorted(result, key=lambda q: q['id'])
    return jsonify({'tasktypes': result})

@app.route('/rest/taskstatustypes')
def getTaskStatusTypes():
    result = rarpc.getTaskStatuses()
    result = sorted(result, key=lambda q: q['id'])
    return jsonify({'taskstatustypes': result})

@app.route('/rest/resourcetypes')
def resourcetypes():
    result = rarpc.getResourceTypes()
    result = sorted(result, key=lambda q: q['id'])
    return jsonify({'resourcetypes': result})

@app.route('/rest/resourceclaimpropertytypes')
def resourceclaimpropertytypes():
    result = rarpc.getResourceClaimPropertyTypes()
    result = sorted(result, key=lambda q: q['id'])
    return jsonify({'resourceclaimpropertytypes': result})

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

@app.route('/rest/mostRecentChangeNumber')
def getMostRecentChangeNumber():
    mrcn = radbchangeshandler.getMostRecentChangeNumber()
    return jsonify({'mostRecentChangeNumber': mrcn})

@app.route('/rest/updates')
def getUpdateEvents():
    return getUpdateEventsSince(-1L)

@app.route('/rest/lofarTime')
def getLofarTime():
    return jsonify({'lofarTime': asIsoFormat(datetime.utcnow())})

def main():
    # make sure we run in UTC timezone
    import os
    os.environ['TZ'] = 'UTC'

    # Check the invocation arguments
    parser = OptionParser('%prog [options]',
                          description='run the resource assignment editor web service')
    parser.add_option('-p', '--port', dest='port', type='int', default=5000, help='port number on which to host the webservice, default: %default')
    parser.add_option('-q', '--broker', dest='broker', type='string', default=None, help='Address of the qpid broker, default: localhost')
    parser.add_option('--radb_busname', dest='radb_busname', type='string', default=DEFAULT_RADB_BUSNAME, help='Name of the bus exchange on the qpid broker on which the radbservice listens, default: %default')
    parser.add_option('--radb_servicename', dest='radb_servicename', type='string', default=DEFAULT_RADB_SERVICENAME, help='Name of the radbservice, default: %default')
    parser.add_option('--radb_notification_busname', dest='radb_notification_busname', type='string', default=DEFAULT_RADB_CHANGES_BUSNAME, help='Name of the notification bus exchange on the qpid broker on which the radb notifications are published, default: %default')
    parser.add_option('--radb_notification_subjects', dest='radb_notification_subjects', type='string', default=DEFAULT_RADB_CHANGES_SUBJECTS, help='Subject(s) to listen for on the radb notification bus exchange on the qpid broker, default: %default')
    parser.add_option('--mom_busname', dest='mom_busname', type='string', default=DEFAULT_MOMQUERY_BUSNAME, help='Name of the bus exchange on the qpid broker on which the momservice listens, default: %default')
    parser.add_option('--mom_servicename', dest='mom_servicename', type='string', default=DEFAULT_MOMQUERY_SERVICENAME, help='Name of the momservice, default: %default')
    parser.add_option('-V', '--verbose', dest='verbose', action='store_true', help='verbose logging')
    (options, args) = parser.parse_args()

    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s',
                        level=logging.DEBUG if options.verbose else logging.INFO)

    global rarpc
    rarpc = RARPC(busname=DEFAULT_RADB_BUSNAME, servicename=DEFAULT_RADB_SERVICENAME, broker=options.broker)
    global momrpc
    momrpc = MoMQueryRPC(busname=DEFAULT_MOMQUERY_BUSNAME, servicename=DEFAULT_MOMQUERY_SERVICENAME, timeout=2.5, broker=options.broker)
    global radbchangeshandler
    radbchangeshandler = RADBChangesHandler(DEFAULT_RADB_CHANGES_BUSNAME, broker=options.broker, momrpc=momrpc)

    with radbchangeshandler, rarpc, momrpc:
        '''Start the webserver'''
        app.run(debug=options.verbose, threaded=True, host='0.0.0.0', port=options.port)

if __name__ == '__main__':
    main()
