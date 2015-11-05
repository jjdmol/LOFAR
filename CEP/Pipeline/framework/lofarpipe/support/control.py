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
import lofarpipe.support.mac_feedback as mac_feedback

#                                             Standalone Pipeline Control System
# ------------------------------------------------------------------------------

class control(StatefulRecipe):
    """
    Basic pipeline control framework.

    Define a pipeline by subclassing and provding a body for the
    :meth:`pipeline_logic`.

    This class provides little, but can be specialised to eg provide a
    MAC/SAS interface etc.
    """
    inputs = {}

    def _send_mac_feedback(self, status):
        """
        Send status information back to MAC, but only if the Python controller
        host to send this information to was given as input argument.
        `status` must be an integer; 0 indicates success, any other value
        indicates failure.
        The port number is calculated as 22000 + observationNr%1000. 
        We need to extract this number from the job-name, which should be equal
        to "Observation" + str(observationNr).
        """
        try:
            host = self.inputs['args'][1]
        except IndexError:
            self.logger.warn(
                "No MAC Python controller host specified. "
                "Not sending status feedback to MAC"
            )
            return
        # Determine port number to use.
        match = re.findall(r'^Observation(\d+)$', self.inputs['job_name'])
        if match:
            port = 22000 + int(match[0]) % 1000
            self.logger.info(
                "Sending status feedback to MAC [%s:%s] (status: %s)" %
                (host, port, status)
            )
        else:
            self.logger.warn(
                r"Job-name does not match with pattern '^Observation(\d+)$'. "
                "Not sending status feedback to MAC"
            )
            return
        # Send feedback information
        try:
            mac_feedback.send_status(host, port, status)
        except IOError, error:
            self.logger.warn(
                "Failed to send status feedback to MAC [%s:%s]: %s" %
                (host, port, error)
            )

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
            self.logger.error(message)
            # Get detailed information of the caught exception
            (type, value, traceback_object) = sys.exc_info()
            self.logger.error("Detailed exception information:")
            self.logger.error(str(type))
            self.logger.error(str(value))
            self.logger.error(str(traceback.extract_tb(traceback_object)))
            #message does not contain the original exception thrown in recipe
            if get_active_stack(self) != None:
                self.logger.error("\n" +
                    get_active_stack(self).toprettyxml(encoding='ascii'))
            self.logger.error("*******************************************")

            self._send_mac_feedback(1)
            return 1
        else:
            self._send_mac_feedback(0)
            return 0

        return 0


