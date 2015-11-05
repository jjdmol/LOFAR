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


allTasks = []
now = datetime.utcnow()

for p in range(1, 4):
    for i in range(20):
        task = _genTask('LC4_%03d Obs %d'% (p, i), now + timedelta(hours=p*i*4), timedelta(hours=4))
        allTasks.append(task)

    for i in range(5):
        task = _genTask('LC4_%03d Pipeline %d'% (p, i), now + timedelta(hours=p*i*4), timedelta(hours=4), type='Pipeline')
        allTasks.append(task)

resourceItems = [{'id': 0, 'name': 'CS001', 'typeId': 0, 'type': 'station', 'group': False},
                 {'id': 1, 'name': 'CS002', 'typeId': 0, 'type': 'station', 'group': False},
                 {'id': 2, 'name': 'CS003', 'typeId': 0, 'type': 'station', 'group': False},
                 {'id': 3, 'name': 'CS004', 'typeId': 0, 'type': 'station', 'group': False},
                 {'id': 4, 'name': 'Core', 'typeId': 1, 'type': 'stationset', 'group': True},
                 {'id': 5, 'name': 'Node1', 'typeId': 2, 'type': 'node', 'group': False},
                 {'id': 6, 'name': 'Node2', 'typeId': 2, 'type': 'node', 'group': False}
                 ]

resourceClaims = [{'id': 0, 'resourceId': 0, 'taskId': 1, 'startTime': '2015-10-28T14:14:00Z', 'endTime': '2015-10-28T17:00:00Z', 'status': 'allocated'},
                  {'id': 1, 'resourceId': 1, 'taskId': 1, 'startTime': '2015-10-28T14:14:00Z', 'endTime': '2015-10-28T17:00:00Z', 'status': 'allocated'},
                  {'id': 2, 'resourceId': 2, 'taskId': 1, 'startTime': '2015-10-28T14:14:00Z', 'endTime': '2015-10-28T17:00:00Z', 'status': 'allocated'},
                  {'id': 3, 'resourceId': 3, 'taskId': 1, 'startTime': '2015-10-28T14:14:00Z', 'endTime': '2015-10-28T17:00:00Z', 'status': 'allocated'},
                  {'id': 4, 'resourceId': 4, 'taskId': 2, 'startTime': '2015-10-29T10:00:00Z', 'endTime': '2015-10-29T12:00:00Z', 'status': 'claimed'},
                  {'id': 5, 'resourceId': 4, 'taskId': 3, 'startTime': '2015-10-29T12:15:00Z', 'endTime': '2015-10-29T18:00:00Z', 'status': 'claimed'},
                  ]
