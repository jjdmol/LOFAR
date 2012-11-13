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
from lofarpipe.support.data_map import DataMap


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
        # TODO: Due it limmitation in DataMap slicePath is here a list
        # of tuple (host, [list of paths], skip)
        slice_paths_map = eval(open(self.inputs["slice_paths_mapfile"]).read())
        input_map = DataMap.load(self.inputs['args'][0])
        # TODO; We miss input validation here due to DATAMAP
#        if self._validate_input_data(input_map):
#            return 1

        # Run the nodes with now collected inputs
        jobs = self._run_create_dbs_node(input_map, slice_paths_map,
                                           assoc_theta)

        # Collect the output of the node scripts write to (map) files
        return self._collect_and_assign_outputs(jobs)


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
        for (input_item, slice_item)  in zip(input_map, slice_paths_map):
            host_ms, concatenated_measurement_set = input_item.host, \
                input_item.file
            host_slice, slice_paths = slice_item[0], slice_item[1]

            # Create the parameters depending on the input_map
            sourcedb_target_path = os.path.join(
                  concatenated_measurement_set + self.inputs["sourcedb_suffix"])

            # The actual call for the node script
            arguments = [concatenated_measurement_set,
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
        self._schedule_jobs(jobs)

        return jobs

    def _collect_and_assign_outputs(self, jobs):
        """
        Collect and combine the outputs of the individual create_dbs node
        recipes. Combine into output mapfiles and save these at the supplied
        path locations       
        """
        # Collect the output of the node scripts write to (map) files
        sourcedb_files = []
        parmdbs = []
        # now parse the node output append to list
        for job in jobs:
            host = job.host
            if job.results.has_key("sourcedb"):
                sourcedb_files.append(tuple([host, job.results["sourcedb"], False]))
            else:
                self.logger.warn("Warning failed ImagerCreateDBs run "
                    "detected: No sourcedb file created, {0} continue".format(
                                                                        host))

            if job.results.has_key("parmdbms"):
                parmdbs.append(tuple([host, job.results["parmdbms"], False]))
            else:
                self.logger.warn("Failed ImagerCreateDBs run detected: No "
                                 "parmdbms created{0} continue".format(host))

        # Fail if none of the nodes returned all data
        if len(sourcedb_files) == 0 or len(parmdbs) == 0:
            self.logger.error("The creation of dbs on the nodes failed:")
            self.logger.error("Not a single node produces all needed data")
            self.logger.error("products. sourcedb_files: {0}    ".format(
                                                        sourcedb_files))
            self.logger.error("parameter dbs: {0}".format(parmdbs))
            return 1

        # write the mapfiles     
        DataMap(sourcedb_files).save(self.inputs["sourcedb_map_path"])
        # TODO: Parmdbs are created for each timeslice: DataMap does not
        # allow multiple return yet
        #store_data_map(self.inputs["parmdbs_map_path"], parmdbs)
        fp = open(self.inputs["parmdbs_map_path"], 'w')
        fp.write(repr(parmdbs))
        fp.close

        self.logger.debug("Wrote sourcedb dataproducts: {0} \n {1}".format(
            self.inputs["sourcedb_map_path"], self.inputs["parmdbs_map_path"]))

        # Set the outputs
        self.outputs['sourcedb_map_path'] = self.inputs["sourcedb_map_path"]
        self.outputs['parmdbs_map_path'] = self.inputs["parmdbs_map_path"]

        return 0

if __name__ == "__main__":
    sys.exit(imager_create_dbs().main())
