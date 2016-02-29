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
import datetime
import time
from optparse import OptionParser
from lofar.common import dbcredentials

logger = logging.getLogger(__name__)

_FETCH_NONE=0
_FETCH_ONE=1
_FETCH_ALL=2

class RADatabase:
    def __init__(self, dbcreds=None):
        self.dbcreds = dbcreds
        self.conn = None
        self.cursor = None

    def _connect(self):
        self.conn = None
        self.cursor = None

        self.conn = psycopg2.connect(host=self.dbcreds.host,
                                     user=self.dbcreds.user,
                                     password=self.dbcreds.password,
                                     database=self.dbcreds.database,
                                     connect_timeout=5)
        self.cursor = self.conn.cursor(cursor_factory = psycopg2.extras.RealDictCursor)

    def _executeQuery(self, query, qargs=None, fetch=_FETCH_NONE):
        '''execute the query and reconnect upon OperationalError'''
        try:
            self.cursor.execute(query, qargs)
        except (psycopg2.OperationalError, AttributeError) as e:
            if isinstance(e, psycopg2.OperationalError):
                logger.error(str(e))
            for i in range(5):
                logger.info("(re)trying to connect to radb")
                self._connect()
                if self.conn:
                    logger.info("connected to radb")
                    self.cursor.execute(query, qargs)
                    break
                time.sleep(i*i)

        if fetch == _FETCH_ONE:
            return self.cursor.fetchone()

        if fetch == _FETCH_ALL:
            return self.cursor.fetchall()


    def commit(self):
        self.conn.commit()

    def getTaskStatuses(self):
        query = '''SELECT * from resource_allocation.task_status;'''

        return list(self._executeQuery(query, fetch=_FETCH_ALL))

    def getTaskStatusNames(self):
        return [x['name'] for x in self.getTaskStatuses()]

    def getTaskStatusId(self, status_name):
        query = '''SELECT id from resource_allocation.task_status
                   WHERE name = %s;'''
        result = self._executeQuery(query, [status_name], fetch=_FETCH_One)

        if result:
            return result['id']

        raise KeyError('No such status: %s Valid values are: %s' % (status_name, ', '.join(self.getTaskStatusNames())))

    def getTaskTypes(self):
        query = '''SELECT * from resource_allocation.task_type;'''

        return list(self._executeQuery(query, fetch=_FETCH_ALL))

    def getTaskTypeNames(self):
        return [x['name'] for x in self.getTaskTypes()]

    def getTaskTypeId(self, type_name):
        query = '''SELECT id from resource_allocation.task_type
                   WHERE name = %s;'''
        result = self._executeQuery(query, [type_name], fetch=_FETCH_ONE)

        if result:
            return result['id']

        raise KeyError('No such type: %s Valid values are: %s' % (type_name, ', '.join(self.getTaskTypeNames())))

    def getResourceClaimStatuses(self):
        query = '''SELECT * from resource_allocation.resource_claim_status;'''

        return list(self._executeQuery(query, fetch=_FETCH_ALL))

    def getResourceClaimStatusNames(self):
        return [x['name'] for x in self.getResourceClaimStatuses()]

    def getResourceClaimStatusId(self, status_name):
        query = '''SELECT id from resource_allocation.resource_claim_status
                   WHERE name = %s;'''
        result = self._executeQuery(query, [status_name], fetch=_FETCH_ONE)

        if result:
            return result['id']

        raise KeyError('No such status: %s. Valid values are: %s' % (status_name, ', '.join(self.getResourceClaimStatusNames())))

    def getTasks(self):
        query = '''SELECT * from resource_allocation.task_view;'''

        return list(self._executeQuery(query, fetch=_FETCH_ALL))

    def getTask(self, id=None, mom_id=None, otdb_id=None):
        '''get a task for either the given (task)id, or for the given mom_id, or for the given otdb_id'''
        ids = [id, mom_id, otdb_id]
        validIds = [x for x in ids if x != None]

        if len(validIds) != 1:
            raise KeyError("Provide one and only one id: id=%s, mom_id=%s, otdb_id=%s" % (id, mom_id, otdb_id))

        query = '''SELECT * from resource_allocation.task_view tv '''
        if id:
            query += '''where tv.id = (%s);'''
        elif mom_id:
            query += '''where tv.mom_id = (%s);'''
        elif otdb_id:
            query += '''where tv.otdb_id = (%s);'''
        result = self._executeQuery(query, validIds, fetch=_FETCH_ONE)

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

        id = self._executeQuery(query, (mom_id, otdb_id, task_status, task_type, specification_id), fetch=_FETCH_ONE)['id']
        if commit:
            self.conn.commit()
        return id

    def deleteTask(self, task_id, commit=True):
        query = '''DELETE FROM resource_allocation.task
                   WHERE resource_allocation.task.id = %s;'''

        self._executeQuery(query, [task_id])
        if commit:
            self.conn.commit()
        return self.cursor.rowcount > 0

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

        self._executeQuery(query, values)
        if commit:
            self.conn.commit()

        return self.cursor.rowcount > 0

    def getSpecifications(self):
        query = '''SELECT * from resource_allocation.specification;'''

        return list(self._executeQuery(query, fetch=_FETCH_ALL))

    def getSpecification(self, specification_id):
        query = '''SELECT * from resource_allocation.specification spec
        WHERE spec.id = (%s);'''

        return list(self._executeQuery(query, [specification_id], fetch=_FETCH_ALL))

    def insertSpecification(self, starttime, endtime, content, commit=True):
        query = '''INSERT INTO resource_allocation.specification
        (starttime, endtime, content)
        VALUES (%s, %s, %s)
        RETURNING id;'''

        id = self._executeQuery(query, (starttime, endtime, content), fetch=_FETCH_ONE)['id']
        if commit:
            self.conn.commit()
        return id

    def deleteSpecification(self, specification_id, commit=True):
        query = '''DELETE FROM resource_allocation.specification
                   WHERE resource_allocation.specification.id = %s;'''

        self._executeQuery(query, [specification_id])
        if commit:
            self.conn.commit()
        return self.cursor.rowcount > 0

    def updateSpecification(self, specification_id, starttime=None, endtime=None, content=None, commit=True):
        fields = []
        values = []

        if starttime:
            fields.append('starttime')
            values.append(starttime)

        if endtime:
            fields.append('endtime')
            values.append(endtime)

        if content:
            fields.append('content')
            values.append(content)

        values.append(specification_id)

        query = '''UPDATE resource_allocation.specification
        SET ({fields}) = ({value_placeholders})
        WHERE resource_allocation.specification.id = {id_placeholder};'''.format(fields=', '.join(fields),
                                                                                 value_placeholders=', '.join('%s' for x in fields),
                                                                                 id_placeholder='%s')

        self._executeQuery(query, values)
        if commit:
            self.conn.commit()

        return self.cursor.rowcount > 0

    def getResourceTypes(self):
        query = '''SELECT rt.*, rtu.units as unit
        from virtual_instrument.resource_type rt
        inner join virtual_instrument.unit rtu on rtu.id = rt.unit_id;
        '''

        return list(self._executeQuery(query, fetch=_FETCH_ALL))

    def getResourceTypeNames(self):
        return [x['name'] for x in self.getResourceTypes()]

    def getResourceTypeId(self, type_name):
        query = '''SELECT id from virtual_instrument.resource_type
                   WHERE name = %s;'''
        result = self._executeQuery(query, [type_name], fetch=_FETCH_ONE)

        if result:
            return result['id']

        raise KeyError('No such type: %s Valid values are: %s' % (type_name, ', '.join(self.getResourceTypeNames())))

    def getResourceGroupTypes(self):
        query = '''SELECT * from virtual_instrument.resource_group_type;'''

        return list(self._executeQuery(query, fetch=_FETCH_ALL))

    def getResourceGroupTypeNames(self):
        return [x['name'] for x in self.getResourceGroupTypes()]

    def getResourceGroupTypeId(self, type_name):
        query = '''SELECT id from virtual_instrument.resource_group_type
                   WHERE name = %s;'''
        result = self._executeQuery(query, [type_name], fetch=_FETCH_ONE)

        if result:
            return result['id']

        raise KeyError('No such type: %s Valid values are: %s' % (type_name, ', '.join(self.getResourceGroupTypeNames())))

    def getUnits(self):
        query = '''SELECT * from virtual_instrument.unit;'''

        return list(self._executeQuery(query, fetch=_FETCH_ALL))

    def getUnitNames(self):
        return [x['units'] for x in self.getUnits()]

    def getUnitId(self, unit_name):
        query = '''SELECT * from virtual_instrument.unit
                   WHERE units = %s;'''
        result = self._executeQuery(query, [unit_name], fetch=_FETCH_ONE)

        if result:
            return result['id']

        raise KeyError('No such unit: %s Valid values are: %s' % (type_name, ', '.join(self.getUnitNames())))

    def getResources(self):
        query = '''SELECT r.*, rt.name as type, rtu.units as unit
        from virtual_instrument.resource r
        inner join virtual_instrument.resource_type rt on rt.id = r.type_id
        inner join virtual_instrument.unit rtu on rtu.id = rt.unit_id;
        '''

        return list(self._executeQuery(query, fetch=_FETCH_ALL))

    def getResourceGroups(self):
        query = '''SELECT rg.*, rgt.name as type
        from virtual_instrument.resource_group rg
        inner join virtual_instrument.resource_group_type rgt on rgt.id = rg.type_id;
        '''

        return list(self._executeQuery(query, fetch=_FETCH_ALL))

    def getResourceClaims(self):
        query = '''SELECT * from resource_allocation.resource_claim_view'''

        return list(self._executeQuery(query, fetch=_FETCH_ALL))

    def getResourceClaim(self, id):
        query = '''SELECT * from resource_allocation.resource_claim_view rcv
        where rcv.id = %s;
        '''
        result = self._executeQuery(query, [id], fetch=_FETCH_ONE)

        return dict(result) if result else None

    def insertResourceClaim(self, resource_id, task_id, starttime, endtime, status, session_id, claim_size, username, user_id, commit=True):
        if status and isinstance(status, basestring):
            #convert status string to status.id
            status = self.getResourceClaimStatusId(status)

        query = '''INSERT INTO resource_allocation.resource_claim
        (resource_id, task_id, starttime, endtime, status_id, session_id, claim_size, username, user_id)
        VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s)
        RETURNING id;'''

        id = self._executeQuery(query, (resource_id, task_id, starttime, endtime, status, session_id, claim_size, username, user_id), fetch=_FETCH_ONE)['id']
        if commit:
            self.conn.commit()
        return id

    def deleteResourceClaim(self, resource_claim_id, commit=True):
        query = '''DELETE FROM resource_allocation.resource_claim
                   WHERE resource_allocation.resource_claim.id = %s;'''

        self._executeQuery(query, [resource_claim_id])
        if commit:
            self.conn.commit()
        return self.cursor.rowcount > 0

    def updateResourceClaim(self, resource_claim_id, resource_id=None, task_id=None, starttime=None, endtime=None, status=None, session_id=None, claim_size=None, username=None, user_id=None, commit=True):
        if status and isinstance(status, basestring):
            #convert status string to status.id
            status = self.getResourceClaimStatusId(status)

        fields = []
        values = []

        if resource_id:
            fields.append('resource_id')
            values.append(resource_id)

        if task_id:
            fields.append('task_id')
            values.append(task_id)

        if starttime:
            fields.append('starttime')
            values.append(starttime)

        if endtime:
            fields.append('endtime')
            values.append(endtime)

        if status:
            fields.append('status_id')
            values.append(status)

        if session_id:
            fields.append('session_id')
            values.append(session_id)

        if claim_size:
            fields.append('claim_size')
            values.append(claim_size)

        if username:
            fields.append('username')
            values.append(username)

        if user_id:
            fields.append('user_id')
            values.append(user_id)

        values.append(resource_claim_id)

        query = '''UPDATE resource_allocation.resource_claim
        SET ({fields}) = ({value_placeholders})
        WHERE resource_allocation.resource_claim.id = {rc_id_placeholder};'''.format(fields=', '.join(fields),
                                                                                     value_placeholders=', '.join('%s' for x in fields),
                                                                                     rc_id_placeholder='%s')

        self._executeQuery(query, values)
        if commit:
            self.conn.commit()

        return self.cursor.rowcount > 0

    def getResourceClaimsForTask(self, task_id):
        query = '''SELECT * from resource_allocation.resource_claim_view
        WHERE resource_allocation.resource_claim_view.task_id = %s'''

        return list(self._executeQuery(query, [task_id], fetch=_FETCH_ALL))

    def updateResourceClaimsForTask(self, task_id, starttime=None, endtime=None, status=None, session_id=None, username=None, user_id=None, commit=True):
        if status and isinstance(status, basestring):
            #convert status string to status.id
            status = self.getResourceClaimStatusId(status)

        fields = []
        values = []

        if starttime:
            fields.append('starttime')
            values.append(starttime)

        if endtime:
            fields.append('endtime')
            values.append(endtime)

        if status:
            fields.append('status_id')
            values.append(status)

        if session_id:
            fields.append('session_id')
            values.append(session_id)

        if username:
            fields.append('username')
            values.append(username)

        if user_id:
            fields.append('user_id')
            values.append(user_id)

        values.append(task_id)

        query = '''UPDATE resource_allocation.resource_claim
        SET ({fields}) = ({value_placeholders})
        WHERE resource_allocation.resource_claim.task_id = {task_id_placeholder};'''.format(fields=', '.join(fields),
                                                                                            value_placeholders=', '.join('%s' for x in fields),
                                                                                            task_id_placeholder='%s')

        self._executeQuery(query, values)
        if commit:
            self.conn.commit()

        return self.cursor.rowcount > 0


