#!/usr/bin/python
# $Id$

'''
Simple Service listening on momqueryservice.GetProjectDetails
which gives the project details for each requested mom object id
'''
import sys
import logging
from mysql import connector
from lofar.messaging import Service

logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger=logging.getLogger("momqueryservice")

class ProjectDetailsQueryHandler:
    '''handler class for details query in mom db'''
    def __init__(self, passwd):
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
        mom_ids_str = ', '.join(mom_ids)

        if not mom_ids_str:
            logger.error("Could not find proper ids in: " + text)
            raise KeyError("Could not find proper ids in: " + text)

        logger.info("Query for mom id%s: %s" % ('\'s' if len(mom_ids) > 1 else '', mom_ids_str))

        cursor = self.conn.cursor(dictionary=True)
        # TODO: make a view for this query in momdb!
        query = '''SELECT project.mom2id as project_mom2id, project.name as project_name, project.description as project_description,
        object.mom2id as object_mom2id, object.name as object_name, object.description as object_description, object.mom2objecttype as object_type, object.group_id as object_group_id
        FROM lofar_mom3.mom2object as object
        inner join lofar_mom3.mom2object as project on project.id = object.ownerprojectid
        where object.mom2id in (%s)
        order by project_mom2id
        ''' % (mom_ids_str)
        cursor.execute(query)

        result = {}
        rows = cursor.fetchall()
        for row in rows:
            object_mom2id = row['object_mom2id']
            result[str(object_mom2id)] = row
            logger.info("Result for %s: %s" % (object_mom2id, str(row)))

        return result


def createService(busname='momqueryservice', momreadonly_passwd=''):
    '''create the GetProjectDetails on given busname
    :param string busname: name of the bus on which this service listens
    :param string momreadonly_passwd: the momreadonly passwd.
    :rtype: lofar.messaging.Service'''
    handler = ProjectDetailsQueryHandler(momreadonly_passwd)
    return Service(busname,
                   'GetProjectDetails',
                   handler,
                   startonwith=True,
                   numthreads=1)


def main():
    # do not commit passwd in svn
    passwd = ''

    with createService('momqueryservice', passwd) as getProjectDetailsService:
        getProjectDetailsService.WaitForInterrupt()

if __name__ == '__main__':
    main()
