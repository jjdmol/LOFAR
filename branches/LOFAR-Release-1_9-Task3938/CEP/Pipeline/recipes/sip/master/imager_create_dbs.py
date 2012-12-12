# LOFAR AUTOMATIC IMAGING PIPELINE
# imager_create_dbs (master)
# Wouter Klijn, 2012
# klijn@astron.nl
# ------------------------------------------------------------------------------
import os
import sys

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
        jobs = self._run_create_dbs_node(input_map, slice_paths_map,
                                           assoc_theta)

        # Collect the output of the node scripts write to (map) files
        return self._collect_and_assign_outputs(jobs, input_map,
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
        #slice_paths_map.iterator = input_map.iterator = DataMap.SkipIterator
        for (input_item, slice_item) in zip(input_map, slice_paths_map):
            if input_item.skip or slice_item.skip:
                jobs.append(None)
                continue
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

        return jobs

    def _collect_and_assign_outputs(self, jobs, input_map, slice_paths_map):
        """
        Collect and combine the outputs of the individual create_dbs node
        recipes. Combine into output mapfiles and save these at the supplied
        path locations       
        """
        # Collect the output of the node scripts write to (map) files
        sourcedb_files = []
        parmdbs = []
        # now parse the node output append to list
        for (input_item, slice_item, job) in zip(input_map, slice_paths_map,
                                                 jobs):
            if job != None:
                node_succeeded = job.results.has_key("parmdbms") and \
                    job.results.has_key("sourcedb")
            else:
                node_succeeded = False
            host = input_item.host

            # The current job has to be skipped (due to skip field)
            # Or if the node failed:
            if input_item.skip or slice_item.skip or not node_succeeded:
                # Log failing nodes
                if not node_succeeded:
                    self.logger.warn("Warning failed ImagerCreateDBs run "
                    "detected: No sourcedb file created, {0} continue".format(
                                                            host))
                sourcedb_files.append(tuple([host, job.results["sourcedb"],
                                              True]))
                parmdbs.append(tuple([host, job.results["parmdbms"], True]))

            # Else it succeeded and we can write te results
            else:
                sourcedb_files.append(tuple([host, job.results["sourcedb"],
                                             False]))
                parmdbs.append(tuple([host, job.results["parmdbms"], False]))

        # Fail if none of the nodes returned all data
        if len(sourcedb_files) == 0 or len(parmdbs) == 0:
            self.logger.error("The creation of dbs on the nodes failed:")
            self.logger.error("Not a single node produces all needed data")
            self.logger.error(
                "products. sourcedb_files: {0}".format(sourcedb_files))
            self.logger.error("parameter dbs: {0}".format(parmdbs))
            return 1

        # write the mapfiles     
        DataMap(sourcedb_files).save(self.inputs["sourcedb_map_path"])
        MultiDataMap(parmdbs).save(self.inputs["parmdbs_map_path"])
        self.logger.debug("Wrote sourcedb dataproducts: {0} \n {1}".format(
            self.inputs["sourcedb_map_path"], self.inputs["parmdbs_map_path"]))

        # Set the outputs
        self.outputs['sourcedb_map_path'] = self.inputs["sourcedb_map_path"]
        self.outputs['parmdbs_map_path'] = self.inputs["parmdbs_map_path"]

        return 0

if __name__ == "__main__":
    sys.exit(imager_create_dbs().main())
