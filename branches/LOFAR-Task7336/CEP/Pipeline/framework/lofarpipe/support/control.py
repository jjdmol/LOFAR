#                                                       LOFAR PIPELINE FRAMEWORK
#
#                                                        Pipeline control recipe
#                                                         John Swinbank, 2009-10
#                                                      swinbank@transientskp.org
#                                                             Marcel Loose, 2012
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------
import os
import sys
import re
import traceback

from lofarpipe.support.stateful import StatefulRecipe
from lofarpipe.support.lofarexceptions import PipelineException
from lofarpipe.support.xmllogging import get_active_stack
from lofar.parameterset import parameterset
from lofar.messagebus.msgbus import ToBus
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

    def __init__(self):
      super(control, self).__init__()
      
      self.parset = parameterset()
      self.momID = 0
      self.sasID = 0

    def usage(self):
        """
        Display usage information
        """
        print >> sys.stderr, "Usage: %s <parset-file>  [options]" % sys.argv[0]
        return 1

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
          self.momID,
          self.sasID,
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
          self.momID,
          self.sasID,
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
          self.momID,
          self.sasID,
          status == 0)

        bus.sendmsg(msg.qpidMsg())

    def pipeline_logic(self):
        """
        Define pipeline logic here in subclasses
        """
        raise NotImplementedError

    def go(self):
        # Read the parset-file that was given as input argument
        try:
            parset_file = os.path.abspath(self.inputs['args'][0])
        except IndexError:
            return self.usage()
        self.parset.adoptFile(parset_file)
        # Set job-name to basename of parset-file w/o extension, if it's not
        # set on the command-line with '-j' or '--job-name'
        if not 'job_name' in self.inputs:
            self.inputs['job_name'] = (
                os.path.splitext(os.path.basename(parset_file))[0]
            )

        # we can call our parent now that we have a job_name
        super(control, self).go()

        # Pull several parameters from the parset
        self.momID = self.parset.getString("Observation.momID", "")
        self.sasID = self.parset.getString("Observation.ObsID", "")

        # Start the pipeline
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
