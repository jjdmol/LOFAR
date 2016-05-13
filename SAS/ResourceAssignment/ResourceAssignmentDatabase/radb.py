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
from datetime import datetime, timedelta
import time
import collections
from optparse import OptionParser
from lofar.common import dbcredentials
from lofar.common.util import to_csv_string
from lofar.common.datetimeutils import totalSeconds

logger = logging.getLogger(__name__)

_FETCH_NONE=0
_FETCH_ONE=1
_FETCH_ALL=2

class RADatabase:
    def __init__(self, dbcreds=None, log_queries=False):
        self.dbcreds = dbcreds
        self.conn = None
        self.cursor = None
        self.log_queries = log_queries

    def _connect(self):
        self.conn = None
        self.cursor = None

        self.conn = psycopg2.connect(host=self.dbcreds.host,
                                     user=self.dbcreds.user,
                                     password=self.dbcreds.password,
                                     database=self.dbcreds.database,
                                     connect_timeout=5)
        self.cursor = self.conn.cursor(cursor_factory = psycopg2.extras.RealDictCursor)

    def _queryAsSingleLine(self, query, qargs=None):
        line = ' '.join(query.replace('\n', ' ').split())
        if qargs:
            line = line % tuple(qargs)
        return line

    def _executeQuery(self, query, qargs=None, fetch=_FETCH_NONE):
        '''execute the query and reconnect upon OperationalError'''
        try:
            if self.log_queries:
                logger.info('executing query: %s' % self._queryAsSingleLine(query, qargs))

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
        except (psycopg2.IntegrityError, psycopg2.ProgrammingError, psycopg2.InternalError, psycopg2.DataError)as e:
            logger.error("Rolling back query=\'%s\' due to error: \'%s\'" % (self._queryAsSingleLine(query, qargs), e))
            self.rollback()
            return []

        if fetch == _FETCH_ONE:
            return self.cursor.fetchone()

        if fetch == _FETCH_ALL:
            return self.cursor.fetchall()


    def commit(self):
        logger.info('commit')
        self.conn.commit()

    def rollback(self):
        logger.info('rollback')
        self.conn.rollback()

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

    def getTasks(self, lower_bound=None, upper_bound=None):
        query = '''SELECT * from resource_allocation.task_view'''

        conditions = []
        qargs = []

        if lower_bound is not None:
            conditions.append('endtime >= %s')
            qargs.append(lower_bound)

        if upper_bound is not None:
            conditions.append('starttime <= %s')
            qargs.append(upper_bound)

        if conditions:
            query += ' WHERE ' + ' AND '.join(conditions)

        tasks = list(self._executeQuery(query, qargs, fetch=_FETCH_ALL))
        predIds = self.getTaskPredecessorIds()
        succIds = self.getTaskSuccessorIds()

        for task in tasks:
            task['predecessor_ids'] = predIds.get(task['id'], [])
            task['successor_ids'] = succIds.get(task['id'], [])
            task['duration'] = totalSeconds(task['endtime'] - task['starttime'])

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

        task = dict(result) if result else None

        if task:
            task['duration'] = totalSeconds(task['endtime'] - task['starttime'])

        return task

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
        logger.info('insertTask mom_id=%s, otdb_id=%s, task_status=%s, task_type=%s, specification_id=%s' %
                    (mom_id, otdb_id, task_status, task_type, specification_id))
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
        logger.info('deleteTask task_id=%s' % task_id)
        query = '''DELETE FROM resource_allocation.task
                   WHERE resource_allocation.task.id = %s;'''

        self._executeQuery(query, [task_id])
        if commit:
            self.commit()
        return self.cursor.rowcount > 0

    def updateTaskStatusForOtdbId(self, otdb_id, task_status, commit=True):
        '''converts task_status and task_type to id's in case one and/or the other are strings'''
        if task_status and isinstance(task_status, basestring):
            #convert task_status string to task_status.id
            task_status = self.getTaskStatusId(task_status)

        query = '''UPDATE resource_allocation.task
        SET (status_id) = (%s)
        WHERE resource_allocation.task.otdb_id = %s;'''

        self._executeQuery(query, [task_status, otdb_id])
        if commit:
            self.commit()

        return self.cursor.rowcount > 0

    def updateTask(self, task_id, mom_id=None, otdb_id=None, task_status=None, task_type=None, specification_id=None, commit=True):
        task_status, task_type = self._convertTaskTypeAndStatusToIds(task_status, task_type)

        fields = []
        values = []

        if mom_id is not None :
            fields.append('mom_id')
            values.append(mom_id)

        if otdb_id is not None :
            fields.append('otdb_id')
            values.append(otdb_id)

        if task_status is not None :
            fields.append('status_id')
            values.append(task_status)

        if task_type is not None :
            fields.append('type_id')
            values.append(task_type)

        if specification_id is not None :
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
        ids = [self.insertTaskPredecessor(task_id, predecessor_id, False) for predecessor_id in predecessor_ids]
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
        logger.info('insertSpecification starttime=%s, endtime=%s' % (starttime, endtime))
        query = '''INSERT INTO resource_allocation.specification
        (starttime, endtime, content)
        VALUES (%s, %s, %s)
        RETURNING id;'''

        id = self._executeQuery(query, (starttime, endtime, content), fetch=_FETCH_ONE)['id']
        if commit:
            self.commit()
        return id

    def deleteSpecification(self, specification_id, commit=True):
        logger.info('deleteSpecification specification_id=%s' % (specification_id))
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

        if content is not None :
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

        raise KeyError('No such unit: %s Valid values are: %s' % (unit_name, ', '.join(self.getUnitNames())))

    def getResources(self, resource_ids=None, resource_types=None, include_availability=False):
        if include_availability:
            query = '''SELECT * from resource_monitoring.resource_view'''
        else:
            query = '''SELECT * from virtual_instrument.resource_view'''

        conditions = []
        qargs = []

        if resource_ids is not None:
            if isinstance(resource_ids, int): # just a single id
                conditions.append('id = %s')
                qargs.append(resource_ids)
            elif len(resource_ids) > 0: # a list of id's
                conditions.append('id in %s')
                qargs.append(tuple(resource_ids))

        if resource_types is not None:
            if isinstance(resource_types, basestring):
                resource_types = [resource_types]
            elif not isinstance(resource_types, collections.Iterable):
                resource_types = [resource_types]

            # convert any resource_type name to id
            resource_type_names = set([x for x in resource_types if isinstance(x, basestring)])
            if resource_type_names:
                resource_type_name_to_id = {x['name']:x['id'] for x in self.getResourceTypes()}
                resource_types = [resource_type_name_to_id[x] if isinstance(x, basestring) else x
                                  for x in resource_types]

            conditions.append('type_id in %s')
            qargs.append(tuple(resource_types))

        if conditions:
            query += ' WHERE ' + ' AND '.join(conditions)

        return list(self._executeQuery(query, qargs, fetch=_FETCH_ALL))

    def updateResourceAvailability(self, resource_id, active=None, available_capacity=None, total_capacity=None, commit=True):
        if active is not None:
            query = '''UPDATE resource_monitoring.resource_availability
            SET (available) = (%s)
            WHERE resource_id = %s;'''
            self._executeQuery(query, (active, resource_id))

        if available_capacity is not None and total_capacity is not None:
            query = '''UPDATE resource_monitoring.resource_capacity
            SET (available, total) = (%s, %s)
            WHERE resource_id = %s;'''
            self._executeQuery(query, (available_capacity, total_capacity, resource_id))
        elif available_capacity is not None:
            query = '''UPDATE resource_monitoring.resource_capacity
            SET (available) = (%s)
            WHERE resource_id = %s;'''
            self._executeQuery(query, (available_capacity, resource_id))
        elif total_capacity is not None:
            query = '''UPDATE resource_monitoring.resource_capacity
            SET (total) = (%s)
            WHERE resource_id = %s;'''
            self._executeQuery(query, (total_capacity, resource_id))

        if active is not None or available_capacity is not None or total_capacity is not None:
            affectedClaims = self.getResourceClaims(resource_ids=resource_id)
            logger.info('updateResourceAvailability: affectedClaims=%s' % affectedClaims)
            self.validateResourceClaimsStatus(affectedClaims, False)

        if commit:
            self.commit()

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

    def getResourceClaimPropertyTypes(self):
        query = '''SELECT * from resource_allocation.resource_claim_property_type;'''

        return list(self._executeQuery(query, fetch=_FETCH_ALL))

    def getResourceClaimPropertyTypeNames(self):
        return [x['name'] for x in self.getResourceClaimPropertyTypes()]

    def getResourceClaimPropertyTypeId(self, type_name):
        query = '''SELECT id from resource_allocation.resource_claim_property_type
                   WHERE name = %s;'''
        result = self._executeQuery(query, [type_name], fetch=_FETCH_ONE)

        if result:
            return result['id']

        raise KeyError('No such resource_claim_property_type: %s Valid values are: %s' % (type_name, ', '.join(self.getResourceClaimPropertyTypeNames())))

    def getResourceClaimProperties(self, claim_ids=None, task_id=None):
        query = '''SELECT rcpv.id, rcpv.resource_claim_id, rcpv.value, rcpv.type_id, rcpv.type_name, sap.number as sap_nr
                   FROM resource_allocation.resource_claim_property_view rcpv
                   LEFT JOIN resource_allocation.sap sap on rcpv.sap_id = sap.id'''

        conditions = []
        qargs = []

        if claim_ids is not None:
            if isinstance(claim_ids, int): # just a single id
                conditions.append('rcpv.resource_claim_id = %s')
                qargs.append(claim_ids)
            elif len(claim_ids) > 0: # list of id's
                conditions.append('rcpv.resource_claim_id in %s')
                qargs.append(tuple(claim_ids))

        if task_id is not None:
            query += ' JOIN resource_allocation.resource_claim rc on rc.id = rcpv.resource_claim_id'
            conditions.append('rc.task_id = %s')
            qargs.append(task_id)

        if conditions:
            query += ' WHERE ' + ' AND '.join(conditions)

        properties = list(self._executeQuery(query, qargs, fetch=_FETCH_ALL))
        for p in properties:
            if p['sap_nr'] is None:
                del p['sap_nr']

        return properties

    def insertResourceClaimProperty(self, claim_id, property_type, value, commit=True):
        return self.insertResourceClaimProperties([(claim_id, property_type, value)], commit)

    def insertResourceClaimProperties(self, props, commit=True):
        if not props:
            return []

        # first insert unique sap numbers
        claim_sap_nrs = list(set([(p[0], p[3]) for p in props if p[3] is not None]))
        sap_ids = self.insertSAPNumbers(claim_sap_nrs, False)

        if sap_ids == None:
            return None

        # make sap_nr to sap_id mapping per claim_id
        claim_id2sap_nr2sap_id = {}
        for claim_sap_nr,sap_id in zip(claim_sap_nrs, sap_ids):
            claim_id = claim_sap_nr[0]
            sap_nr = claim_sap_nr[1]
            if claim_id not in claim_id2sap_nr2sap_id:
                claim_id2sap_nr2sap_id[claim_id] = {}
            claim_id2sap_nr2sap_id[claim_id][sap_nr] = sap_id

        logger.info('insertResourceClaimProperties inserting %d properties' % len(props))

        # convert all property type strings to id's
        type_strings = set([p[1] for p in props if isinstance(p[1], basestring)])
        type_string2id = {t:self.getResourceClaimPropertyTypeId(t) for t in type_strings}

        # finally we have all the info we need,
        # so we can build the bulk property insert query
        insert_values = ','.join(self.cursor.mogrify('(%s, %s, %s, %s)',
                                                     (p[0],
                                                      type_string2id[p[1]] if
                                                      isinstance(p[1], basestring) else p[1],
                                                      p[2],
                                                      claim_id2sap_nr2sap_id[p[0]].get(p[3]) if
                                                      p[0] in claim_id2sap_nr2sap_id else None))
                                                     for p in props)

        query = '''INSERT INTO resource_allocation.resource_claim_property
        (resource_claim_id, type_id, value, sap_id)
        VALUES {values}
        RETURNING id;'''.format(values=insert_values)

        ids = [x['id'] for x in self._executeQuery(query, fetch=_FETCH_ALL)]

        if [x for x in ids if x < 0]:
            logger.error("One or more properties could not be inserted. Rolling back.")
            self.rollback()
            return None

        if commit:
            self.commit()
        return ids

    def insertSAPNumbers(self, sap_numbers, commit=True):
        if not sap_numbers:
            return []

        logger.info('insertSAPNumbers inserting %d sap numbers' % len(sap_numbers))

        insert_values = ','.join(self.cursor.mogrify('(%s, %s)', rcid_sapnr) for rcid_sapnr in sap_numbers)

        query = '''INSERT INTO resource_allocation.sap
        (resource_claim_id, number)
        VALUES {values}
        RETURNING id;'''.format(values=insert_values)

        sap_ids = [x['id'] for x in self._executeQuery(query, fetch=_FETCH_ALL)]

        if [x for x in sap_ids if x < 0]:
            logger.error("One or more sap_nr's could not be inserted. Rolling back.")
            self.rollback()
            return None

        if commit:
            self.commit()

        return sap_ids

    def getResourceClaims(self, claim_ids=None, lower_bound=None, upper_bound=None, resource_ids=None, task_ids=None, status=None, resource_type=None, extended=False, include_properties=False):
        extended |= resource_type is not None
        query = '''SELECT * from %s''' % ('resource_allocation.resource_claim_extended_view' if extended else 'resource_allocation.resource_claim_view')

        if lower_bound and not isinstance(lower_bound, datetime):
            lower_bound = None

        if upper_bound and not isinstance(upper_bound, datetime):
            upper_bound = None

        if status is not None and isinstance(status, basestring):
            #convert status string to status.id
            status = self.getResourceClaimStatusId(status)

        if resource_type is not None and isinstance(resource_type, basestring):
            #convert resource_type string to resource_type.id
            resource_type = self.getResourceTypeId(resource_type)

        conditions = []
        qargs = []

        if claim_ids is not None:
            if isinstance(claim_ids, int): # just a single id
                conditions.append('id = %s')
                qargs.append(claim_ids)
            elif len(claim_ids) > 0: # list of id's
                conditions.append('id in %s')
                qargs.append(tuple(claim_ids))

        if lower_bound:
            conditions.append('endtime >= %s')
            qargs.append(lower_bound)

        if upper_bound:
            conditions.append('starttime <= %s')
            qargs.append(upper_bound)

        if resource_ids is not None:
            if isinstance(resource_ids, int): # just a single id
                conditions.append('resource_id = %s')
                qargs.append(resource_ids)
            elif len(resource_ids) > 0: # list of id's
                conditions.append('resource_id in %s')
                qargs.append(tuple(resource_ids))

        if task_ids is not None:
            #if task_id is normal positive we do a normal inclusive filter
            #if task_id is negative we do an exclusive filter
            if isinstance(task_ids, int): # just a single id
                conditions.append('task_id = %s' if task_ids >= 0 else 'task_id != %s')
                qargs.append(abs(task_ids))
            else:
                inclusive_task_ids = [t for t in task_ids if t >= 0]
                exclusive_task_ids = [-t for t in task_ids if t < 0]

                if inclusive_task_ids:
                    conditions.append('task_id in %s')
                    qargs.append(tuple(inclusive_task_ids))

                if exclusive_task_ids:
                    conditions.append('task_id not in %s')
                    qargs.append(tuple(exclusive_task_ids))

        if status is not None:
            conditions.append('status_id = %s')
            qargs.append(status)

        if resource_type is not None and extended:
            conditions.append('resource_type_id = %s')
            qargs.append(resource_type)

        if conditions:
            query += ' WHERE ' + ' AND '.join(conditions)

        claims = list(self._executeQuery(query, qargs, fetch=_FETCH_ALL))

        if self.log_queries:
            logger.info("found %s claims" % len(claims))

        if include_properties and claims:
            claimDict = {c['id']:c for c in claims}
            claim_ids = claimDict.keys()
            properties = self.getResourceClaimProperties(claim_ids=claim_ids)
            for p in properties:
                try:
                    claim = claimDict[p['resource_claim_id']]
                    del p['resource_claim_id']
                    if 'sap_nr' in p:
                        if not 'saps' in claim:
                            claim['saps'] = {}
                        if not p['sap_nr'] in claim['saps']:
                            claim['saps'][p['sap_nr']] = []
                        claim['saps'][p['sap_nr']].append(p)
                        del p['sap_nr']
                    else:
                        if not 'properties' in claim:
                            claim['properties'] = []
                        claim['properties'].append(p)
                except KeyError:
                    pass

            for claim in claims:
                if 'saps' in claim:
                    claim['saps'] = [{'sap_nr':sap_nr, 'properties':props} for sap_nr, props in claim['saps'].items()]

        return claims

    def getResourceClaim(self, id):
        query = '''SELECT * from resource_allocation.resource_claim_view rcv
        where rcv.id = %s;
        '''
        result = self._executeQuery(query, [id], fetch=_FETCH_ONE)

        return dict(result) if result else None

    def insertResourceClaim(self, resource_id, task_id, starttime, endtime, status, session_id, claim_size, username, user_id, properties=None, commit=True):
        # for code re-use:
        # put the one claim in a list
        # and do a bulk insert of the one-item-list
        claim = {'resource_id':resource_id,
                'starttime':starttime,
                'endtime':endtime,
                'status':status,
                'claim_size':1}

        if properties:
            claim['properties'] = properties

        result = self.insertResourceClaims(task_id, [claim], session_id, username, user_id, commit)
        if result:
            return result[0]

    def insertResourceClaims(self, task_id, claims, session_id, username, user_id, commit=True):
        '''bulk insert of resource claims for a task
        claims is a list of dicts. Each dict is a claim for one resource containing the fields: starttime, endtime, status, claim_size
        '''
        logger.info('insertResourceClaims for task_id=%d with %d claim(s)' % (task_id, len(claims)))

        status_strings = set([c['status'] for c in claims if isinstance(c['status'], basestring)])
        if status_strings:
            status_string2id = {s:self.getResourceClaimStatusId(s) for s in status_strings}
            for c in claims:
                if isinstance(c['status'], basestring):
                    c['status'] = status_string2id[c['status']]

        try:
            claim_values = [(c['resource_id'], task_id, c['starttime'], c['endtime'],
                            c['status'], session_id, c['claim_size'],
                            username, user_id) for c in claims]
        except Exception as e:
            logger.error("Invalid claim dict, rolling back. %s" % e)
            self.rollback()
            return None

        try:
            # use psycopg2 mogrify to parse and build the insert values
            # this way we can insert many values in one insert query, returning the id's of each inserted row.
            # this is much faster than psycopg2's executeMany method
            insert_values = ','.join(self.cursor.mogrify('(%s, %s, %s, %s, %s, %s, %s, %s, %s)', cv) for cv in claim_values)
        except Exception as e:
            logger.error("Invalid input, rolling back: %s\n%s" % (claim_values, e))
            self.rollback()
            return None

        query = '''INSERT INTO resource_allocation.resource_claim
        (resource_id, task_id, starttime, endtime, status_id, session_id, claim_size, username, user_id)
        VALUES {values}
        RETURNING id;'''.format(values=insert_values)

        claimIds = [x['id'] for x in self._executeQuery(query, fetch=_FETCH_ALL)]

        if not claimIds or [x for x in claimIds if x < 0]:
            logger.error("One or more claims cloud not be inserted. Rolling back.")
            self.rollback()
            return None

        # gather all properties for all claims
        # store them as list of (claim_id, prop_type, prop_value, sap_nr) tuples
        properties = []
        for claim_id, claim in zip(claimIds, claims):
            if 'properties' in claim and len(claim['properties']) > 0:
                claim_props = [(claim_id, p['type'], p['value'], p.get('sap_nr')) for p in claim['properties']]
                properties += claim_props

        if properties:
            property_ids = self.insertResourceClaimProperties(properties, False)
            if property_ids == None:
                return None

        # get the claims as they were inserted
        # and validate them against all other claims
        insertedClaims = self.getResourceClaims(claim_ids=claimIds)
        self.validateResourceClaimsStatus(insertedClaims, False)

        if commit:
            self.commit()

        logger.info('inserted %d resource claim(s) for task_id=%d' % (len(claimIds), task_id))
        return claimIds

    def deleteResourceClaim(self, resource_claim_id, commit=True):
        query = '''DELETE FROM resource_allocation.resource_claim
                   WHERE resource_allocation.resource_claim.id = %s;'''

        self._executeQuery(query, [resource_claim_id])
        if commit:
            self.commit()
        return self.cursor.rowcount > 0

    def updateResourceClaim(self, resource_claim_id, resource_id=None, task_id=None, starttime=None, endtime=None, status=None, session_id=None, claim_size=None, username=None, user_id=None, validate=True, commit=True):
        return self.updateResourceClaims([resource_claim_id], resource_id, task_id, starttime, endtime, status, session_id, claim_size, username, user_id, validate, commit)

    def updateResourceClaims(self, resource_claim_ids, resource_id=None, task_id=None, starttime=None, endtime=None, status=None, session_id=None, claim_size=None, username=None, user_id=None, validate=True, commit=True):
        if not resource_claim_ids:
            return

        logger.info("updateResourceClaims for %d claims" % len(resource_claim_ids))

        if validate:
            claimsBeforeUpdate = self.getResourceClaims(resource_claim_ids)

        if status is not None and isinstance(status, basestring):
            #convert status string to status.id
            status = self.getResourceClaimStatusId(status)

        fields = []
        values = []

        if resource_id is not None:
            fields.append('resource_id')
            values.append(resource_id)

        if task_id is not None:
            fields.append('task_id')
            values.append(task_id)

        if starttime:
            fields.append('starttime')
            values.append(starttime)

        if endtime:
            fields.append('endtime')
            values.append(endtime)

        if status is not None:
            fields.append('status_id')
            values.append(status)

        if session_id is not None:
            fields.append('session_id')
            values.append(session_id)

        if claim_size is not None:
            fields.append('claim_size')
            values.append(claim_size)

        if username is not None:
            fields.append('username')
            values.append(username)

        if user_id is not None:
            fields.append('user_id')
            values.append(user_id)

        values.append(tuple(resource_claim_ids))

        query = '''UPDATE resource_allocation.resource_claim
        SET ({fields}) = ({value_placeholders})
        WHERE resource_allocation.resource_claim.id in {rc_ids_placeholder};'''.format(fields=', '.join(fields),
                                                                                         value_placeholders=', '.join('%s' for x in fields),
                                                                                         rc_ids_placeholder='%s')

        self._executeQuery(query, values)

        if validate:
            self.validateResourceClaimsStatus(self.getResourceClaims(resource_claim_ids), commit=False)
            self.validateResourceClaimsStatusForMovedClaims(claimsBeforeUpdate, commit=False)

        if commit:
            self.commit()

        return self.cursor.rowcount > 0


    def updateTaskAndResourceClaims(self, task_id, starttime=None, endtime=None, task_status=None, claim_status=None, session_id=None, username=None, user_id=None, commit=True):
        claimsBeforeUpdate = self.getResourceClaims(task_ids=task_id)

        if claim_status is not None and isinstance(claim_status, basestring):
            #convert status string to status.id
            claim_status = self.getResourceClaimStatusId(claim_status)

        updated = True

        if task_status is not None :
            updated &= self.updateTask(task_id, task_status=task_status, commit=False)

        if starttime or endtime:
            task = self.getTask(task_id)
            updated &= self.updateSpecification(task['specification_id'], starttime=starttime, endtime=endtime, commit=False)

        if (starttime or endtime or claim_status is not None or session_id is not None or
            username is not None or user_id is not None):
            # update the claims as well
            # updateResourceClaims also validates the updated claims
            claim_ids = [c['id'] for c in claimsBeforeUpdate]
            updated &= self.updateResourceClaims(claim_ids,
                                                 starttime=starttime,
                                                 endtime=endtime,
                                                 status=claim_status,
                                                 session_id=session_id,
                                                 username=username, user_id=user_id,
                                                 validate=True,
                                                 commit=False)

            # because we moved or changed the status of these claims,
            # validate the claims 'underneath' which may have been in conflict
            # and which now may be 'freed'
            self.validateResourceClaimsStatusForMovedClaims(claimsBeforeUpdate, commit=False)

        if commit:
            self.commit()

        return updated

    def validateResourceClaimsStatusForMovedClaims(self, moved_claims, commit=True):
        if not moved_claims:
            return

        moved_claim_ids = set([c['id'] for c in moved_claims])
        resource_ids = list(set([c['resource_id'] for c in moved_claims]))
        min_starttime = min(c['starttime'] for c in moved_claims)
        max_endtime = min(c['endtime'] for c in moved_claims)

        otherClaims = [c for c in self.getResourceClaims(resource_ids=resource_ids,
                                                         lower_bound=min_starttime,
                                                         upper_bound=max_endtime)
                       if c['id'] not in moved_claim_ids]

        if otherClaims:
            logger.info("validating %d claims which may have been freed" % len(otherClaims))
            self.validateResourceClaimsStatus(otherClaims, commit=commit)

    def validateResourceClaimsStatusForTask(self, task_id, commit=True):
        claims = self.getResourceClaims(task_ids=task_id)
        return self.validateResourceClaimsStatus(claims, commit)

    def validateResourceClaimsStatus(self, claims, commit=True):
        # TODO: this should be a trigger function in the radb itself
        if not claims:
            return

        claimDict = {c['id']:c for c in claims}
        resource_ids = list(set([c['resource_id'] for c in claims]))
        task_ids = list(set(c['task_id'] for c in claims))
        min_starttime = min(c['starttime'] for c in claims)
        max_endtime = min(c['endtime'] for c in claims)

        logger.info("validating status of %d resource claim(s) for task_id(s) %s" % (len(claims), to_csv_string(task_ids)))

        # cache status_id's for conflift and claimed
        claimsStatuses = self.getResourceClaimStatuses()
        conflistStatusId = next(cs['id'] for cs in claimsStatuses if cs['name'] == 'conflict')
        claimedStatusId = next(cs['id'] for cs in claimsStatuses if cs['name'] == 'claimed')
        allocatedStatusId = next(cs['id'] for cs in claimsStatuses if cs['name'] == 'allocated')

        # 'result' dict for new statuses for claims
        newClaimStatuses = {conflistStatusId:[], claimedStatusId:[]}

        #get all resources including availability
        #convert to id->resource dict
        resources = self.getResources(include_availability=True)
        resources = {r['id']:r for r in resources}

        # get all claims for given resource_ids, within the given timeframe
        otherClaims = self.getResourceClaims(resource_ids=resource_ids,
                                             lower_bound=min_starttime,
                                             upper_bound=max_endtime)

        #group claims per resource
        resource2otherClaims = {r_id:[] for r_id in resource_ids}
        for claim in otherClaims:
            if claim['id'] not in claimDict:
                resource2otherClaims[claim['resource_id']].append(claim)

        for claim_id, claim in claimDict.items():
            claimSize = claim['claim_size']
            resource_id = claim['resource_id']
            resource = resources[resource_id]
            resourceOtherClaims = resource2otherClaims[resource_id]
            totalOtherClaimSize = sum(c['claim_size'] for c in resourceOtherClaims)

            logger.info('resource_id=%s claimSize=%s totalOtherClaimSize=%s total=%s available_capacity=%s' %
                        (resource_id,
                         claimSize,
                         totalOtherClaimSize,
                         totalOtherClaimSize + claimSize,
                         resource['available_capacity']))

            if totalOtherClaimSize + claimSize >= resource['available_capacity']:
                newClaimStatuses[conflistStatusId].append(claim_id)
            elif claim['status_id'] != allocatedStatusId:
                newClaimStatuses[claimedStatusId].append(claim_id)

        if newClaimStatuses:
            for status_id, claim_ids in newClaimStatuses.items():
                changed_claim_ids = [c_id for c_id in claim_ids if claimDict[c_id]['status_id'] != status_id]
                self.updateResourceClaims(resource_claim_ids=changed_claim_ids, status=status_id, validate=False)

        # update each task
        # depending on the task's claims in conflict/other status
        for task_id in task_ids:
            if self.getResourceClaims(task_ids=task_id, status=conflistStatusId):
                # if any claims in conflict -> task: conflict
                self.updateTask(task_id=task_id, task_status='conflict', commit=False)
            elif self.getTask(task_id)['status'] == 'conflict':
                # if no claims in conflict and task was in conflict -> task: prescheduled
                self.updateTask(task_id=task_id, task_status='prescheduled', commit=False)

        if commit:
            self.commit()

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
            self.rollback()

        return {'inserted': False, 'specification_id': None, 'task_id': None}

    def getTaskConflictReasons(self, task_ids=None):
        query = '''SELECT * from resource_allocation.task_conflict_reason_view'''

        conditions = []
        qargs = []

        if task_ids is not None:
            if isinstance(task_ids, int): # just a single id
                conditions.append('task_id = %s')
                qargs.append(task_ids)
            elif len(task_ids) > 0: # list of id's
                conditions.append('task_id in %s')
                qargs.append(tuple(task_ids))

        if conditions:
            query += ' WHERE ' + ' AND '.join(conditions)

        conflict_reasons = list(self._executeQuery(query, qargs, fetch=_FETCH_ALL))
        return conflict_reasons

    def insertTaskConflicts(self, task_id, conflict_reason_ids, commit=True):
        if not self.cursor:
            self._connect()

        insert_values = ','.join(self.cursor.mogrify('(%s, %s)', (task_id, cr_id)) for cr_id in conflict_reason_ids)

        query = '''INSERT INTO resource_allocation.task_conflict_reason
        (task_id, conflict_reason_id)
        VALUES {values}
        RETURNING id;'''.format(values=insert_values)

        ids = [x['id'] for x in self._executeQuery(query, fetch=_FETCH_ALL)]

        if [x for x in ids if x < 0]:
            logger.error("One or more conflict reasons could not be inserted. Rolling back.")
            self.rollback()
            return None

        if commit:
            self.commit()
        return ids


    def getResourceClaimConflictReasons(self, claim_ids=None, resource_ids=None, task_ids=None):
        query = '''SELECT * from resource_allocation.resource_claim_conflict_reason_view'''

        conditions = []
        qargs = []

        if claim_ids is not None:
            if isinstance(claim_ids, int): # just a single id
                conditions.append('id = %s')
                qargs.append(claim_ids)
            elif len(claim_ids) > 0: # list of id's
                conditions.append('id in %s')
                qargs.append(tuple(claim_ids))

        if resource_ids is not None:
            if isinstance(resource_ids, int): # just a single id
                conditions.append('resource_id = %s')
                qargs.append(resource_ids)
            elif len(resource_ids) > 0: # list of id's
                conditions.append('resource_id in %s')
                qargs.append(tuple(resource_ids))

        if task_ids is not None:
            if isinstance(task_ids, int): # just a single id
                conditions.append('task_id = %s')
                qargs.append(task_ids)
            elif len(task_ids) > 0: # list of id's
                conditions.append('task_id in %s')
                qargs.append(tuple(task_ids))

        if conditions:
            query += ' WHERE ' + ' AND '.join(conditions)

        conflict_reasons = list(self._executeQuery(query, qargs, fetch=_FETCH_ALL))
        return conflict_reasons

    def insertResourceClaimConflicts(self, claim_id, conflict_reason_ids, commit=True):
        if not self.cursor:
            self._connect()

        insert_values = ','.join(self.cursor.mogrify('(%s, %s)', (claim_id, cr_id)) for cr_id in conflict_reason_ids)

        query = '''INSERT INTO resource_allocation.resource_claim_conflict_reason
        (resource_claim_id, conflict_reason_id)
        VALUES {values}
        RETURNING id;'''.format(values=insert_values)

        ids = [x['id'] for x in self._executeQuery(query, fetch=_FETCH_ALL)]

        if [x for x in ids if x < 0]:
            logger.error("One or more conflict reasons could not be inserted. Rolling back.")
            self.rollback()
            return None

        if commit:
            self.commit()
        return ids

    def getResourceUsages(self, claim_ids=None, lower_bound=None, upper_bound=None, resource_ids=None, task_ids=None, status=None, resource_type=None):
        claims = self.getResourceClaims(claim_ids=claim_ids, lower_bound=lower_bound, upper_bound=upper_bound, resource_ids=resource_ids, task_ids=task_ids, status=status, resource_type=resource_type)

        #gather start/end events per resource per claim_status
        eventsDict = {}
        for claim in claims:
            event_start = { 'timestamp': claim['starttime'], 'delta': claim['claim_size'] }
            event_end = { 'timestamp': claim['endtime'], 'delta': -claim['claim_size'] }

            resource_id = claim['resource_id']
            status = claim['status']
            if not resource_id in eventsDict:
                eventsDict[resource_id] = {}

            if not status in eventsDict[resource_id]:
                eventsDict[resource_id][status] = []

            eventsDict[resource_id][status].append(event_start)
            eventsDict[resource_id][status].append(event_end)

        # sort events per resource by event timestamp ascending
        # and integrate event delta's into usage
        all_usages = {}
        for resource_id, status_events in eventsDict.items():
            usages = {}
            for status, events in status_events.items():
                if events:
                    usages[status] = []
                    prev_usage = { 'timestamp': datetime(1971, 1, 1), 'value': 0 }

                    events = sorted(events, key=lambda event: event['timestamp'])

                    for event in events:
                        prev_value = prev_usage['value']
                        prev_timestamp = prev_usage['timestamp']
                        new_value = prev_value + event['delta']
                        usage = { 'timestamp': event['timestamp'], 'value': new_value }

                        if prev_timestamp == event['timestamp']:
                            usages[status][-1]['value'] += event['delta']
                        else:
                            usages[status].append(usage)

                        prev_usage = usage

            resource_usages = { 'resource_id': resource_id, 'usages': usages }
            all_usages[resource_id] = resource_usages

        resource_ids = all_usages.keys()
        resources = self.getResources(resource_ids=resource_ids, include_availability=True)

        for resource in resources:
            resource_id = resource['id']
            if resource_id in all_usages:
                resource_usages = all_usages[resource_id]
                # copy resource capacities
                for item in ['total_capacity', 'available_capacity', 'used_capacity']:
                    try:
                        resource_usages[item] = 0
                        if item in resource:
                            resource_usages[item] = resource[item]
                            if item == 'used_capacity':
                                # and compute unaccounted-for usage,
                                # which is the actual used_capacity minus the currently allocated total claim size
                                # defaults to used_capacity if no currently allocated total claim size
                                resource_usages['misc_used_capacity'] = resource['used_capacity']
                                utcnow = datetime.utcnow()
                                allocated_usages = resource_usages['usages'].get('allocated', [])
                                past_allocated_usages = sorted([au for au in allocated_usages if au['timestamp'] <= utcnow])
                                if past_allocated_usages:
                                    currently_allocated_usage = past_allocated_usages[-1]
                                    resource_usages['misc_used_capacity'] = resource['used_capacity'] - currently_allocated_usage['value']
                    except Exception as e:
                        logger.error(e)

        all_usages_list = all_usages.values()
        return all_usages_list


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

    logger.info("Using dbcreds: %s" % dbcreds.stringWithHiddenPassword())

    db = RADatabase(dbcreds=dbcreds, log_queries=True)

    def resultPrint(method):
        print '\n-- ' + str(method.__name__) + ' --'
        print '\n'.join([str(x) for x in method()])

    resultPrint(db.getTasks)

    for t in db.getTasks():
        print db.getTask(t['id'])

    exit()

    #print db.getResourceClaims(task_id=440)
    #print
    #print db.getResourceClaims(lower_bound=datetime.utcnow() + timedelta(days=9))
    #print
    #print db.getResourceClaims(upper_bound=datetime.utcnow() + timedelta(days=19))
    #print
    #print db.getResourceClaims(status='allocated')
    #print
    #print db.getResourceClaims(status='claimed')
    #print
    #print db.getResourceClaims(resource_type='storage')

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

    #claims = db.getResourceClaims()
    #db.updateTaskAndResourceClaims(claims[0]['task_id'], starttime=claims[1]['starttime'], endtime=claims[1]['endtime'])
    #print
    #print db.getResourceClaims()

    #resultPrint(db.getResourceClaims)


    db.updateResourceAvailability(0, available_capacity=2)
    exit(0)

    import pprint
    pprint.pprint(db.getTaskConflictReasons())
    db.updateTask(21, task_status='conflict')
    db.insertTaskConflicts(21, [1, 2, 3])
    pprint.pprint(db.getTaskConflictReasons())
    db.updateTask(21, task_status='scheduled')
    pprint.pprint(db.getTaskConflictReasons())
    db.insertTaskConflicts(21, [1, 2, 3])
    pprint.pprint(db.getTaskConflictReasons())

    pprint.pprint(db.getResourceClaimConflictReasons(task_ids=22))
    #pprint.pprint(db.getResourceUsages())

    exit(0)

    for s in db.getSpecifications():
        db.deleteSpecification(s['id'])

    resources = db.getResources()

    #task_id = db.insertSpecificationAndTask(1234, 5678, 600, 0, datetime.utcnow(), datetime.utcnow() + timedelta(hours=1), "", False)['task_id']
    #task = db.getTask(task_id)

    #claim = {'resource_id':resources[0]['id'],
            #'starttime':task['starttime'],
            #'endtime':task['endtime'],
            #'status':'claimed',
            #'claim_size':1}
    #db.insertResourceClaims(task_id, [claim], 1, 'anonymous', -1, False)

    #claim = {'resource_id':resources[1]['id'],
            #'starttime':task['starttime'],
            #'endtime':task['endtime'],
            #'status':'claimed',
            #'claim_size':1,
            #'properties':[{'type':'nr_of_is_files', 'value':10},{'type':'nr_of_cs_files', 'value':20}]}
    #db.insertResourceClaims(task_id, [claim], 1, 'anonymous', -1, False)

    #claim = {'resource_id':resources[2]['id'],
            #'starttime':task['starttime'],
            #'endtime':task['endtime'],
            #'status':'claimed',
            #'claim_size':1,
            #'properties':[{'type':'nr_of_is_files', 'value':10, 'sap_nr':0 },
                          #{'type':'nr_of_cs_files', 'value':20, 'sap_nr':0},
                          #{'type':'nr_of_uv_files', 'value':30, 'sap_nr':1},]}
    #db.insertResourceClaims(task_id, [claim], 1, 'anonymous', -1, False)

    #claim = {'resource_id':resources[3]['id'],
            #'starttime':task['starttime'],
            #'endtime':task['endtime'],
            #'status':'claimed',
            #'claim_size':1,
            #'properties':[{'type':'nr_of_is_files', 'value':15 },
                          #{'type':'nr_of_cs_files', 'value':25 },
                          #{'type':'nr_of_is_files', 'value':10, 'sap_nr':0 },
                          #{'type':'nr_of_cs_files', 'value':20, 'sap_nr':0},
                          #{'type':'nr_of_uv_files', 'value':30, 'sap_nr':1},]}
    #db.insertResourceClaims(task_id, [claim], 1, 'anonymous', -1, False)

    #db.commit()
    #import pprint
    #pprint.pprint(db.getResourceClaims(include_properties=True))
    #print '\n'.join(str(x) for x in db.getResourceClaims(include_properties=True))


    #c = db.cursor
    #query = '''INSERT INTO resource_allocation.resource_claim
    #(resource_id, task_id, starttime, endtime, status_id, session_id, claim_size, username, user_id)
    #VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s)
    #RETURNING id;'''

    #print c.mogrify(query, [(0, 0, datetime.utcnow(), datetime.utcnow(), 200, 1, 1, 'piet', 1)])
    #exit(0)


    #for s in db.getSpecifications():
        #db.deleteSpecification(s['id'])

    from lofar.common.datetimeutils import totalSeconds
    begin = datetime.utcnow()
    for i in range(5):
        stepbegin = datetime.utcnow()
        result = db.insertSpecificationAndTask(1234+i, 5678+i, 350, 0, datetime.utcnow() + timedelta(hours=1.25*i), datetime.utcnow() + timedelta(hours=1.25*i+1), "", False)

        #resultPrint(db.getSpecifications)
        #resultPrint(db.getTasks)

        task = db.getTask(result['task_id'])

        claims = [{'resource_id':r['id'],
                'starttime':task['starttime'],
                'endtime':task['endtime'],
                'status': ['claimed', 'allocated', 'conflict'][i%3],
                'claim_size':1} for r in resources[:1]]

        for c in claims[:]:
            c['properties'] = [{'type':0, 'value':10}, {'type':1, 'value':20}, {'type':2, 'value':30}]

        for i, c in enumerate(claims[:4]):
            c['properties'][0]['sap_nr'] = i % 2

        db.insertResourceClaims(task['id'], claims, 1, 'paulus', 1, False)

        #resultPrint(db.getResourceClaims)
        #raw_input()
        db.commit()
        now = datetime.utcnow()
        print totalSeconds(now - begin), totalSeconds(now - stepbegin)

    import pprint
    pprint.pprint(db.getResourceUsages(resource_type='storage'))

    #resultPrint(db.getResourceClaims)
    #resultPrint(db.getResourceClaimPropertyTypes)
    ##resultPrint(db.getResourceClaimPropertyTypeNames)
    ##resultPrint(db.getResourceClaimProperties)

    #print '\n'.join(str(x) for x in db.getResourceClaimProperties())
    #print '\n'.join(str(x) for x in db.getResourceClaimProperties(task_id=task['id']))
    #print '\n'.join(str(x) for x in db.getResourceClaims(include_properties=True))

    #db.commit()

    #resultPrint(db.getTasks)
    #resultPrint(db.getResourceClaims)

    #tasks = db.getTasks()
    #db.updateTaskAndResourceClaims(tasks[0]['id'], endtime=tasks[1]['endtime'])

    #resultPrint(db.getTasks)
    #resultPrint(db.getResourceClaims)
    #db.updateTaskAndResourceClaims(tasks[0]['id'], endtime=tasks[0]['starttime'] + timedelta(hours=1))

    #resultPrint(db.getTasks)
    #resultPrint(db.getResourceClaims)


    #claims = db.getResourceClaims()
    #for c in claims:
        #db.deleteResourceClaim(c['id'])
        ##resultPrint(db.getResourceClaims)

    #predTaskId = None
    #for i in range(2):
        #specId = db.insertSpecification(datetime.utcnow(), datetime.utcnow() + timedelta(hours=4), "")
        #taskId = db.insertTask(1234+i, 5678+i, 600, 0, specId)

        #if predTaskId:
            #db.insertTaskPredecessor(taskId, predTaskId)
        #predTaskId = taskId

        #resources = db.getResources()
        #for r in resources:
            #rcId = db.insertResourceClaim(r['id'], taskId, datetime.utcnow() + timedelta(hours=2*i), datetime.utcnow() + timedelta(hours=2*(i+1)), 0, 1, 10, 'einstein', -1)


    ##tasks = db.getTasks()
    ##for t in tasks:
        ##db.deleteTask(t['id'])
        ####resultPrint(db.getTasks)
        ####resultPrint(db.getResourceClaims)

    #import random

    ##for i in range(1):
        ##taskId = db.insertTask(1234, 5678, 600, 0, 1)
        ##for j in range(2*i):
            ##rcId = db.insertResourceClaim(j, taskId, datetime.utcnow() + timedelta(hours=4*i), datetime.utcnow() + timedelta(hours=4*i+3.5), 0, 4, 10, 'einstein', -1)

        ##time.sleep(0.5)

    ##resultPrint(db.getTasks)
    ##resultPrint(db.getResourceClaims)

    #ts = db.getTaskStatuses()

    #tasks = sorted(db.getTasks(), key=lambda x: x['id'])
    #for t in tasks:
        #db.updateTask(t['id'], task_status=ts[random.randint(0, len(ts)-1)]['id'])
        #time.sleep(0.01)

    #db.commit()
