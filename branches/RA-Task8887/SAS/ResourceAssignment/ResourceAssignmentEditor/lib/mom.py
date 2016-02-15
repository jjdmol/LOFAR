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
    :param task: dictionary with the task
    :param momrpc: MoM rpc object the query for details'''
    logger.info(task)
    if not momrpc:
        return
    details = momrpc.getProjectDetails(task['mom_id'])
    if details:
        logger.info('-----------------------')
        logger.info(details)
        details = details[str(task['mom_id'])]
        logger.info('-----------------------')
        logger.info(details)
        logger.info('-----------------------')
        task['name'] = details['object_name']
        task['project_name'] = details['project_name']
        task['project_mom_id'] = details['project_mom2id']
        logger.info(task)

