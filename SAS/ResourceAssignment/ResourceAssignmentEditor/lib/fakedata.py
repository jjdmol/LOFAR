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

# $Id: webservice.py 32739 2015-10-30 15:21:48Z schaap $

from datetime import datetime
from datetime import timedelta

numProjects = 3
numObsPerProject = 2
numPipelinesPerObs = 2
numCoreStations = 10
numRemoteStations = 5
numCobaltNodes = 2
numComputeNodes = 2
numIngestNodes = 2

_taskIdCntr = 0

def _genTask(name, startTime, duration, status = 'scheduled', type = 'Observation'):
    global _taskIdCntr
    _taskIdCntr = _taskIdCntr + 1
    return {'id': _taskIdCntr,
            'momId': 123 + _taskIdCntr,
            'obsId': 876 + _taskIdCntr,
            'status': status,
            'name': name,
            'from': startTime,
            'to': startTime + duration,
            'type': type}


tasks = {}
now = datetime.utcnow()

for p in range(numProjects):
    for i in range(numObsPerProject):
        task = _genTask('LC4_%03d Obs %02d'% (p+1, i+1), now + timedelta(hours=(p+1)*(i+1)*4.05), timedelta(hours=4))
        tasks[task['id']] = task

        for j in range(numPipelinesPerObs):
            task = _genTask('LC4_%03d Pipeline %02d'% (p+1, j+1), now + timedelta(hours=(p+1)*(i+1)*4.05 + 4.05 + (j)*4.05), timedelta(hours=4), type='Pipeline')
            tasks[task['id']] = task

        task = _genTask('LC4_%03d Ingest %02d'% (p+1, j+1), now + timedelta(hours=(p+1)*(i+1)*4.05 + 4.05 + (numPipelinesPerObs)*4.05), timedelta(hours=2), type='Ingest')
        tasks[task['id']] = task

resourceGroups = []
coreStationsGroup = {'id': 0, 'name': 'Core Stations', 'childGroupIds': [], 'resourceIds': []}
remoteStationsGroup = {'id': 1, 'name': 'Remote Stations', 'childGroupIds': [], 'resourceIds': []}
stationsGroup = {'id': 2, 'name': 'Stations', 'childGroupIds': [coreStationsGroup['id'], remoteStationsGroup['id']], 'resourceIds': []}
cobaltGroup = {'id': 3, 'name': 'Cobalt', 'childGroupIds': [], 'resourceIds': []}
cep4NodeGroup = {'id': 4, 'name': 'CEP4 Nodes', 'childGroupIds': [], 'resourceIds': []}
cep4StorageGroup = {'id': 5, 'name': 'Storage', 'childGroupIds': [], 'resourceIds': []}
ingestGroup = {'id': 6, 'name': 'Ingest Nodes', 'childGroupIds': [], 'resourceIds': []}
resourceGroups.append(coreStationsGroup)
resourceGroups.append(remoteStationsGroup)
resourceGroups.append(stationsGroup)
resourceGroups.append(cobaltGroup)
resourceGroups.append(cep4NodeGroup)
resourceGroups.append(cep4StorageGroup)
resourceGroups.append(ingestGroup)

resourceItems = []

for i in range(1, numCoreStations+1):
    station = {'id': len(resourceItems), 'name': 'CS%03d' % i, 'typeId': 0, 'type': 'station'}
    coreStationsGroup['resourceIds'].append(station['id'])
    stationsGroup['resourceIds'].append(station['id'])
    resourceItems.append(station)

for i in range(1, numRemoteStations+1):
    station = {'id': len(resourceItems), 'name': 'RS%03d' % i, 'typeId': 0, 'type': 'station'}
    remoteStationsGroup['resourceIds'].append(station['id'])
    stationsGroup['resourceIds'].append(station['id'])
    resourceItems.append(station)

for i in range(1, numCobaltNodes+1):
    node = {'id': len(resourceItems), 'name': 'cobalt%03d' % i, 'typeId': 1, 'type': 'correlator'}
    cobaltGroup['resourceIds'].append(node['id'])
    resourceItems.append(node)

for i in range(1, numComputeNodes+1):
    node = {'id': len(resourceItems), 'name': 'cep4node_%03d' % i, 'typeId': 2, 'type': 'computenode'}
    cep4NodeGroup['resourceIds'].append(node['id'])
    resourceItems.append(node)

for i in range(1, numIngestNodes+1):
    node = {'id': len(resourceItems), 'name': 'lexar%03d' % i, 'typeId': 3, 'type': 'ingestnode'}
    ingestGroup['resourceIds'].append(node['id'])
    resourceItems.append(node)

cep4storage = {'id': len(resourceItems), 'name': 'CEP4 Storage', 'typeId': 4, 'type': 'cep4storage'}
cep4StorageGroup['resourceIds'].append(cep4storage['id'])
resourceItems.append(cep4storage)

stations = [r for r in resourceItems if r['typeId'] == 0]
correlators = [r for r in resourceItems if r['typeId'] == 1]
computenodes = [r for r in resourceItems if r['typeId'] == 2]
ingestnodes = [r for r in resourceItems if r['typeId'] == 3]

resourceClaims = []
resourceGroupClaims = []
for task in tasks.values():
    taskResourceGroupIds = set()
    taskResources = []
    if task['type'] == 'Observation':
        taskResources = stations + correlators
    elif task['type'] == 'Pipeline':
        taskResources = computenodes
    elif task['type'] == 'Ingest':
        taskResources = ingestnodes

    for resource in taskResources:
        claim = {'id': len(resourceClaims), 'resourceId': resource['id'], 'taskId': task['id'], 'startTime': task['from'], 'endTime': task['to'], 'status': 'allocated'}
        resourceClaims.append(claim)

        groupIds = [rg['id'] for rg in resourceGroups if resource['id'] in rg['resourceIds']]
        taskResourceGroupIds |= set(groupIds)

    claim = {'id': len(resourceClaims), 'resourceId': cep4storage['id'], 'taskId': task['id'], 'startTime': task['from'], 'endTime': task['from'] + timedelta(days=1), 'status': 'allocated'}
    resourceClaims.append(claim)

    for groupId in taskResourceGroupIds:
        groupClaim = {'id': len(resourceGroupClaims), 'resourceGroupId': groupId, 'taskId': task['id'], 'startTime': task['from'], 'endTime': task['to'], 'status': 'allocated'}
        resourceGroupClaims.append(groupClaim)
