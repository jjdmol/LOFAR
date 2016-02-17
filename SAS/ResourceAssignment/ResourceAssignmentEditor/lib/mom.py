#!/usr/bin/env python

# mom.py
#
# Copyright (C) 2015
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id: mom.py 1580 2015-09-30 14:18:57Z loose $

"""
TODO: documentation
"""

import logging

logger = logging.getLogger(__name__)

def updateTaskMomDetails(task, momrpc):
    '''fill in the task propeties with mom object and project details.
    :param task: dictionary or list of dictionaries with the task(s)
    :param momrpc: MoM rpc object the query for details'''
    if not momrpc:
        return

    def copyValues(t, m):
        t['name'] = m['object_name']
        t['project_name'] = m['project_name']
        t['project_mom_id'] = m['project_mom2id']


    if isinstance(task, list):
        momIds = ','.join([str(t['mom_id']) for t in task])
    else:
        momIds = task['mom_id']

    logger.info('-----------------------')
    logger.info(momIds)

    details = momrpc.getProjectDetails(momIds)

    logger.info('-----------------------')
    logger.info(details)
    logger.info('-----------------------')

    if isinstance(task, list):
        for t in task:
            mom_id = str(t['mom_id'])
            if mom_id in details:
                copyValues(t, details[mom_id])
            else:
                print t
                t['name'] = 'Task (sasId: %d)' % t['otdb_id']
                t['project_name'] = 'OTDB Only'
                t['project_mom_id'] = -42
    else:
        mom_id = task['mom_id']
        if mom_id in details:
            copyValues(task, details[mom_id])

    logger.info(task)
    logger.info('-----------------------')
