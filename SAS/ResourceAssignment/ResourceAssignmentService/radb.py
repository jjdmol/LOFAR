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

'''
TODO: documentation
'''
import logging
import psycopg2
import psycopg2.extras

logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s',
                    level=logging.INFO)
logger = logging.getLogger(__name__)


class RADatabase:
    def __init__(self, password='',
                 username='resourceassignment',
                 host='mcu005.control.lofar',
                 database='resourceassignment'):
        self.conn = psycopg2.connect(host=host,
                                     user=username,
                                     password=password,
                                     database=database)
        self.cursor = self.conn.cursor(cursor_factory = psycopg2.extras.RealDictCursor)

    def getTaskStatuses(self):
        query = '''SELECT * from resource_allocation.task_status;'''
        self.cursor.execute(query)

        result = self.cursor.fetchall()
        return result

    def getTaskStatusNames(self):
        return [x['name'] for x in self.getTaskStatuses()]

    def getTaskTypes(self):
        query = '''SELECT * from resource_allocation.task_type;'''
        self.cursor.execute(query)

        result = self.cursor.fetchall()
        return result

    def getTaskTypeNames(self):
        return [x['name'] for x in self.getTaskTypes()]

    def getResourceClaimStatuses(self):
        query = '''SELECT * from resource_allocation.resource_claim_status;'''
        self.cursor.execute(query)

        result = self.cursor.fetchall()
        return result

    def getResourceClaimStatusNames(self):
        return [x['name'] for x in self.getResourceClaimStatuses()]

    def getTasks(self):
        query = '''SELECT t.*, ts.name as status, tt.name as type
        from resource_allocation.task t
        inner join resource_allocation.task_status ts on ts.id = t.status_id
        inner join resource_allocation.task_type tt on tt.id = t.type_id;
        '''
        self.cursor.execute(query)

        result = self.cursor.fetchall()
        return result

    def getResourceTypes(self):
        query = '''SELECT rt.*, rtu.units as unit
        from virtual_instrument.resource_type rt
        inner join virtual_instrument.unit rtu on rtu.id = rt.unit_id;
        '''
        self.cursor.execute(query)

        result = self.cursor.fetchall()
        return result

    def getResourceTypeNames(self):
        return [x['name'] for x in self.getResourceTypes()]

    def getResourceGroupTypes(self):
        query = '''SELECT * from virtual_instrument.resource_group_type;'''
        self.cursor.execute(query)

        result = self.cursor.fetchall()
        return result

    def getResourceGroupTypeNames(self):
        return [x['name'] for x in self.getResourceGroupTypes()]

    def getUnits(self):
        query = '''SELECT * from virtual_instrument.unit;'''
        self.cursor.execute(query)

        result = self.cursor.fetchall()
        return result

    def getUnitNames(self):
        return [x['units'] for x in self.getUnits()]

    def getResources(self):
        query = '''SELECT r.*, rt.name as type, rtu.units as unit
        from virtual_instrument.resource r
        inner join virtual_instrument.resource_type rt on rt.id = r.type_id
        inner join virtual_instrument.unit rtu on rtu.id = rt.unit_id;
        '''
        self.cursor.execute(query)

        result = self.cursor.fetchall()
        return result

    def getResourceGroups(self):
        query = '''SELECT rg.*, rgt.name as type
        from virtual_instrument.resource_group rg
        inner join virtual_instrument.resource_group_type rgt on rgt.id = rg.type_id;
        '''
        self.cursor.execute(query)

        result = self.cursor.fetchall()
        return result

    def getResourceClaims(self):
        query = '''SELECT rc.*, rcs.name as status
        from resource_allocation.resource_claim rc
        inner join resource_allocation.resource_claim_status rcs on rcs.id = rc.status_id;
        '''
        self.cursor.execute(query)

        result = self.cursor.fetchall()
        return result

if __name__ == '__main__':
    db = RADatabase(host='10.149.96.6', password='123456')

    def resultPrint(method):
        print '\n-- ' + str(method.__name__) + ' --'
        print '\n'.join([str(x) for x in method()])

    resultPrint(db.getTaskStatuses)
    resultPrint(db.getTaskStatusNames)
    resultPrint(db.getTaskTypes)
    resultPrint(db.getTaskTypeNames)
    resultPrint(db.getResourceClaimStatuses)
    resultPrint(db.getResourceClaimStatusNames)
    resultPrint(db.getUnits)
    resultPrint(db.getUnitNames)
    resultPrint(db.getResourceTypes)
    resultPrint(db.getResourceTypeNames)
    resultPrint(db.getResourceGroupTypes)
    resultPrint(db.getResourceGroupTypeNames)
    resultPrint(db.getResources)
    resultPrint(db.getResourceGroups)
    resultPrint(db.getTasks)
    resultPrint(db.getResourceClaims)