if __name__ == '__main__':
    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s',
                        level=logging.INFO)

    # Check the invocation arguments
    parser = OptionParser("%prog [options]",
                          description='runs some test queries on the radb')
    parser.add_option_group(dbcredentials.options_group(parser))
    parser.set_defaults(dbcredentials="RADB")
    (options, args) = parser.parse_args()

    dbcreds = dbcredentials.parse_options(options)

    db = RADatabase(dbcreds=dbcreds)

    def resultPrint(method):
        print '\n-- ' + str(method.__name__) + ' --'
        print '\n'.join([str(x) for x in method()])


    #resultPrint(db.getTaskStatuses)
    #resultPrint(db.getTaskStatusNames)
    #resultPrint(db.getTaskTypes)
    #resultPrint(db.getTaskTypeNames)
    #resultPrint(db.getResourceClaimStatuses)
    #resultPrint(db.getResourceClaimStatusNames)
    #resultPrint(db.getUnits)
    #resultPrint(db.getUnitNames)
    #resultPrint(db.getResourceTypes)
    #resultPrint(db.getResourceTypeNames)
    #resultPrint(db.getResourceGroupTypes)
    #resultPrint(db.getResourceGroupTypeNames)
    #resultPrint(db.getResources)
    #resultPrint(db.getResourceGroups)
    #resultPrint(db.getTasks)
    #resultPrint(db.getSpecifications)
    #resultPrint(db.getResourceClaims)

    #rcId = db.insertResourceClaim(1, 1, datetime.datetime.utcnow(), datetime.datetime.utcnow() + datetime.timedelta(hours=1), 'CLAIMED', 1, 10, 'einstein', -1, True)

    #resultPrint(db.getResourceClaims)

    #time.sleep(1)

    #rcId = db.updateResourceClaim(rcId, starttime=datetime.datetime.utcnow(), status='ALLOCATED')

    #resultPrint(db.getResourceClaims)

    #taskId = db.insertTask(1234, 5678, 'active', 'OBSERVATION', 1)

    #resultPrint(db.getTasks)

    #print db.updateTask(taskId, task_status='scheduled', otdb_id=723, task_type='PIPELINE')

    #resultPrint(db.getTasks)

    #for s in db.getSpecifications():
        #db.updateSpecification(s['id'], datetime.datetime.utcnow(), datetime.datetime.utcnow() + datetime.timedelta(hours=1))

    #claims = db.getResourceClaims()
    #for c in claims:
        #db.deleteResourceClaim(c['id'])
        ##resultPrint(db.getResourceClaims)

    #tasks = db.getTasks()
    #for t in tasks:
        #db.deleteTask(t['id'])
        ###resultPrint(db.getTasks)
        ###resultPrint(db.getResourceClaims)

    import random

    #for i in range(20):
        #taskId = db.insertTask(1234, 5678, 600, 0, 1)
        #for j in range(2*i):
            #rcId = db.insertResourceClaim(j, taskId, datetime.datetime.utcnow() + datetime.timedelta(hours=4*i), datetime.datetime.utcnow() + datetime.timedelta(hours=4*i+3.5), 0, 4, 10, 'einstein', -1)

        #time.sleep(0.5)

    #resultPrint(db.getTasks)
    #resultPrint(db.getResourceClaims)

    ts = db.getTaskStatuses()

    tasks = sorted(db.getTasks(), key=lambda x: x['id'])
    for t in tasks:
        db.updateTask(t['id'], task_status=ts[random.randint(0, len(ts)-1)]['id'])
        time.sleep(0.01)

    #db.commit()
