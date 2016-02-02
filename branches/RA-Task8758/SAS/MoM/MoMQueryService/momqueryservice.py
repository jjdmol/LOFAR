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
from optparse import OptionParser
from mysql import connector
from lofar.messaging import Service
from lofar.messaging.Service import MessageHandlerInterface
from lofar.common.util import waitForInterrupt
from lofar.mom.momqueryservice.config import DEFAULT_BUSNAME, DEFAULT_SERVICENAME

logger=logging.getLogger(__file__)

class MoMDatabaseWrapper:
    '''handler class for details query in mom db'''
    def __init__(self, passwd):
        self.conn = connector.connect(host="mysql1.control.lofar",
                                        user="momreadonly",
                                        passwd=passwd,
                                        database="lofar_mom3")

    def getProjectDetails(self, mom_ids):
        ''' get the project details (project_mom2id, project_name,
        project_description, object_mom2id, object_name, object_description,
        object_type, object_group_id) for given mom object mom_ids
        :param mixed mom_ids comma seperated string of mom2object id's, or list of ints
        :rtype list of dict's key value pairs with the project details
        '''

        if isinstance(mom_ids, basestring):
            # parse text: it should contain a list of ints
            # filter out everything else to prevent sql injection
            mom_ids_orig = mom_ids
            mom_ids = [x.strip() for x in mom_ids.split(',')]
            mom_ids = [x for x in mom_ids if x.isdigit()]
            mom_ids = ', '.join(mom_ids)

            if not mom_ids:
                raise KeyError("Could not find proper ids in: " + mom_ids_orig)

        logger.info("Query for mom id%s: %s" %
                    ('\'s' if len(mom_ids) > 1 else '', mom_ids))

        cursor = self.conn.cursor(dictionary=True)
        # TODO: make a view for this query in momdb!
        query = '''SELECT project.mom2id as project_mom2id, project.name as project_name, project.description as project_description,
        object.mom2id as object_mom2id, object.name as object_name, object.description as object_description, object.mom2objecttype as object_type, object.group_id as object_group_id
        FROM lofar_mom3.mom2object as object
        inner join lofar_mom3.mom2object as project on project.id = object.ownerprojectid
        where object.mom2id in (%s)
        order by project_mom2id
        ''' % (mom_ids)
        cursor.execute(query)

        rows = cursor.fetchall()

        logger.info("Found %d results for mom id%s: %s" %
                    (len(rows), '\'s' if len(mom_ids) > 1 else '', mom_ids))

        result = {}
        for row in rows:
            object_mom2id = row['object_mom2id']
            result[str(object_mom2id)] = dict(row)

        return result

    def getProjects(self):
        ''' get the list of all projects with columns (project_mom2id, project_name,
        project_description, status_name, status_id, last_user_id,
        last_user_name, statustime)
        :rtype list of dict's key value pairs with all projects
        '''
        cursor = self.conn.cursor(dictionary=True)
        # TODO: make a view for this query in momdb!
        query = '''SELECT project.mom2id as mom2id, project.name as name, project.description as description,
                lofar_mom3.statustype.code as status_name,  lofar_mom3.statustype.id as status_id,
        lofar_mom3.status.userid as last_user_id, lofar_mom3.status.name as last_user_name, lofar_mom3.status.statustime as statustime
        FROM lofar_mom3.mom2object as project
        left join lofar_mom3.mom2objectstatus as status on project.currentstatusid = status.id
        left join lofar_mom3.status as statustype on status.statusid=statustype.id
        where project.mom2objecttype='PROJECT'
        order by project_mom2id;
        '''
        cursor.execute(query)

        return cursor.fetchall()


class ProjectDetailsQueryHandler(MessageHandlerInterface):
    '''handler class for details query in mom db
    :param MoMDatabaseWrapper momdb inject database access via wrapper
    '''
    def __init__(self, **kwargs):
        MessageHandlerInterface.__init__(self, **kwargs)
        self.momreadonly_passwd = kwargs.pop("momreadonly_passwd", '')

        self.service2MethodMap = {
            'GetProjects': self.getProjects,
            'GetProjectDetails': self.getProjectDetails
            }

    def prepare_loop(self):
        self.momdb = MoMDatabaseWrapper(self.momreadonly_passwd)

    def getProjectDetails(self, mom_ids):
        return self.momdb.getProjectDetails(mom_ids)

    def getProjects(self):
        return self.momdb.getProjects()

def createService(busname=DEFAULT_BUSNAME,
                  servicename=DEFAULT_SERVICENAME,
                  momreadonly_passwd='',
                  handler=None):
    '''create the GetProjectDetails on given busname
    :param string busname: name of the bus on which this service listens
    :param string servicename: name of the service
    :param string momreadonly_passwd: the momreadonly passwd.
    :param ProjectDetailsQueryHandler handler: ProjectDetailsQueryHandler class Type, or mock like type
    :rtype: lofar.messaging.Service'''

    if not handler:
        handler = ProjectDetailsQueryHandler

    return Service(servicename,
                   handler,
                   busname=busname,
                   numthreads=1,
                   use_service_methods=True,
                   verbose=False,
                   handler_args={'momreadonly_passwd':momreadonly_passwd})


def main(busname=DEFAULT_BUSNAME,
         servicename=DEFAULT_SERVICENAME,
         momreadonly_passwd=None):
    '''Starts the momqueryservice.GetProjectDetails service'''

    if not momreadonly_passwd:
        from lofar.mom.momqueryservice.config import momreadonly_passwd

    # Check the invocation arguments
    parser = OptionParser("%prog [options]",
                          description='runs the momqueryservice')
    parser.add_option("-b", "--busname", dest="busname", type="string", default=busname, help="Name of the bus exchange on the qpid broker, default: %s" % busname)
    parser.add_option("-s", "--servicename", dest="servicename", type="string", default=servicename, help="Name for this service, default: %s" % servicename)
    (options, args) = parser.parse_args()

    # start the service and listen.
    with createService(busname=options.busname, servicename=options.servicename, momreadonly_passwd=momreadonly_passwd):
        waitForInterrupt()

if __name__ == '__main__':
    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
    main()
