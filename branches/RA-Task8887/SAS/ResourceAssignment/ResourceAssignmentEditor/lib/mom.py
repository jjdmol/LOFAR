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
    def applyDefaults(t):
        '''apply sane default values for a task'''
        t['name'] = 'Task (sasId: %d)' % t['otdb_id']
        t['project_name'] = '<unknown>'
        t['project_mom_id'] = -99

    tasklist = task if isinstance(task, list) else [task]

    for t in tasklist:
        applyDefaults(t)

    if not momrpc:
        return

    try:
        momIds = ','.join([str(t['mom_id']) for t in tasklist])
        logger.info('momrpc.getProjectDetails(momIds)')
        details = momrpc.getProjectDetails(momIds)
        logger.info('details=' + str(details))

        for t in tasklist:
            mom_id = str(t['mom_id'])
            if mom_id in details:
                m = details[mom_id]
                t['name'] = m['object_name']
                t['project_name'] = m['project_name']
                t['project_mom_id'] = m['project_mom2id']
            else:
                t['project_name'] = 'OTDB Only'
                t['project_mom_id'] = -98
    except Exception as e:
        logger.error(str(e))

