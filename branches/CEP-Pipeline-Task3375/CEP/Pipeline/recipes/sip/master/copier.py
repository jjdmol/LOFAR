#                                                        LOFAR IMAGING PIPELINE
#
#                                 Master recipe for copying files between nodes
#
#                                                            Wouter Kijn, 2012
#                                                               klijn@astron.nl
#------------------------------------------------------------------------------

import os
import sys

import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.group_data import load_data_map, store_data_map
from lofarpipe.support.lofarexceptions import PipelineException

class MasterNodeInterface(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Abstract class for master script collecting functionality regarding
    master node communication in a single interface

    The abstract part of this class definition indicates that this class is
    intended only to be a base class of other classes: It contains basic
    functionality with a number of stubs to be implemented in the inheriting
    class:
    on_error(this) : Called when a node recipe returned with an invalid return
                     code
    on_succes(this): Called when all node recipes returned with a correct
                     return code
    TODO: Suggested improvements
    on_partial_succes(this):   To prepare for rerun of partial runs
    on_warn(this):          To distinguish between
    """
    def __init__(self, command=None):
        """
        constructor, expects a string command used for calling the node script
        This class cannot be created with the base constructor. Inheriting
        should call this constructor with an command string
        """
        self.logger = None
        if not isinstance(command, basestring):
            # Pipeline logger NOT called: This is an 'language' type error and
            # has nothing to do with the pipelines
            raise NotImplementedError("MasterNodeInterface not constructed"
                "with a command string. This is an abstract class, inheriting"
                "class should implement an constructor calling this function"
                "with an command string")

        # call possible baseclass constructors
        super(MasterNodeInterface, self).__init__()
        self._command = command
        self._jobs = []

    def append_job(self, host, arguments):
        """
        append_job adds a job to the current job list. It expects the host,
        a list of arguments.
        """
        compute_job = ComputeJob(host, self._command, arguments)
        self._jobs.append(compute_job)

    def run_jobs(self):
        """
        Starts the set of tasks in the job lists. Call the on_error function if
        errors occured. On finish it will call the _on_succes function
        finishing the output of the recipe.
        An log message is displayed on the stdout or in a logger if the object
        contains one.
        """
        log_message = "Start scheduling jobs with command {0}".format(
                                                                self._command)
        if hasattr(self, 'logger'):
            self.logger.info(log_message)
        else:
            print log_message

        self._schedule_jobs(self._jobs)

        if self.error.isSet():
            self.on_error()
            return 1
        else:
            self.on_succes()
            return 0

    def on_error(self):
        """
        on_error should be implemented by the inheriting class. This function
        is called when the node script returned with a invalid return value
        """
        raise NotImplementedError("on_error called on abstract class"
           " MasterNodeInterface.\n Inheriting classes should implement an "
           "on_error function")

    def on_succes(self):
        """
        on_succes should be implemented by the inheriting class. This function
        is called when the node script return with a valid return value == 0
        Typical usage would be the construction of the return dictionary
        containing processed data.
        """
        raise NotImplementedError("on_succes called on abstract class"
           " MasterNodeInterface.\n Inheriting classes should implement an "
           "on_succes function")


class copier(MasterNodeInterface):
    """
    The copier recipe is used to copy paths provided in the source mapfile
    to the same node as 'matched' list provided in the target mapfile.
    The primairy use is to collect data on computation nodes, which nodes is
    sometime only specified in the mapfiles.
    There are Two operations performed by this script
    1. COPY the source path to the parent directory of
    the path provided in the target mapfilem eg: Copy instrument tables
    next to the measurement sets on which they will applied.
    To Use this operation set target_dir to "" or do not specify it
    2. COLLECT information from source nodes to a central path on different
    nodes specified in the target mapfile. eg Copy instrument tables from
    the node they are produces to the same node as the measurement sets
    privided in the target file BUT place them all in the same dir. Provide
    a target_dir for this operation: all paths not starting with /, not
    absolute     will be placed in a dir with this name relative to the
    working dir.

    **Arguments**

    A mapfile describing the data to be processed.
    """
    inputs = {
        'mapfile_source': ingredient.StringField(
            '--mapfile-source',
            help="Full path of mapfile of node:path pairs of source dataset"
        ),
        'mapfile_target': ingredient.StringField(
            '--mapfile-target',
            help="Full path of mapfile of node:path pairs of target location"
        ),
        'allow_rename': ingredient.BoolField(
            '--allow-rename',
            default=True,
            help="Allow renaming of basename at target location"
        ),
        'mapfiles_dir': ingredient.StringField(
            '--mapfiles-dir',
            help="Path of directory, shared by all nodes, which will be used"
                " to write mapfile for master-node communication"
        ),
        'target_dir': ingredient.StringField(
            '--target-dir',
            default="",
            help="Optional parameter: If this option is set ignore the"
            "path in the target mapfile. "
            "Absolute path (starting with slash): files will be copied her "
            "A relative path/name will be prepended with the working directory"
            "The filename will be used in the actual copy action"
        ),
        'working_directory': ingredient.StringField(
            '-w', '--working-directory',
            help="Working directory used on output nodes. Results location"
        ),
        'mapfile': ingredient.StringField(
            '--mapfile',
            help="full path to mapfile containing copied paths"
        ),
    }

    outputs = {
               'mapfile': ingredient.StringField()
    }

    def __init__(self):
        """
        Constructor sets the python command used to call node scripts
        """
        self.new_instrument_mapfile = None
        super(copier, self).__init__(
            "python {0}".format(self.__file__.replace('master', 'nodes')))

    ## Function expected by MasterNodeInterface
    def on_error(self):
        """
        On_error is called if a run fails on one of the nodes
        """
        self.logger.error("On of the node copier runs failed. \n"
                          "Exiting copier master recipe")

    ## Function expected by MasterNodeInterface
    def on_succes(self):
        """
        on_succes is called after all jobs have finished and
        no errors have been encountered.
        It constructs the output to be generated from this recipe based on
        results from the node script
        """
        store_data_map(self.inputs['mapfile'], self.new_instrument_mapfile)
        self.logger.debug(
                "wrote mapfile with (to be copied) files: {0}".format(
                        self.inputs['mapfile']))
        self.logger.info("copier exiting with succesfull run")
        self.outputs['mapfile'] = self.inputs['mapfile']

    def go(self):
        self.logger.info("Starting copier run")
        super(copier, self).go()

        # Load data from mapfiles
        source_map = load_data_map(self.inputs['mapfile_source'])
        target_map = load_data_map(self.inputs['mapfile_target'])
        mapfile_dir = self.inputs["mapfiles_dir"]

        # validate data in mapfiles
        if not self._validate_source_target_mapfile(source_map, target_map,
                                             self.inputs['allow_rename']):
            return 1

        # At this location the working dir is know: construct the abs target
        # dir
        target_dir = self._construct_target_dir(self.inputs["target_dir"],
                                     self.inputs["working_directory"])

        # 'sort' the input data based on node
        source_target_dict, new_instrument_mapfile = \
            self._create_target_node_keyed_dict(
                        source_map, target_map, target_dir)

        #assign to local value for later outpu
        self.new_instrument_mapfile = new_instrument_mapfile

        # Create node specific mapfiles
        mapfiles_dict = self._construct_node_specific_mapfiles(
                      source_target_dict, mapfile_dir)

        # Run the compute nodes with the node specific mapfiles
        for host, (source_mapfile, target_mapfile) in mapfiles_dict.items():
            args = [source_mapfile, target_mapfile]
            self.append_job(host, args)

        # start the jobs
        exit_value_jobs = self.run_jobs()
        return exit_value_jobs

    def _construct_target_dir(self, input_path, working_dir):
        if input_path == "":
            return input_path
        if os.path.isabs(input_path):
            return input_path
        else:
            if '/' in input_path:
                self.logger.warn("Using nested relative paths "
                    "is not supported!")
                raise PipelineException(
                    "Nested relative target path, not supported: {0}".format(
                            input_path))
            return os.path.join(working_dir, input_path)

    def _validate_source_target_mapfile(self, source_map, target_map,
                                        allow_rename=False):
        """
        Validation the input source and target files, are they the same size
        And if rename is not allowed, test if the 'file names' are the same
        """
        # Same length? Of no then fail
        if len(source_map) != len(target_map):
            self.logger.error("Number of entries in the source and target map"
                "Is not the same: \n target \n {0}\n source \n {1}".format(
                            target_map, source_map))
            return False

        # Construct mapfiles for each node containing the source and target
        for source_pair, target_pair in zip(source_map, target_map):
            target_node, target_path = target_pair
            source_node, source_path = source_pair

            # skip strict checking of basename equality of rename is allowed
            if not allow_rename:
                target_name = os.path.basename(target_path)
                source_name = os.path.basename(source_path)
                if not (target_name == source_name):
                    self.logger.error("One of the suplied source target pairs"
                        "contains a different 'filename': {0} != {1}\n"
                        " aborting".format(target_name, source_name))
                    return False

        return True

    def _create_target_node_keyed_dict(self, source_map, target_map,
                                        target_dir):
        """
        Create a dictionary of arrays keyed on the target node.
        Each array contains pairs of target and source maps entries(each a node
        path pair)
        In this function the containing dir is created from the target path
        or the new 'suplied' path is added as the target path
        Also, create the output mapfile with new instrument locations
        this needs to be done here for the order is stil ok here.
        TODO: Refactor functionality into two function with joined for loop
        """
        node_source_node_target_dict = {}
        new_instrument_mapfile = []

        for source_pair, target_pair in zip(source_map, target_map):
            target_node, target_path = target_pair
            source_node, source_path = source_pair

            # remove the Observation specific details in the path
            # In previous function the basename are already tested to be
            # the same
            if target_dir == "":
                new_target_pair = (target_node, os.path.dirname(target_path))
            else:
                new_target_pair = (target_node, target_dir)

            full_target_path = os.path.join(new_target_pair[1],
                    os.path.basename(target_path))

            new_instrument_mapfile.append((target_node, full_target_path))

            # If the data is already on the correct node, skip this file
            # It is added to the 'output' (new_instrument_mapfile)
            if target_node == source_node and target_path == source_path:
                continue

            # Check if current target is already known
            if target_node in node_source_node_target_dict:
                node_source_node_target_dict[target_node].append(
                    (source_pair, new_target_pair))
            else:
                node_source_node_target_dict[target_node] = [(source_pair,
                                                             new_target_pair)]

        return node_source_node_target_dict, new_instrument_mapfile

    def _construct_node_specific_mapfiles(self, source_target_dict,
                    mapfile_dir):
        """
        Write node specific mapfiles in the public mapfile dir. this function
        expects a dict keyed on the target node. Each entry in the dict should
        contain for that node a list tupels each containing a node, path pair.
        The first entry in the tupel should contain the source.
        """
        mapfile_dict = {}
        for target_node, source_target_list in source_target_dict.items():
            # use unzip of the source_target_list to get the mapfiles as sets
            # (automagically)
            source_map_set, target_map_set = zip(*source_target_list)
            # convert to list
            source_map = list(source_map_set)
            target_map = list(target_map_set)

            # Construct the mapfile paths
            source_name = "copier_source_{0}.map".format(target_node)
            source_mapfile_path = os.path.join(mapfile_dir, source_name)

            target_name = "copier_target_{0}.map".format(target_node)
            target_mapfile_path = os.path.join(mapfile_dir, target_name)

            # Write the mapfiles
            store_data_map(target_mapfile_path, target_map)
            self.logger.debug("Wrote mapfile with node specific target"
                              " paths: {0}".format(target_mapfile_path))

            store_data_map(source_mapfile_path, source_map)
            self.logger.debug("Wrote mapfile with node specific source"
                              " paths: {0}".format(source_mapfile_path))

            # Save the locations
            mapfile_dict[target_node] = (source_mapfile_path,
                                          target_mapfile_path)

        return mapfile_dict


if __name__ == '__main__':

    sys.exit(copier().main())
