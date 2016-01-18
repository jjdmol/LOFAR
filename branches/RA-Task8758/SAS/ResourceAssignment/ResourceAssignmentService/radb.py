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
                 host='10.149.96.6', #mcu005.control.lofar
                 database='resourceassignment'):
        self.conn = psycopg2.connect(host=host,
                                     user=username,
                                     password=password,
                                     database=database)
        self.cursor = self.conn.cursor(cursor_factory = psycopg2.extras.RealDictCursor)

    def getTaskStatuses(self):
        query = '''SELECT * from resource_allocation.task_status;'''
        self.cursor.execute(query)

        return list(self.cursor.fetchall())

    def getTaskStatusNames(self):
        return [x['name'] for x in self.getTaskStatuses()]

    def getTaskStatusId(self, status_name):
        query = '''SELECT id from resource_allocation.task_status
                   WHERE name = %s;'''
        self.cursor.execute(query, [status_name])
        result = self.cursor.fetchone()

        if result:
            return result['id']

        raise KeyError('No such status: %s' % status_name)

    def getTaskTypes(self):
        query = '''SELECT * from resource_allocation.task_type;'''
        self.cursor.execute(query)

        return list(self.cursor.fetchall())

    def getTaskTypeNames(self):
        return [x['name'] for x in self.getTaskTypes()]

    def getTaskTypeId(self, type_name):
        query = '''SELECT id from resource_allocation.task_type
                   WHERE name = %s;'''
        self.cursor.execute(query, [type_name])
        result = self.cursor.fetchone()

        if result:
            return result['id']

        raise KeyError('No such type: %s' % type_name)

    def getResourceClaimStatuses(self):
        query = '''SELECT * from resource_allocation.resource_claim_status;'''
        self.cursor.execute(query)

        return list(self.cursor.fetchall())

    def getResourceClaimStatusNames(self):
        return [x['name'] for x in self.getResourceClaimStatuses()]

    def getTasks(self):
        query = '''SELECT t.*, ts.name as status,
        tt.name as type,
        s.starttime as starttime,
        s.endtime as endtime
        from resource_allocation.task t
        inner join resource_allocation.task_status ts on ts.id = t.status_id
        inner join resource_allocation.task_type tt on tt.id = t.type_id
        inner join resource_allocation.specification s on s.id = t.specification_id;
        '''
        self.cursor.execute(query)

        return list(self.cursor.fetchall())

    def getTask(self, id):
        query = '''SELECT t.*, ts.name as status,
        tt.name as type,
        s.starttime as starttime,
        s.endtime as endtime
        from resource_allocation.task t
        inner join resource_allocation.task_status ts on ts.id = t.status_id
        inner join resource_allocation.task_type tt on tt.id = t.type_id
        inner join resource_allocation.specification s on s.id = t.specification_id
        where t.id = (%s)
        '''
        self.cursor.execute(query, (id,))

        result = self.cursor.fetchone()
        return dict(result) if result else None

    def _convertTaskTypeAndStatusToIds(self, task_status, task_type):
        '''converts task_status and task_type to id's in case one and/or the other are strings'''
        if task_status and isinstance(task_status, basestring):
            #convert task_status string to task_status.id
            task_status = self.getTaskStatusId(task_status)

        if task_type and isinstance(task_type, basestring):
            #convert task_type string to task_type.id
            task_type = self.getTaskTypeId(task_type)

        return task_status, task_type

    def insertTask(self, mom_id, otdb_id, task_status, task_type, specification_id, commit=True):
        task_status, task_type = self._convertTaskTypeAndStatusToIds(task_status, task_type)

        query = '''INSERT INTO resource_allocation.task
        (mom_id, otdb_id, status_id, type_id, specification_id)
        VALUES (%s, %s, %s, %s, %s)
        RETURNING id;'''

        self.cursor.execute(query, (mom_id, otdb_id, task_status, task_type, specification_id))
        id = self.cursor.fetchone()['id']
        if commit:
            self.conn.commit()
        return id

    def updateTask(self, task_id, mom_id=None, otdb_id=None, task_status=None, task_type=None, specification_id=None, commit=True):
        task_status, task_type = self._convertTaskTypeAndStatusToIds(task_status, task_type)

        fields = []
        values = []

        if mom_id:
            fields.append('mom_id')
            values.append(mom_id)

        if otdb_id:
            fields.append('otdb_id')
            values.append(otdb_id)

        if task_status:
            fields.append('status_id')
            values.append(task_status)

        if task_type:
            fields.append('type_id')
            values.append(task_type)

        if specification_id:
            fields.append('specification_id')
            values.append(specification_id)

        values.append(task_id)

        query = '''UPDATE resource_allocation.task
        SET ({fields}) = ({value_placeholders})
        WHERE resource_allocation.task.id = {task_id_placeholder};'''.format(fields=', '.join(fields),
                                                                             value_placeholders=', '.join('%s' for x in fields),
                                                                             task_id_placeholder='%s')

        self.cursor.execute(query, values)
        if commit:
            self.conn.commit()

        return self.cursor.rowcount > 0

    def getSpecifications(self):
        query = '''SELECT * from resource_allocation.specification;'''
        self.cursor.execute(query)

        return list(self.cursor.fetchall())

    def getSpecification(self, specification_id):
        query = '''SELECT * from resource_allocation.specification spec
        WHERE spec.id = (%s);'''
        self.cursor.execute(query, (specification_id,))

        return list(self.cursor.fetchall())

    def getResourceTypes(self):
        query = '''SELECT rt.*, rtu.units as unit
        from virtual_instrument.resource_type rt
        inner join virtual_instrument.unit rtu on rtu.id = rt.unit_id;
        '''
        self.cursor.execute(query)

        return list(self.cursor.fetchall())

    def getResourceTypeNames(self):
        return [x['name'] for x in self.getResourceTypes()]

    def getResourceTypeId(self, type_name):
        query = '''SELECT id from virtual_instrument.resource_type
                   WHERE name = %s;'''
        self.cursor.execute(query, [type_name])
        result = self.cursor.fetchone()

        if result:
            return result['id']

        raise KeyError('No such type: %s' % type_name)

    def getResourceGroupTypes(self):
        query = '''SELECT * from virtual_instrument.resource_group_type;'''
        self.cursor.execute(query)

        return list(self.cursor.fetchall())

    def getResourceGroupTypeNames(self):
        return [x['name'] for x in self.getResourceGroupTypes()]

    def getResourceGroupTypeId(self, type_name):
        query = '''SELECT id from virtual_instrument.resource_group_type
                   WHERE name = %s;'''
        self.cursor.execute(query, [type_name])
        result = self.cursor.fetchone()

        if result:
            return result['id']

        raise KeyError('No such type: %s' % type_name)

    def getUnits(self):
        query = '''SELECT * from virtual_instrument.unit;'''
        self.cursor.execute(query)

        return list(self.cursor.fetchall())

    def getUnitNames(self):
        return [x['units'] for x in self.getUnits()]

    def getUnitId(self, unit_name):
        query = '''SELECT * from virtual_instrument.unit
                   WHERE units = %s;'''
        self.cursor.execute(query, [unit_name])
        result = self.cursor.fetchone()

        if result:
            return result['id']

        raise KeyError('No such unit: %s' % unit_name)

    def getResources(self):
        query = '''SELECT r.*, rt.name as type, rtu.units as unit
        from virtual_instrument.resource r
        inner join virtual_instrument.resource_type rt on rt.id = r.type_id
        inner join virtual_instrument.unit rtu on rtu.id = rt.unit_id;
        '''
        self.cursor.execute(query)

        return list(self.cursor.fetchall())

    def getResourceGroups(self):
        query = '''SELECT rg.*, rgt.name as type
        from virtual_instrument.resource_group rg
        inner join virtual_instrument.resource_group_type rgt on rgt.id = rg.type_id;
        '''
        self.cursor.execute(query)

        return list(self.cursor.fetchall())

    def getResourceClaims(self):
        query = '''SELECT rc.*, rcs.name as status
        from resource_allocation.resource_claim rc
        inner join resource_allocation.resource_claim_status rcs on rcs.id = rc.status_id;
        '''
        self.cursor.execute(query)

        return list(self.cursor.fetchall())



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
    resultPrint(db.getSpecifications)
    resultPrint(db.getResourceClaims)

    taskId = db.insertTask(1234, 5678, 'active', 'OBSERVATION', 1)

    resultPrint(db.getTasks)

    print db.updateTask(taskId, task_status='scheduled', otdb_id=723, task_type='PIPELINE')

    resultPrint(db.getTasks)

    units = db.getUnits()
    for q in units:
        print q['id'], db.getUnitId(q['units'])

    statuses = db.getTaskStatuses()
    for q in statuses:
        print q['id'], db.getTaskStatusId(q['name'])





