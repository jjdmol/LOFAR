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
import lofar.messagebus.messagebus as messagebus
from lofar.messagebus.protocols import TaskFeedbackDataproducts, TaskFeedbackProcessing, TaskFeedbackState

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

        if self.feedback_method == "messagebus":
          bus = messagebus.ToBus("lofar.task.feedback.processing")
          msg = TaskFeedbackProcessing(
            "lofarpipe.support.control",
            "",
            "Processing feedback from the pipeline framework",
            self.momID,
            self.sasID,
            feedback)

          bus.send(msg)

    def send_feedback_dataproducts(self, feedback):
        """
        Send dataproduct feedback information back to LOFAR.

        `feedback` must be a parameterset
        """

        if self.feedback_method == "messagebus":
          bus = messagebus.ToBus("lofar.task.feedback.dataproducts")
          msg = TaskFeedbackDataproducts(
            "lofarpipe.support.control",
            "",
            "Dataproduct feedback from the pipeline framework",
            self.momID,
            self.sasID,
            feedback)

          bus.send(msg)

    def _send_feedback_status(self, status):
        """
        Send status information back to LOFAR.

        `status` must be an integer; 0 indicates success, any other value
        indicates failure.
        """

        if self.feedback_method == "messagebus":
          bus = messagebus.ToBus("lofar.task.feedback.state")
          msg = TaskFeedbackState(
            "lofarpipe.support.control",
            "",
            "Status feedback from the pipeline framework",
            self.momID,
            self.sasID,
            status == 0)

          bus.send(msg)

    def pipeline_logic(self):
        """
        Define pipeline logic here in subclasses
        """
        raise NotImplementedError

    def go(self):
        # Read the parset-file that was given as input argument
        try:
            self.parset_file = os.path.abspath(self.inputs['args'][0])
        except IndexError:
            return self.usage()
        self.parset.adoptFile(self.parset_file)
        # Set job-name to basename of parset-file w/o extension, if it's not
        # set on the command-line with '-j' or '--job-name'
        if not 'job_name' in self.inputs:
            self.inputs['job_name'] = (
                os.path.splitext(os.path.basename(self.parset_file))[0]
            )

        # we can call our parent now that we have a job_name
        super(control, self).go()

        # we now have a self.config -- read our settings
        try:
          self.feedback_method = self.config.get('feedback', 'method')
        except:
          self.feedback_method = "messagebus"

        if self.feedback_method == "messagebus" and not messagebus.MESSAGING_ENABLED:
          self.logger.error("Feedback over messagebus requested, but messagebus support is not enabled or functional")
          return 1

        # Pull several parameters from the parset
        self.momID = self.parset.getString("ObsSW.Observation.momID", "")  # Note: 0 if obs was copied in Scheduler
        self.sasID = self.parset.getString("ObsSW.Observation.otdbID", "") # SAS ID

        # Start the pipeline
        self.logger.info("LOFAR Pipeline (%s) starting." % self.name)
        self.logger.info("SASID = %s, MOMID = %s, Feedback method = %s" % (self.sasID, self.momID, self.feedback_method))

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
            self.logger.error("LOFAR Pipeline finished unsuccesfully.");
            return 1
        else:
            # Emit process status
            self._send_feedback_status(0)
            self.logger.info("LOFAR Pipeline finished succesfully.");
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
