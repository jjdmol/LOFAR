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
        except psycopg2.IntegrityError as e:
            logger.error("Rolling back query=\'%s\' args=\'%s\' due to error: \'%s\'" % (query, ', '.join(str(x) for x in qargs), e))
            self.conn.rollback()
            return -1

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
        result = self._executeQuery(query, [status_name], fetch=_FETCH_ONE)

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
        tasks = list(self._executeQuery(query, fetch=_FETCH_ALL))
        predIds = self.getTaskPredecessorIds()
        succIds = self.getTaskSuccessorIds()

        for task in tasks:
            task['predecessor_ids'] = predIds.get(task['id'], [])
            task['successor_ids'] = succIds.get(task['id'], [])

        return tasks


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
            self.commit()
        return id

    def deleteTask(self, task_id, commit=True):
        query = '''DELETE FROM resource_allocation.task
                   WHERE resource_allocation.task.id = %s;'''

        self._executeQuery(query, [task_id])
        if commit:
            self.commit()
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
            self.commit()

        return self.cursor.rowcount > 0

    def getTaskPredecessorIds(self):
        query = '''SELECT * from resource_allocation.task_predecessor tp;'''
        items = list(self._executeQuery(query, fetch=_FETCH_ALL))
        predIdDict = {}
        for item in items:
            taskId = item['task_id']
            if taskId not in predIdDict:
                predIdDict[taskId] = []
            predIdDict[taskId].append(item['predecessor_id'])
        return predIdDict

    def getTaskSuccessorIds(self):
        query = '''SELECT * from resource_allocation.task_predecessor tp;'''
        items = list(self._executeQuery(query, fetch=_FETCH_ALL))
        succIdDict = {}
        for item in items:
            predId = item['predecessor_id']
            if predId not in succIdDict:
                succIdDict[predId] = []
            succIdDict[predId].append(item['task_id'])
        return succIdDict

    def getTaskPredecessorIdsForTask(self, task_id):
        query = '''SELECT * from resource_allocation.task_predecessor tp
        WHERE tp.task_id = %s;'''

        items = list(self._executeQuery(query, [task_id], fetch=_FETCH_ALL))
        return [x['predecessor_id'] for x in items]

    def getTaskSuccessorIdsForTask(self, task_):
        query = '''SELECT * from resource_allocation.task_predecessor tp
        WHERE tp.predecessor_id = %s;'''

        items = list(self._executeQuery(query, [task_], fetch=_FETCH_ALL))
        return [x['task_id'] for x in items]

    def insertTaskPredecessor(self, task_id, predecessor_id, commit=True):
        query = '''INSERT INTO resource_allocation.task_predecessor
        (task_id, predecessor_id)
        VALUES (%s, %s)
        RETURNING id;'''

        id = self._executeQuery(query, (task_id, predecessor_id), fetch=_FETCH_ONE)['id']
        if commit:
            self.commit()
        return id

    def insertTaskPredecessors(self, task_id, predecessor_ids, commit=True):
        ids = [self.insertTaskPredecessor(task_id, predecessor_id, false) for predecessor_id in predecessor_ids]
        if commit:
            self.commit()
        return ids

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
            self.commit()
        return id

    def deleteSpecification(self, specification_id, commit=True):
        query = '''DELETE FROM resource_allocation.specification
                   WHERE resource_allocation.specification.id = %s;'''

        self._executeQuery(query, [specification_id])
        if commit:
            self.commit()
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
            self.commit()

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

    def getResourceGroupMemberships(self):
        '''get a dict containing the resource->group and group->group relations'''
        query = '''select
                    prg.id as resource_group_parent_id,
                    prg.name as resource_group_parent_name,
                    crg.id as resource_group_id,
                    crg.name as resource_group_name
                    from virtual_instrument.resource_group_to_resource_group rg2rg
                    left join virtual_instrument.resource_group prg on rg2rg.parent_id = prg.id
                    inner join virtual_instrument.resource_group crg on rg2rg.child_id = crg.id
        '''
        relations = self._executeQuery(query, fetch=_FETCH_ALL)

        rg_items = {}
        # loop over list of relations
        # for each unique resource_group item create a dict, and add parent_ids to it
        for relation in relations:
            rg_item_id = relation['resource_group_id']
            if not rg_item_id in rg_items:
                rg_item = {k:relation[k] for k in ('resource_group_id', 'resource_group_name')}
                rg_item['child_ids'] = []
                rg_item['parent_ids'] = []
                rg_item['resource_ids'] = []
                rg_items[rg_item_id] = rg_item

            parent_id = relation['resource_group_parent_id']
            if parent_id != None:
                rg_items[rg_item_id]['parent_ids'].append(parent_id)

        # now that we have a full list (dict.values) of rg_items...
        # add a child_id reference to each item's parent
        # this gives us a full bidirectional graph
        for rg_item in rg_items.values():
            parentIds = rg_item['parent_ids']
            rg_item_id = rg_item['resource_group_id']
            for parentId in parentIds:
                if parentId in rg_items:
                    parentNode = rg_items[parentId]
                    parentNode['child_ids'].append(rg_item_id)

        query = '''select
                    prg.id as resource_group_parent_id,
                    prg.name as resource_group_parent_name,
                    cr.id as resource_id,
                    cr.name as resource_name
                    from virtual_instrument.resource_to_resource_group r2rg
                    left join virtual_instrument.resource_group prg on r2rg.parent_id = prg.id
                    inner join virtual_instrument.resource cr on r2rg.child_id = cr.id
        '''
        relations = self._executeQuery(query, fetch=_FETCH_ALL)

        r_items = {}
        # loop over list of relations
        # for each unique resource item create a dict, and add parent_ids to it
        for relation in relations:
            r_item_id = relation['resource_id']
            if not r_item_id in r_items:
                r_item = {k:relation[k] for k in ('resource_id', 'resource_name')}
                r_item['parent_group_ids'] = []
                r_items[r_item_id] = r_item

            parent_id = relation['resource_group_parent_id']
            if parent_id != None:
                r_items[r_item_id]['parent_group_ids'].append(parent_id)
                rg_items[parent_id]['resource_ids'].append(r_item_id)

        result = {'groups': rg_items,
                'resources': r_items }

        return result

    def getResourceClaims(self):
        query = '''SELECT * from resource_allocation.resource_claim_view'''

        return list(self._executeQuery(query, fetch=_FETCH_ALL))

    def getResourceClaim(self, id):
        query = '''SELECT * from resource_allocation.resource_claim_view rcv
        where rcv.id = %s;
        '''
        result = self._executeQuery(query, [id], fetch=_FETCH_ONE)

        return dict(result) if result else None

    def insertResourceClaim(self, resource_id, task_id, starttime, endtime, status, session_id, claim_size, username, user_id, nr_of_parts=1, commit=True):
        if status and isinstance(status, basestring):
            #convert status string to status.id
            status = self.getResourceClaimStatusId(status)

        query = '''INSERT INTO resource_allocation.resource_claim
        (resource_id, task_id, starttime, endtime, status_id, session_id, claim_size, nr_of_parts, username, user_id)
        VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
        RETURNING id;'''

        id = self._executeQuery(query, (resource_id, task_id, starttime, endtime, status, session_id, claim_size, nr_of_parts, username, user_id), fetch=_FETCH_ONE)['id']
        if commit:
            self.commit()
        return id

    def deleteResourceClaim(self, resource_claim_id, commit=True):
        query = '''DELETE FROM resource_allocation.resource_claim
                   WHERE resource_allocation.resource_claim.id = %s;'''

        self._executeQuery(query, [resource_claim_id])
        if commit:
            self.commit()
        return self.cursor.rowcount > 0

    def updateResourceClaim(self, resource_claim_id, resource_id=None, task_id=None, starttime=None, endtime=None, status=None, session_id=None, claim_size=None, nr_of_parts=None, username=None, user_id=None, commit=True):
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

        if nr_of_parts:
            fields.append('nr_of_parts')
            values.append(nr_of_parts)

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
            self.commit()

        return self.cursor.rowcount > 0

    def getResourceClaimsForTask(self, task_id):
        query = '''SELECT * from resource_allocation.resource_claim_view
        WHERE resource_allocation.resource_claim_view.task_id = %s'''

        return list(self._executeQuery(query, [task_id], fetch=_FETCH_ALL))

    def updateTaskAndResourceClaims(self, task_id, starttime=None, endtime=None, task_status=None, claim_status=None, session_id=None, username=None, user_id=None, commit=True):
        if claim_status and isinstance(claim_status, basestring):
            #convert status string to status.id
            claim_status = self.getResourceClaimStatusId(claim_status)

        updated = True

        if task_status:
            updated &= self.updateTask(task_id, task_status=task_status, commit=False)

        if starttime or endtime:
            task = self.getTask(task_id)
            updated &= self.updateSpecification(task['specification_id'], starttime=starttime, endtime=endtime, commit=False)

        fields = []
        values = []

        if starttime:
            fields.append('starttime')
            values.append(starttime)

        if endtime:
            fields.append('endtime')
            values.append(endtime)

        if claim_status:
            fields.append('status_id')
            values.append(claim_status)

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
        updated &= self.cursor.rowcount > 0

        if commit:
            self.commit()

        return updated

    def insertSpecificationAndTask(self, mom_id, otdb_id, task_status, task_type, starttime, endtime, content, commit=True):
        '''
        Insert a new specification and task in one transaction.
        Removes existing task with same otdb_id if present in the same transaction.
        '''
        try:
            task = self.getTask(otdb_id=otdb_id)

            if task:
                # delete old specification, task, and resource claims using cascaded delete
                self.deleteSpecification(task['specification_id'], False)

            specId = self.insertSpecification(starttime, endtime, content, False)
            taskId = self.insertTask(mom_id, otdb_id, task_status, task_type, specId, False)

            if specId >= 0 and taskId >= 0:
                if commit:
                    self.commit()
                return {'inserted': True, 'specification_id': specId, 'task_id': taskId}
        except:
            self.conn.rollback()

        return {'inserted': False, 'specification_id': None, 'task_id': None}


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
    #resultPrint(db.getResourceGroupMemberships)
    #resultPrint(db.getTasks)
    #print db.getTaskPredecessorIds()
    #print db.getTaskSuccessorIds()
    #resultPrint(db.getSpecifications)
    #resultPrint(db.getResourceClaims)

    #db.updateTaskAndResourceClaims(16, starttime= datetime.datetime.utcnow())

    #import pprint
    #pprint.pprint(db.getResourceGroupMemberships())

    #rcId = db.insertResourceClaim(1, 1, datetime.datetime.utcnow(), datetime.datetime.utcnow() + datetime.timedelta(hours=1), 'CLAIMED', 1, 10, 'einstein', -1, True)

    #resultPrint(db.getResourceClaims)

    #time.sleep(1)

    #rcId = db.updateResourceClaim(rcId, starttime=datetime.datetime.utcnow(), status='ALLOCATED')

    #resultPrint(db.getResourceClaims)

    #taskId = db.insertTask(1234, 5678, 'active', 'OBSERVATION', 1)

    #resultPrint(db.getTasks)

    #print db.updateTask(taskId, task_status='scheduled', otdb_id=723, task_type='PIPELINE')

    resultPrint(db.getSpecifications)
    resultPrint(db.getTasks)

    #raw_input()

    #for s in db.getSpecifications():
        #db.deleteSpecification(s['id'])

    #resultPrint(db.getSpecifications)

    result = db.insertSpecificationAndTask(1234, 5678, 600, 0, datetime.datetime.utcnow(), datetime.datetime.utcnow() + datetime.timedelta(hours=1), "", False)
    print result

    resultPrint(db.getSpecifications)
    resultPrint(db.getTasks)

    #raw_input()

    db.commit()

    resultPrint(db.getSpecifications)
    resultPrint(db.getTasks)

    #claims = db.getResourceClaims()
    #for c in claims:
        #db.deleteResourceClaim(c['id'])
        ##resultPrint(db.getResourceClaims)

    #predTaskId = None
    #for i in range(2):
        #specId = db.insertSpecification(datetime.datetime.utcnow(), datetime.datetime.utcnow() + datetime.timedelta(hours=4), "")
        #taskId = db.insertTask(1234+i, 5678+i, 600, 0, specId)

        #if predTaskId:
            #db.insertTaskPredecessor(taskId, predTaskId)
        #predTaskId = taskId

        #resources = db.getResources()
        #for r in resources:
            #rcId = db.insertResourceClaim(r['id'], taskId, datetime.datetime.utcnow() + datetime.timedelta(hours=2*i), datetime.datetime.utcnow() + datetime.timedelta(hours=2*(i+1)), 0, 1, 10, 'einstein', -1)


    ##tasks = db.getTasks()
    ##for t in tasks:
        ##db.deleteTask(t['id'])
        ####resultPrint(db.getTasks)
        ####resultPrint(db.getResourceClaims)

    #import random

    ##for i in range(1):
        ##taskId = db.insertTask(1234, 5678, 600, 0, 1)
        ##for j in range(2*i):
            ##rcId = db.insertResourceClaim(j, taskId, datetime.datetime.utcnow() + datetime.timedelta(hours=4*i), datetime.datetime.utcnow() + datetime.timedelta(hours=4*i+3.5), 0, 4, 10, 'einstein', -1)

        ##time.sleep(0.5)

    ##resultPrint(db.getTasks)
    ##resultPrint(db.getResourceClaims)

    #ts = db.getTaskStatuses()

    #tasks = sorted(db.getTasks(), key=lambda x: x['id'])
    #for t in tasks:
        #db.updateTask(t['id'], task_status=ts[random.randint(0, len(ts)-1)]['id'])
        #time.sleep(0.01)

    #db.commit()
