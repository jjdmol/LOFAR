#!/usr/bin/python
# $Id$

'''
TODO: add doc
'''
import logging
from optparse import OptionParser
from lofar.sas.otdb.OTDBBusListener import OTDBBusListener
from lofar.common.util import waitForInterrupt
from lofar.sas.otdb.config import DEFAULT_OTDB_NOTIFICATION_BUSNAME, DEFAULT_OTDB_NOTIFICATION_SUBJECT
from lofar.sas.resourceassignment.resourceassignmentservice.config import DEFAULT_BUSNAME as DEFAULT_RADB_BUSNAME
from lofar.sas.resourceassignment.resourceassignmentservice.config import DEFAULT_SERVICENAME as DEFAULT_RADB_SERVICENAME
from lofar.sas.resourceassignment.resourceassignmentservice.rpc import RARPC

logger = logging.getLogger(__name__)

class OTDBtoRATaskStatusPropagator(OTDBBusListener):
    def __init__(self,
                 otdb_notification_busname=DEFAULT_OTDB_NOTIFICATION_BUSNAME,
                 otdb_notification_subject=DEFAULT_OTDB_NOTIFICATION_SUBJECT,
                 radb_busname=DEFAULT_RADB_BUSNAME,
                 radb_servicename=DEFAULT_RADB_SERVICENAME,
                 broker=None, **kwargs):
        super(OTDBtoRATaskStatusPropagator, self).__init__(busname=otdb_notification_busname,
                                                           subject=otdb_notification_subject,
                                                           broker=broker,
                                                           **kwargs)

        self.radb = RARPC(busname=radb_busname, servicename=radb_servicename, broker=broker)

    def start_listening(self, **kwargs):
        self.radb.open()
        super(OTDBtoRATaskStatusPropagator, self).start_listening(**kwargs)

    def stop_listening(self, **kwargs):
        self.radb.close()
        super(OTDBtoRATaskStatusPropagator, self).stop_listening(**kwargs)

    def _update_radb_task_status(self, otdb_id, task_status):
        logger.info("updating task with otdb_id %s to status %s" % (otdb_id, task_status))
        result = self.radb.updateTaskStatusForOtdbId(otdb_id=otdb_id, status=task_status)

        if not result or 'updated' not in result or not result['updated']:
            logger.warning("could not update task with otdb_id %s to status %s" % (otdb_id, task_status))

    def onObservationPrepared(self, treeId, modificationTime):
        self._update_radb_task_status(treeId, 'prepared')

    def onObservationApproved(self, treeId, modificationTime):
        self._update_radb_task_status(treeId, 'approved')

    def onObservationOnHold(self, treeId, modificationTime):
        self._update_radb_task_status(treeId, 'on_hold')

    def onObservationConflict(self, treeId, modificationTime):
        self._update_radb_task_status(treeId, 'conflict')

    def onObservationPrescheduled(self, treeId, modificationTime):
        logger.info("not propagating prescheduled status for otdb_id %s to radb because the resource assigner takes care of this" % (treeId))

    def onObservationScheduled(self, treeId, modificationTime):
        self._update_radb_task_status(treeId, 'scheduled')

    def onObservationQueued(self, treeId, modificationTime):
        self._update_radb_task_status(treeId, 'queued')

    def onObservationStarted(self, treeId, modificationTime):
        self._update_radb_task_status(treeId, 'active')

    def onObservationCompleting(self, treeId, modificationTime):
        self._update_radb_task_status(treeId, 'completing')

    def onObservationFinished(self, treeId, modificationTime):
        self._update_radb_task_status(treeId, 'finished')

    def onObservationAborted(self, treeId, modificationTime):
        self._update_radb_task_status(treeId, 'aborted')

def main():
    # Check the invocation arguments
    parser = OptionParser("%prog [options]",
                                                description='runs the resourceassignment database service')
    parser.add_option('-q', '--broker', dest='broker', type='string', default=None, help='Address of the qpid broker, default: localhost')
    parser.add_option("--otdb_notification_busname", dest="otdb_notification_busname", type="string",
                      default=DEFAULT_OTDB_NOTIFICATION_BUSNAME,
                      help="Bus or queue where the OTDB notifications are published. [default: %default]")
    parser.add_option("--otdb_notification_subject", dest="otdb_notification_subject", type="string",
                      default=DEFAULT_OTDB_NOTIFICATION_SUBJECT,
                      help="Subject of OTDB notifications on otdb_notification_busname. [default: %default]")
    parser.add_option("--radb_busname", dest="radb_busname", type="string",
                      default=DEFAULT_RADB_BUSNAME,
                      help="Name of the radb bus exchange on the qpid broker. [default: %default]")
    parser.add_option("--radb_servicename", dest="radb_servicename", type="string",
                      default=DEFAULT_RADB_SERVICENAME,
                      help="Name of the radb service. [default: %default]")
    (options, args) = parser.parse_args()

    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)

    with OTDBtoRATaskStatusPropagator(otdb_notification_busname=options.otdb_notification_busname,
                                      otdb_notification_subject=options.otdb_notification_subject,
                                      radb_busname=options.radb_busname,
                                      radb_servicename=options.radb_servicename,
                                      broker=options.broker):
        waitForInterrupt()

if __name__ == '__main__':
    main()
