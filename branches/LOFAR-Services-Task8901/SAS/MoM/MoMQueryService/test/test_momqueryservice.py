#!/usr/bin/python

# Copyright (C) 2012-2015    ASTRON (Netherlands Institute for Radio Astronomy)
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.

# $Id:  $

import unittest
import uuid
from lofar.mom.momqueryservice.momqueryservice import createService
from lofar.mom.momqueryservice.momqueryservice import ProjectDetailsQueryHandler
from lofar.mom.momqueryservice import momprojectdetailsquery
from qpid.messaging import Connection
from qpidtoollibs import BrokerAgent

try:
    # setup broker connection
    connection = Connection.establish('127.0.0.1')
    broker = BrokerAgent(connection)

    # add test service busname
    busname = "momqueryservice-test-%s" % (uuid.uuid1())
    broker.addExchange('topic', busname)

    testid = '1234'

    # create a mock for the MoMDatabaseWrapper
    # so we don't need the actual momdb for this test
    # and we don't need the momdb passwd
    class MockMoMDatabaseWrapper:
        def getProjectDetails(self, mom_ids_str):
            return [{'project_mom2id': '4567', 'project_name': 'foo', 'project_description': 'bar', 'object_mom2id': testid}]

    class MockProjectDetailsQueryHandler(ProjectDetailsQueryHandler):
        def prepare_loop(self):
            self.momdb = MockMoMDatabaseWrapper()

    # inject the mock into the service
    with createService(busname, handler=MockProjectDetailsQueryHandler):

        class TestLTAStorageDb(unittest.TestCase):
            def testProjectDetailsQuery(self):
                result = momprojectdetailsquery.getProjectDetails(testid, busname)
                self.assertEquals(1, len(result.keys()))
                self.assertEquals(testid, result.keys()[0])
                self.assertTrue('project_mom2id' in result[testid])
                self.assertTrue('project_name' in result[testid])
                self.assertTrue('project_description' in result[testid])

            def testSqlInjection(self):
                inj_testid = testid + '; select * from lofar_mom3.mom2object;'
                result = momprojectdetailsquery.getProjectDetails(inj_testid, busname)

                self.assertTrue('errmsg' in result)
                self.assertTrue('KeyError' in result['errmsg'])

        unittest.main(verbosity=2)

finally:
    # cleanup test bus and exit
    broker.delExchange(busname)
    connection.close()
