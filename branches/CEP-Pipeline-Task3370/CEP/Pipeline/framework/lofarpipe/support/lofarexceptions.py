#                                                       LOFAR PIPELINE FRAMEWORK
#
#                                                                     Exceptions
#                                                         John Swinbank, 2009-10
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------
import inspect


class ExecutableMissing(Exception):
    pass

class PipelineException(Exception):
    def __init__(self, *args, **argsw):
        """
        Expand the exception with knowledge of the current local namespace
        This allows detailed reporting of local state back from node recipes
        """
        super(PipelineException, self).__init__(*args, **argsw)
        frame = inspect.currentframe()
        self._locals = frame.f_back.f_locals

    pass

class PipelineRecipeFailed(PipelineException):
    pass

class PipelineReceipeNotFound(PipelineException):
    pass

class PipelineQuit(PipelineException):
    """
    If this exception is raised during a pipeline run, we skip over all
    subsequent steps and exit cleanly.
    """
    pass

class ClusterError(PipelineException):
    pass
