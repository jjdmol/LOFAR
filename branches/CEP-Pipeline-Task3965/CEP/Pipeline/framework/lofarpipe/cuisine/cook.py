#from message import ErrorLevel, NotifyLevel, VerboseLevel, DebugLevel
import imp

from lofarpipe.support.pipelinelogging import getSearchingLogger


class CookError(Exception):
    """
    Base class for all exceptions raised by this module.
    """
    pass


class WSRTCook(object):
    """
    Baseclass for initiating cook parameters:
    the inputs,outputs, the task and the logger
    """
    def __init__(self, task, inputs, outputs, logger):
        self.inputs = inputs
        self.outputs = outputs
        self.task = task.strip()
        self.logger = logger


class PipelineCook(WSRTCook):
    """
    A system for spawning a recipe, providing it with correct inputs, and
    collecting its outputs.
    """
    def __init__(self, task, inputs, outputs, logger, recipe_path):
        super(PipelineCook, self).__init__(task, inputs, outputs, logger)
        # Ensures the recipe to be run can be imported from the recipe path
        try:
            # Try finding the requested task as a module: raise import error
            # if not found
            try:
                module_details = imp.find_module(task, recipe_path)
            except ImportError:
                # ...also support lower-cased file names.
                module_details = imp.find_module(task.lower(), recipe_path)

            # load the module
            module = imp.load_module(task, *module_details)
            self.recipe = None
            try:
                self.recipe = getattr(module, task)()
            except AttributeError:
                # Try with first letter capital (python type nameconvention)
                self.recipe = getattr(module, task.capitalize())()
            # Add a logger
            self.recipe.logger = getSearchingLogger("%s.%s" % (self.logger.name, task))
            self.recipe.logger.setLevel(self.logger.level)
        except Exception, e:
            self.logger.exception("Exception caught: " + str(e))
            self.logger.error("Could not open the task: {0}".format(task))
            self.logger.error("from path: {0}".format(recipe_path))
            raise CookError (task + ' can not be loaded')

    def try_running(self):
        """
        Run the recipe, inputs should already have been checked.
        """
        self.recipe.name = self.task
        if not self.recipe.run(self.task):
            self.copy_outputs()
        else:
            raise CookError (self.task + ' failed')

    def copy_inputs(self):
        """
        Ensure inputs are available to the recipe to be run
        """
        for k in self.inputs.keys():
            self.recipe.inputs[k] = self.inputs[k]

    def copy_outputs(self):
        """
        Pass outputs from the recipe back to the rest of the pipeline
        """
        if self.recipe.outputs == None:
            raise CookError (self.task + ' has no outputs') ## should it have??
        else:
            for k in self.recipe.outputs.keys():
                self.outputs[k] = self.recipe.outputs[k]

    def spawn(self):
        """
        Copy inputs to the target recipe then run it
        """
        self.copy_inputs()
        self.try_running()
