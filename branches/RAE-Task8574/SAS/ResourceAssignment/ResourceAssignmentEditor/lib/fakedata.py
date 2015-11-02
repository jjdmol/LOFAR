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

observartionTasks = [{'id': 0, 'momId': 123, 'obsId': 876, 'status': 'scheduled', 'name': 'Lobos Obs 2a', 'from': datetime.utcnow() - timedelta(hours=1), 'to': datetime.utcnow() + timedelta(hours=5)},
                     {'id': 1, 'momId': 345, 'obsId': 654, 'status': 'approved', 'name': 'LOTAAS Obs 32q', 'from': datetime.utcnow() + timedelta(hours=6), 'to': datetime.utcnow() + timedelta(hours=12)},
                     {'id': 2, 'momId': 567, 'obsId': 432, 'status': 'approved', 'name': 'Pulsar Obs 3', 'from': datetime.utcnow() + timedelta(hours=13), 'to': datetime.utcnow() + timedelta(hours=32)}
                     ]

pipelineTasks = [{'id': 100, 'momId': 100123, 'obsId': 100876, 'status': 'scheduled', 'name': 'Averaging', 'from': datetime.utcnow() - timedelta(hours=1), 'to': datetime.utcnow() + timedelta(hours=1)},
                 {'id': 101, 'momId': 100124, 'obsId': 100875, 'status': 'scheduled', 'name': 'Averaging', 'from': datetime.utcnow() + timedelta(hours=1), 'to': datetime.utcnow() + timedelta(hours=2)}
                 ]

maintenanceTasks = [{'id': 200, 'momId': 200124, 'obsId': 200875, 'status': 'scheduled', 'name': 'Mowing', 'from': datetime.utcnow() + timedelta(days=1), 'to': datetime.utcnow() + timedelta(days=2)}
                     ]

reservationTasks = [{'id': 300, 'momId': 300124, 'obsId': 300875, 'status': 'scheduled', 'name': 'Glow', 'from': datetime.utcnow() + timedelta(days=1), 'to': datetime.utcnow() + timedelta(days=3)}
                    ]
ingestTasks = [{'id': 400, 'momId': 400124, 'obsId': 400875, 'status': 'scheduled', 'name': 'Ingest', 'from': datetime.utcnow() - timedelta(hours=1), 'to': datetime.utcnow() + timedelta(hours=10)}
               ]

allTasks = observartionTasks + pipelineTasks + maintenanceTasks + reservationTasks + ingestTasks

resourceItems = [{'id': 0, 'name': 'CS001', 'typeId': 0, 'type': 'station', 'group': False},
                 {'id': 1, 'name': 'CS002', 'typeId': 0, 'type': 'station', 'group': False},
                 {'id': 2, 'name': 'CS003', 'typeId': 0, 'type': 'station', 'group': False},
                 {'id': 3, 'name': 'CS004', 'typeId': 0, 'type': 'station', 'group': False},
                 {'id': 4, 'name': 'Core', 'typeId': 1, 'type': 'stationset', 'group': True},
                 {'id': 5, 'name': 'Node1', 'typeId': 2, 'type': 'node', 'group': False},
                 {'id': 6, 'name': 'Node2', 'typeId': 2, 'type': 'node', 'group': False}
                 ]

resourceClaims = [{'id': 0, 'resourceId': 0, 'taskId': 0, 'startTime': '2015-10-28T14:14:00Z', 'endTime': '2015-10-28T17:00:00Z', 'status': 'allocated'},
                  {'id': 1, 'resourceId': 1, 'taskId': 0, 'startTime': '2015-10-28T14:14:00Z', 'endTime': '2015-10-28T17:00:00Z', 'status': 'allocated'},
                  {'id': 2, 'resourceId': 2, 'taskId': 0, 'startTime': '2015-10-28T14:14:00Z', 'endTime': '2015-10-28T17:00:00Z', 'status': 'allocated'},
                  {'id': 3, 'resourceId': 3, 'taskId': 0, 'startTime': '2015-10-28T14:14:00Z', 'endTime': '2015-10-28T17:00:00Z', 'status': 'allocated'},
                  {'id': 4, 'resourceId': 4, 'taskId': 1, 'startTime': '2015-10-29T10:00:00Z', 'endTime': '2015-10-29T12:00:00Z', 'status': 'claimed'},
                  {'id': 5, 'resourceId': 4, 'taskId': 2, 'startTime': '2015-10-29T12:15:00Z', 'endTime': '2015-10-29T18:00:00Z', 'status': 'claimed'},
                  ]
