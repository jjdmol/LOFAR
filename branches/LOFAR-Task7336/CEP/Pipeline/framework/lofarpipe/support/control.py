#                                                       LOFAR PIPELINE FRAMEWORK
#
#                                                        Pipeline control recipe
#                                                         John Swinbank, 2009-10
#                                                      swinbank@transientskp.org
#                                                             Marcel Loose, 2012
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------
import sys
import re
import traceback

from lofarpipe.support.stateful import StatefulRecipe
from lofarpipe.support.lofarexceptions import PipelineException
from lofarpipe.support.xmllogging import get_active_stack
from lofar.parameterset import parameterset
from lofar.messagebus import ToBus
from lofar.messagebus.protocols.taskfeedbackdataproducts import TaskFeedbackDataproducts
from lofar.messagebus.protocols.taskfeedbackprocessing import TaskFeedbackProcessing
from lofar.messagebus.protocols.taskfeedbackstatus import TaskFeedbackStatus

#                                             Standalone Pipeline Control System
# ------------------------------------------------------------------------------

class control(StatefulRecipe):
    """
    Basic pipeline control framework.

    Define a pipeline by subclassing and providing a body for the
    :meth:`pipeline_logic`.

    This class provides little, but can be specialised to eg provide a
    MAC/SAS interface etc.
    """
    inputs = {}

    def send_feedback_processing(self, feedback):
        """
        Send processing feedback information back to LOFAR.

        `feedback` must be a parameterset
        """

        bus = ToBus("lofar.task.feedback.processing")
        msg = TaskFeedbackProcessing(
          "lofarpipe.support.control",
          "",
          "Processing feedback from the pipeline framework",
          momID,
          sasID,
          feedback)

        bus.sendmsg(msg.qpidMsg())

    def send_feedback_dataproducts(self, feedback):
        """
        Send dataproduct feedback information back to LOFAR.

        `feedback` must be a parameterset
        """

        bus = ToBus("lofar.task.feedback.dataproduct")
        msg = TaskFeedbackDataproducts(
          "lofarpipe.support.control",
          "",
          "Dataproduct feedback from the pipeline framework",
          momID,
          sasID,
          feedback)

        bus.sendmsg(msg.qpidMsg())

    def _send_feedback_status(self, status):
        """
        Send status information back to LOFAR.

        `status` must be an integer; 0 indicates success, any other value
        indicates failure.
        """

        bus = ToBus("lofar.task.feedback.status")
        msg = TaskFeedbackStatus(
          "lofarpipe.support.control",
          "",
          "Status feedback from the pipeline framework",
          momID,
          sasID,
          status == 0)

        bus.sendmsg(msg.qpidMsg())

    def pipeline_logic(self):
        """
        Define pipeline logic here in subclasses
        """
        raise NotImplementedError

    def go(self):
        super(control, self).go()
        self.logger.info("LOFAR Pipeline (%s) starting." % self.name)
        try:
            self.pipeline_logic()
        except Exception, message:
            self.logger.error("*******************************************")
            self.logger.error("Failed pipeline run: {0}".format(
                        self.inputs['job_name']))

            # Get detailed information of the caught exception
            (type, value, traceback_object) = sys.exc_info()
            self.logger.error("Detailed exception information:")
            self.logger.error(str(type))
            self.logger.error(str(value))
            # Get the stacktrace and pretty print it:
            # self.logger.error("\n" + " ".join(traceback.format_list(
            #            traceback.extract_tb(traceback_object))))

            self.logger.error("*******************************************")

            # Emit process status
            self._send_feedback_status(1)
            return 1
        else:
            # Emit process status
            self._send_feedback_status(0)
            return 0
        finally:
            # always print a xml stats file
            if get_active_stack(self) != None:
                xmlfile = self.config.get("logging", "xml_stat_file")
                try:
                    fp = open(xmlfile, "w")
                    fp.write(get_active_stack(self).toxml(encoding='ascii'))
                    fp.close()
                except Exception, except_object:
                    self.logger.error("Failed opening xml stat file:")
                    self.logger.error(except_object)

        return 0
