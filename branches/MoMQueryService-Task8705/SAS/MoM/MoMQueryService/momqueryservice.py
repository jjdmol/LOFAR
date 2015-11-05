#!/usr/bin/python
# $Id$

'''
Simple Service listening on momqueryservice.GetProjectDetails
which gives the project details for each requested mom object id
'''

from mysql import connector
from lofar.messaging import Service

# do not commit passwd in svn
passwd=''

class DBlistener:
    def __init__(self):
        self.conn = connector.connect(host="mysql1.control.lofar",
                                        user="momreadonly",
                                        passwd=passwd,
                                        database="lofar_mom3")

    def __call__(self, text):
        # parse text
        # it should contain a list of ints
        # filter out everything else to prevent sql injection
        mom_ids = [x.strip() for x in text.split(',')]
        mom_ids = [x for x in mom_ids if x.isdigit()]

        cursor = self.conn.cursor()
        # TODO: make a view for this query in momdb!
        query = '''SELECT project.mom2id as project_mom2id, project.name as project_name, project.description as project_description,
        object.mom2id as object_mom2id, object.name as object_name, object.description as object_description, object.mom2objecttype as object_type, object.group_id as object_group_id
        FROM lofar_mom3.mom2object as object
        inner join lofar_mom3.mom2object as project on project.id = object.ownerprojectid
        where object.mom2id in (%s)
        order by project_mom2id
        ''' % (','.join(mom_ids))
        cursor.execute(query)

        result={}

        rows= cursor.fetchall()
        for row in rows:
            item = {'project_mom2id': int(row[0]),
                    'project_name': row[1],
                    'project_description': row[2],
                    'object_mom2id': int(row[3]),
                    'object_name': row[4],
                    'object_description': row[5],
                    'object_type': row[6],
                    'object_group_id': int(row[7]) if row[7] else None}
            result[str(row[3])] = item

        return result

with Service("momqueryservice", "GetProjectDetails", DBlistener(), startonwith=True) as getProjectDetailsService:
    getProjectDetailsService.WaitForInterrupt()
