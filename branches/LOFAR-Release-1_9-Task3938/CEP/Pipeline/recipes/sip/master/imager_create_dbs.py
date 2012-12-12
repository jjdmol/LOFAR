# LOFAR AUTOMATIC IMAGING PIPELINE
# imager_create_dbs (master)
# Wouter Klijn, 2012
# klijn@astron.nl
# ------------------------------------------------------------------------------
import os
import sys
import copy

import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.data_map import DataMap, MultiDataMap, validate_data_maps


class imager_create_dbs(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    responsible for creating a number 
    of databases needed by imaging pipeline:
    
    1. Using pointing extracted from the input measurement set a database is 
       created of sources based on information in the global sky model (gsm)
       One source db is created for each image/node:
       
       a. The pointing is supplied to to GSM database resulting in a sourcelist
       b. This sourcelist is converted into a source db
       c. Possible additional sourcelist from external sources are added to this 
          source list
    2. For each of the timeslice in image a parmdb is created. Each timeslice is 
       recorded on a different time and needs its own calibration and therefore
       instrument parameters. 
    """

    inputs = {
        'working_directory': ingredient.StringField(
            '-w', '--working-directory',
            help="Working directory used on nodes. Results location"
        ),
        'sourcedb_suffix': ingredient.StringField(
            '--sourcedb-suffix',
            default=".sky",
            help="suffix for created sourcedbs"
        ),
        'monetdb_hostname': ingredient.StringField(
            '--monetdb-hostname',
            help="Hostname of monet database"
        ),
        'monetdb_port': ingredient.IntField(
            '--monetdb-port',
            help="port for monet database"
        ),
        'monetdb_name': ingredient.StringField(
            '--monetdb-name',
            help="db name of monet database"
        ),
        'monetdb_user': ingredient.StringField(
            '--monetdb-user',
            help="user on the monet database"
        ),
        'monetdb_password': ingredient.StringField(
            '--monetdb-password',
            help="password on monet database"
        ),
        'assoc_theta': ingredient.StringField(
            '--assoc-theta',
            default="",
            help="assoc_theta is used in creating the skymodel, default == None"
        ),
        'parmdb_executable': ingredient.ExecField(
            '--parmdbm-executable',
            help="Location of the parmdb executable"
        ),
        'slice_paths_mapfile': ingredient.FileField(
            '--slice-paths-mapfile',
            help="Location of the mapfile containing the slice paths"
        ),
        'parmdb_suffix': ingredient.StringField(
            '--parmdb-suffix',
            help="suffix of the to be created paramdbs"
        ),
        'makesourcedb_path': ingredient.ExecField(
             '--makesourcedb-path',
             help="Path to makesourcedb executable."
        ),
        'source_list_path': ingredient.StringField(
             '--source-list-path',
             help="Path to sourcelist from external source (eg. bdsm) "\
             "use an empty string for gsm generated data"
        ),
        'parmdbs_map_path': ingredient.StringField(
            '--parmdbs-map-path',
            help="path to mapfile containing produced parmdb files"
        ),
        'sourcedb_map_path': ingredient.StringField(
            '--sourcedb-map-path',
            help="path to mapfile containing produced sourcedb files"
        ),
    }

    outputs = {
        'sourcedb_map_path': ingredient.FileField(
            help="On succes contains path to mapfile containing produced "
            "sourcedb files"),
        'parmdbs_map_path': ingredient.FileField(
            help="On succes contains path to mapfile containing produced"
            "parmdb files")
    }

    def __init__(self):
        super(imager_create_dbs, self).__init__()

    def go(self):
        super(imager_create_dbs, self).go()

        # get assoc_theta, convert from empty string if needed 
        assoc_theta = self.inputs["assoc_theta"]
        if assoc_theta == "":
            assoc_theta = None

        # Load mapfile data from files
        self.logger.error(self.inputs["slice_paths_mapfile"])
        slice_paths_map = MultiDataMap.load(self.inputs["slice_paths_mapfile"])
        input_map = DataMap.load(self.inputs['args'][0])

        if self._validate_input_data(input_map, slice_paths_map):
            return 1

        # Run the nodes with now collected inputs
        jobs, output_map = self._run_create_dbs_node(
                 input_map, slice_paths_map, assoc_theta)

        # Collect the output of the node scripts write to (map) files
        return self._collect_and_assign_outputs(jobs, output_map,
                                    slice_paths_map)


    def _validate_input_data(self, slice_paths_map, input_map):
        """
        Performs a validation of the supplied slice_paths_map and inputmap.
        Displays error message if this fails
        """
        validation_failed = None
        error_received = None
        try:
            validation_failed = not validate_data_maps(slice_paths_map,
                                                     input_map)
        except  AssertionError, exception :
            validation_failed = True
            error_received = str(exception)

        if validation_failed:
            self.logger.error(error_received)
            self.logger.error("Incorrect mapfiles: {0} and {1}".format(
                 self.inputs["slice_paths_mapfile"], self.inputs['args'][0]))
            self.logger.error("content input_map: \n{0}".format(input_map))
            self.logger.error("content slice_paths_map: \n{0}".format(
                                                            slice_paths_map))
            # return with failure
            return 1

        # return with zero (all is ok state) 
        return 0

    def _run_create_dbs_node(self, input_map, slice_paths_map,
                                         assoc_theta):
        """
        Decompose the input mapfiles into task for specific nodes and 
        distribute these to the node recipes. Wait for the jobs to finish and
        return the list of created jobs.
        """
        # Compile the command to be executed on the remote machine
        node_command = " python %s" % (self.__file__.replace("master", "nodes"))
        # create jobs
        jobs = []
        output_map = copy.deepcopy(input_map)

        # Update the skip fields of the four maps. If 'skip' is True in any of
        # these maps, then 'skip' must be set to True in all maps.
        for w, x, y in zip(input_map, output_map, slice_paths_map):
            w.skip = x.skip = y.skip = (
                w.skip or x.skip or y.skip
            )
        slice_paths_map.iterator = input_map.iterator = DataMap.SkipIterator
        for (input_item, slice_item) in zip(input_map, slice_paths_map):
            host_ms, concat_ms = input_item.host, input_item.file
            host_slice, slice_paths = slice_item.host, slice_item.file

            # Create the parameters depending on the input_map
            sourcedb_target_path = os.path.join(
                  concat_ms + self.inputs["sourcedb_suffix"])

            # The actual call for the node script
            arguments = [concat_ms,
                         sourcedb_target_path,
                         self.inputs["monetdb_hostname"],
                         self.inputs["monetdb_port"],
                         self.inputs["monetdb_name"],
                         self.inputs["monetdb_user"],
                         self.inputs["monetdb_password"],
                         assoc_theta,
                         self.inputs["parmdb_executable"],
                         slice_paths,
                         self.inputs["parmdb_suffix"],
                         self.environment,
                         self.inputs["working_directory"],
                         self.inputs["makesourcedb_path"],
                         self.inputs["source_list_path"]]

            jobs.append(ComputeJob(host_ms, node_command, arguments))
        # Wait the nodes to finish
        if len(jobs) > 0:
            self._schedule_jobs(jobs)

        return jobs, output_map

    def _collect_and_assign_outputs(self, jobs, output_map, slice_paths_map):
        """
        Collect and combine the outputs of the individual create_dbs node
        recipes. Combine into output mapfiles and save these at the supplied
        path locations       
        """
        # Create a container for the output parmdbs: same host and 
        output_map.iterator = DataMap.TupleIterator
        parmdbs_list = []
        # loop over the raw data including the skip file (use the data member)
        for output_entry in output_map.data:
            parms_tuple = tuple([output_entry.host, [],
                                output_entry.skip])
            parmdbs_list.append(parms_tuple)

        parmdbs_map = MultiDataMap(parmdbs_list)
        self.logger.debug()

        output_map.iterator = parmdbs_map.iterator = DataMap.SkipIterator # The maps are synced
        succesfull_run = False
        for (output_item, parmdbs_item, job) in zip(
                                                output_map, parmdbs_map, jobs):
            node_succeeded = job.results.has_key("parmdbs") and \
                    job.results.has_key("sourcedb")

            host = output_item.host

            # The current job has to be skipped (due to skip field)
            # Or if the node failed:
            if not node_succeeded:
                self.logger.warn("Warning failed ImagerCreateDBs run "
                    "detected: No sourcedb file created, {0} continue".format(
                                                            host))
                output_item.file = "failed"
                output_item.skip = True
                parmdbs_item.file = ["failed"]
                parmdbs_item.skip = True

            # Else it succeeded and we can write te results
            else:
                succesfull_run = True
                output_item.file = job.results["sourcedb"]
                parmdbs_item.file = job.results["parmdbs"]

                # we also need to manually set the skip for this new 
                # file list
                parmdbs_item.file_skip = [False] * len(job.results["parmdbs"])

        # Fail if none of the nodes returned all data
        if not succesfull_run:
            self.logger.error("The creation of dbs on the nodes failed:")
            self.logger.error("Not a single node produces all needed data")
            self.logger.error(
                "products. sourcedb_files: {0}".format(output_map))
            self.logger.error("parameter dbs: {0}".format(parmdbs_map))
            return 1

        # write the mapfiles     
        output_map.save(self.inputs["sourcedb_map_path"])
        parmdbs_map.save(self.inputs["parmdbs_map_path"])
        self.logger.debug("Wrote sourcedb dataproducts: {0} \n {1}".format(
            self.inputs["sourcedb_map_path"], self.inputs["parmdbs_map_path"]))

        # Set the outputs
        self.outputs['sourcedb_map_path'] = self.inputs["sourcedb_map_path"]
        self.outputs['parmdbs_map_path'] = self.inputs["parmdbs_map_path"]

        return 0

if __name__ == "__main__":
    sys.exit(imager_create_dbs().main())
