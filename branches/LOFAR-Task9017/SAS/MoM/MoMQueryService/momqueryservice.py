#!/usr/bin/python
# $Id$

'''
Simple Service listening on momqueryservice.GetProjectDetails
which gives the project details for each requested mom object id

Example usage:
service side: just run this service somewhere where it can access the momdb and
a qpid broker.
Make sure the bus exists: qpid-config add exchange topic <busname>

client side: do a RPC call to the <busname>.GetProjectDetails with a
comma seperated string of mom2object id's as argument.
You get a dict of mom2id to project-details-dict back.

with RPC(busname, 'GetProjectDetails') as getProjectDetails:
    res, status = getProjectDetails(ids_string)

'''
from os import stat
import sys
import logging
from mysql import connector
from lofar.messaging import Service
from lofar.messaging.Service import MessageHandlerInterface
from lofar.common.util import waitForInterrupt

logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger=logging.getLogger("momqueryservice")

class MoMDatabaseWrapper:
    '''handler class for details query in mom db'''
    def __init__(self, passwd):
        self.conn = connector.connect(host="mysql1.control.lofar",
                                        user="momreadonly",
                                        passwd=passwd,
                                        database="lofar_mom3")

    def getProjectDetails(self, mom_ids_str):
        ''' get the project details (project_mom2id, project_name,
        project_description, object_mom2id, object_name, object_description,
        object_type, object_group_id) for given mom object mom_ids
        :param string mom_ids_str comma seperated string of mom2object id's
        :rtype list of dict's key value pairs with the project details
        '''
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

        return cursor.fetchall()


class ProjectDetailsQueryHandler(MessageHandlerInterface):
    '''handler class for details query in mom db
    :param MoMDatabaseWrapper momdb inject database access via wrapper
    '''
    def __init__(self, **kwargs):
        MessageHandlerInterface.__init__(self, **kwargs)
        self.momreadonly_passwd = kwargs.pop("momreadonly_passwd", '')
        self.kwargs = kwargs

    def prepare_loop(self):
        self.momdb = MoMDatabaseWrapper(self.momreadonly_passwd)

    def handle_message(self, text):
        '''The actual handler function.
        Parses the message text, converts it to csv id string,
        looks up the project(s) details via the momdb wrapper
        and returns the result
        :param string text The message's text content
        :rtype dict of momid -> details dict
        '''

        # parse text: it should contain a list of ints
        # filter out everything else to prevent sql injection
        mom_ids = [x.strip() for x in text.split(',')]
        mom_ids = [x for x in mom_ids if x.isdigit()]
        mom_ids_str = ', '.join(mom_ids)

        if not mom_ids_str:
            raise KeyError("Could not find proper ids in: " + text)

        logger.info("Query for mom id%s: %s" %
                    ('\'s' if len(mom_ids) > 1 else '', mom_ids_str))

        result = {}
        rows = self.momdb.getProjectDetails(mom_ids_str)
        for row in rows:
            object_mom2id = row['object_mom2id']
            result[str(object_mom2id)] = row
            logger.info("Result for %s: %s" % (object_mom2id, str(row)))

        return result


def createService(busname='momqueryservice',
                  momreadonly_passwd='',
                  handler=None):
    '''create the GetProjectDetails on given busname
    :param string busname: name of the bus on which this service listens
    :param string momreadonly_passwd: the momreadonly passwd.
    :param ProjectDetailsQueryHandler handler: ProjectDetailsQueryHandler class Type, or mock like type
    :rtype: lofar.messaging.Service'''

    if not handler:
        handler = ProjectDetailsQueryHandler

    return Service('GetProjectDetails',
                   handler,
                   busname=busname,
                   numthreads=1,
                   handler_args={'momreadonly_passwd':momreadonly_passwd})


def main():
    '''Starts the momqueryservice.GetProjectDetails service'''

    # make sure config.py is mode 600 to hide passwords
    if oct(stat('config.py').st_mode & 0777) != '0600':
        print 'Please change permissions of config.py to 600'
        exit(-1)

    # safely import momreadonly_passwd
    from lofar.mom.momqueryservice.config import momreadonly_passwd

    # start the service and listen.
    with createService('momqueryservice', momreadonly_passwd):
        waitForInterrupt()

if __name__ == '__main__':
    main()
