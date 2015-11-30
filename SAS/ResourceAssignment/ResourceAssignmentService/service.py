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
from lofar.messaging import Service
from lofar.messaging.Service import MessageHandlerInterface
from lofar.common.util import waitForInterrupt
from lofar.sas.resourceassignment.resourceassignmentservice import radb

logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger=logging.getLogger(__name__)

class _RADBHandler(MessageHandlerInterface):
    def __init__(self, **kwargs):
        super(_RADBHandler, self).__init__(**kwargs)
        self.username = kwargs.pop("username", 'resourceassignment')
        self.password = kwargs.pop("password", '')

    def prepare_loop(self):
        self.radb = radb.RADatabase(username=self.username,
                                    password=self.password)

class _GetTaskStatusesHandler(_RADBHandler):
    def handle_message(self, text):
        return self.radb.getTaskStatuses()



def main():
    getTaskStatusesService = Service('GetTaskStatuses',
                   _GetTaskStatusesHandler,
                   busname='raservice',
                   handler_args={'password':'123456'})

    with getTaskStatusesService:
        waitForInterrupt()

if __name__ == '__main__':
    main()
