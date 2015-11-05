#                                                         LOFAR IMAGING PIPELINE
#
#                                  Master recipe for copying files between nodes 
#                                    
#                                                             MWouter Kijn, 2012
#                                                                klijn@astron.nl
# ------------------------------------------------------------------------------

import os
import sys

import lofarpipe.support.lofaringredient as ingredient

from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.group_data import load_data_map, store_data_map
from lofarpipe.support.group_data import validate_data_maps

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
    def __init__(self, command = None):
        """
        constructor, expects a string command used for calling the node script
        This class cannot be created with the base constructor. Inheriting 
        should call this constructor with an command string 
        """
        if not isinstance(command, basestring):
            # Pipeline logger NOT called: This is an 'language' type error and
            # has nothing to do with the pipelines
            raise NotImplementedError("MasterNodeInterface not constructed"
                "with a command string. This is an abstract class, inheriting"
                "class should implement an constructor calling this function with"
                "an comand string")


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
        errors occured. On finish it will call the _on_succes function finishing
        the output of the recipe.
        An log message is displayed on the stdout or in a logger if the object 
        contains one  
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
    Recipe to export calibration solutions, using the program `parmexportcal`.
    The main purpose of this program is to strip off the time axis information
    from a instrument model (a.k.a ParmDB)

    **Arguments**

    A mapfile describing the data to be processed.
    """
    inputs = {
        'mapfile_source': ingredient.StringField(
            '--mapfile-source',
            help = "Full path of mapfile of node:path pairs of source dataset"
        ),
        'mapfile_target': ingredient.StringField(
            '--mapfile-target',
            help = "Full path of mapfile of node:path pairs of target location"
        ),
        'allow_rename': ingredient.BoolField(
            '--allow-rename',
            default = False,
            help = "Allow renaming of basename at target location"
        ),
        'mapfile_dir': ingredient.StringField(
            '--mapfile-dir',
            help = "Path of directory, shared by all nodes, which will be used"
                " to write mapfile for master-node communication"
        )
    }

    def __init__(self):
        """
        Constructor sets the python command used to call node scripts
        """
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
        pass


    def go(self):
        self.logger.info("Starting copier run")
        super(copier, self).go()

        # Load data from mapfiles
        source_map = load_data_map(self.inputs['mapfile_source'])
        target_map = load_data_map(self.inputs['mapfile_target'])
        mapfile_dir = self.inputs["mapfile_dir"]

        # validate data in mapfiles
        if not self._validate_source_target_mapfile(source_map, target_map,
                                             self.inputs['allow_rename']):
            return 1 #return failure

        # 'sort' the input data based on node
        source_target_dict = self._create_target_node_keyed_dict(
                        source_map, target_map)

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

    def _validate_source_target_mapfile(self, source_map, target_map,
                                        allow_rename = False):
        """
        Validation the input source and target files, are they the same name
        And if rename is not allowed, test if the 'file names' are the same 
        """
        # Same length? Of no then fail
        if len(source_map) != len(target_map):
            self.logger.error("Number of entries in the source and target map"
                              "Is not the same ")
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

    def _create_target_node_keyed_dict(self, source_map, target_map):
        """
        Create a dictionary of arrays keyed on the target node.
        Each array contains pairs of target and source maps entries(each a node
        path pair)
        """
        node_source_node_target_dict = {}

        for source_pair, target_pair in zip(source_map, target_map):
            target_node, target_path = target_pair
            source_node, source_path = source_pair

            # remove the Observation specific details in the path 
            # In previous function the basename are already tested to be the same
            new_target_pair = (target_node, os.path.dirname(target_path))

            # Check if current target is already known
            if node_source_node_target_dict.has_key(target_node):
                node_source_node_target_dict[target_node].append(
                    (source_pair, new_target_pair))
            else:
                node_source_node_target_dict[target_node] = [(source_pair,
                                                             new_target_pair)]

        return node_source_node_target_dict


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

    def _create_target_map_for_instruments(self, instrument_map, input_data_map):
        # Not used by mapfile, allows quick testing of functionality
        target_map = []
        for instrument_pair, input_data_pair in zip(instrument_map, input_data_map):
            instrument_node, instrument_path = instrument_pair
            input_data_node, input_data_path = input_data_pair

            target_dir = os.path.dirname(input_data_path)
            target_name = os.path.basename(instrument_path)
            target_path = os.path.join(target_dir, target_name)
            target_map.append((input_data_node, target_path))

        return target_map


if __name__ == '__main__':

    sys.exit(copier().main())
