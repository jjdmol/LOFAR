#                                                       LOFAR PIPELINE FRAMEWORK
#
#                                                          Stateful LOFAR Recipe
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

from functools import wraps

import os.path
import cPickle

from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.lofarexceptions import PipelineException
from lofarpipe.support.xmllogging import _enter_active_stack_node, \
    _exit_active_stack_node, add_child
from xml.dom.minidom import parseString, Document

def stateful(run_task):
    @wraps(run_task)
    def wrapper(self, configblock, datafiles=[], **kwargs):
        try:

            my_state = self.completed.pop()
        except (AttributeError, IndexError):
            my_state = ('', '')

        if configblock == my_state[0]:
            # We have already run this task and stored its state, or...
            self.logger.info("Task %s already exists in saved state; skipping"
                             % configblock)
            return my_state[1]
        elif my_state[0] != '':
            # There is a stored task, but it doesn't match this one, or...
            self.logger.error("Stored state does not match pipeline"
                              " definition. bailing out")
            raise PipelineException("Stored state does not match pipeline"
                                    "definition")
        else:
            # We need to run this task now.

            # Create an xml node with all input information for 
            # the master node
            # first the full parset (TODO: maybee only partial parset?)
            local_document = Document()
            local_node = local_document.createElement('input_xml')
            local_node.appendChild(self.xml_parset)

            # Add a node with inputs
            argument_node = add_child(local_node, "recipe_arguments")
            for (key, value) in kwargs.items():
                argument_node.setAttribute(key, str(value))

            #raise Exception(local_node)
            # send xml to the master recipe: Use the inputs
            kwargs['input_xml'] = local_node.toxml(encoding='ascii')

            # Run the actual recipe
            outputs = run_task(self, configblock, datafiles, **kwargs)


            #save the outputs
            self.state.append((configblock, outputs))
            self._save_state()

            # Allow transfer of timing info from recipes to toplevel
            # location of new toplevel to master interface
            if "return_xml" in outputs:
                #raise Exception(outputs['return_xml'])
                recipe_node_timing = _enter_active_stack_node(
                    self, 'active_xml', "recipe")
                recipe_node_timing.setAttribute("task", configblock)

                recipe_node_timing.appendChild(
                    parseString(outputs['return_xml']).documentElement)#.documentElement)

                _exit_active_stack_node(self, 'active_xml')

            return outputs
    return wrapper

class StatefulRecipe(BaseRecipe):
    """
    Enables recipes to save and restore state.

    This is used exactly as :class:`~lofarpipe.support.baserecipe.BaseRecipe`,
    but will write a ``statefile`` in the job directory, recording the current
    state of the pipeline after each recipe completes. If the pipeline is
    interrupted, it can automatically resume where it left off.

    To reset the pipeline and start from the beginning again, just remove the
    ``statefile``.
    """
    inputs = {} # No non-default inputs
    def __init__(self):
        super(StatefulRecipe, self).__init__()
        self.state = []
        self.completed = []

    def _save_state(self):
        """
        Dump pipeline state to file.
        """
        statefile = open(
            os.path.join(
                self.config.get('layout', 'job_directory'),
                'statefile'
            ),
        'w')
        state = [self.inputs, self.state]
        cPickle.dump(state, statefile)

    def go(self):
        super(StatefulRecipe, self).go()
        statefile = os.path.join(
            self.config.get('layout', 'job_directory'),
            'statefile'
        )
        try:
            statefile = open(statefile, 'r')
            inputs, self.state = cPickle.load(statefile)
            statefile.close()

            # What's the correct thing to do if inputs differ from the saved
            # state? start_time will always change.
            for key, value in inputs.iteritems():
                if key != "start_time" and self.inputs[key] != value:
                    raise PipelineException(
                        "Input %s (%s) differs from saved state (%s)" %
                        (key, str(self.inputs[key]), inputs[key])
                    )

            self.completed = list(reversed(self.state))
        except (IOError, EOFError):
            # Couldn't load state
            self.completed = []

    run_task = stateful(BaseRecipe.run_task)
