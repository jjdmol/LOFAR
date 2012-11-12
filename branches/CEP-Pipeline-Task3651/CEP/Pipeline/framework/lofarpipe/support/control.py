#                                                       LOFAR PIPELINE FRAMEWORK
#
#                                                        Pipeline control recipe
#                                                         John Swinbank, 2009-10
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

from lofarpipe.support.stateful import StatefulRecipe
from lofarpipe.support.lofarexceptions import PipelineException
from lofarpipe.support.xmllogging import get_active_stack

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
    def pipeline_logic(self):
        """
        Define pipeline logic here in subclasses
        """
        raise NotImplementedError

    def go(self):
        super(control, self).go()

        self.logger.info(
            "LOFAR Pipeline (%s) starting." %
            (self.name,)
        )

        try:
            self.pipeline_logic()
        except Exception, message:
            self.logger.error("*******************************************")
            self.logger.error("Failed pipeline run: {0}".format(
                        self.inputs['job_name']))
            #message does not contain the original exception thrown in recipe
            if get_active_stack(self) != None:
                self.logger.error("\n" +
                    get_active_stack(self).toprettyxml(encoding='ascii'))
            self.logger.error("*******************************************")

            return 1

        return 0



