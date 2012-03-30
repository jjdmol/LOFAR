#                                                         LOFAR IMAGING PIPELINE
#
#  imager_create_dbs (master)
#
#                                                          Wouter Klijn, 2012
#                                                                klijn@astron.nl
# ------------------------------------------------------------------------------
import os
import sys
import collections
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.group_data import load_data_map, store_data_map

class imager_create_dbs(BaseRecipe, RemoteCommandRecipeMixIn):
    """

    """
    inputs = {
        'working_directory': ingredient.StringField(
            '-w', '--working-directory',
            help = "Working directory used on outpuconfigt nodes. Results location"
        ),
         'initscript': ingredient.FileField(
            '--initscript',
            help = '''The full path to an (Bourne) shell script which will\
             intialise the environment (ie, ``lofarinit.sh``)'''
        ),
        'parset': ingredient.FileField(
            '-p', '--parset',
            help = "The full path to a bbs_imager configuration parset."
        ),
        'sourcedb_suffix': ingredient.StringField(
            '--sourcedb-suffix',
            default = ".sky",
            help = "suffix for created sourcedbs"
        ),
        'monetdb_hostname': ingredient.StringField(
            '--monetdb-hostname',
            help = "Hostname of monet database"
        ),
        'monetdb_port': ingredient.IntField(
            '--monetdb-port',
            help = "port for monet database"
        ),
        'monetdb_name': ingredient.StringField(
            '--monetdb-name',
            help = "db name of monet database"
        ),
        'monetdb_user': ingredient.StringField(
            '--monetdb-user',
            help = "user on the monet database"
        ),
        'monetdb_password': ingredient.StringField(
            '--monetdb-password',
            help = "password on monet database"
        ),
        'assoc_theta': ingredient.StringField(
            '--assoc-theta',
            default = "",
            help = "assoc_theta is used in creating the skymodel, default == None"
        ),
        'parmdb_executable': ingredient.ExecField(
            '--parmdbm-executable',
            help = "Location of the parmdb executable"
        ),
        'slice_paths_mapfile': ingredient.FileField(
            '--slice-paths-mapfile',
            help = "Location of the mapfile containing the slice paths"
        ),
        'parmdb_suffix': ingredient.StringField(
            '--parmdb-suffix',
            help = "suffix of the to be created paramdbs"
        ),
        'makesourcedb_path': ingredient.ExecField(
             '--makesourcedb-path',
             help = "Path to makesourcedb executable."
        ),
        'source_list_path': ingredient.StringField(
             '--source-list-path',
             help = "Path to sourcelist from external source (eg. bdsm) "\
             "use an empty string for gsm generated data"
        ),
        'parmdbs_path': ingredient.StringField(
            '--parmdbs-path',
            help = "path to mapfile containing produced sky files"
        ),
        'sky_path': ingredient.StringField(
            '--sky-path',
            help = "path to mapfile containing produced sky files"
        ),

    }

    outputs = {
        'sky_path': ingredient.FileField(),
        'parmdbs_path': ingredient.FileField()
    }


    def __init__(self):
        super(imager_create_dbs, self).__init__()

    def go(self):
        super(imager_create_dbs, self).go()

        # ********************************************************************
        # collect the inputs into local variables
        monetdb_hostname = self.inputs["monetdb_hostname"]
        monetdb_port = self.inputs["monetdb_port"]
        monetdb_name = self.inputs["monetdb_name"]
        monetdb_user = self.inputs["monetdb_user"]
        monetdb_password = self.inputs["monetdb_password"]

        if self.inputs["assoc_theta"] == "":
            assoc_theta = None
        else:
            assoc_theta = self.inputs["assoc_theta"]

        parmdb_executable = self.inputs["parmdb_executable"]
        parmdb_suffix = self.inputs["parmdb_suffix"]
        init_script = self.inputs["initscript"]
        working_directory = self.inputs["working_directory"]
        makesourcedb_path = self.inputs["makesourcedb_path"]
        source_list_path = self.inputs["source_list_path"]

        # Get the input data
        slice_paths_map = load_data_map(self.inputs["slice_paths_mapfile"])
        input_map = load_data_map(self.inputs['args'][0])

        # Compile the command to be executed on the remote machine
        node_command = " python %s" % (self.__file__.replace("master", "nodes"))
        # create jobs
        jobs = []
        for (input_ms, slice_paths)  in zip(input_map, slice_paths_map):
            host_ms, concatenated_measurement_set = input_ms
            host_slice, slice_paths = slice_paths

            #Check if inputs are correct.
            if host_ms != host_slice:
                self.logger.error("Host found in the timeslice and input ms are different")
                self.logger.error(input_map)
                self.logger.error(slice_paths_map)
                return 1

            host_slice_map = [("paths", slice_paths)]
            host = host_ms

            #Create the parameters depending on the input_map
            sourcedb_target_path = os.path.join(
                  concatenated_measurement_set + self.inputs["sourcedb_suffix"])

            #The actual call for the node script
            arguments = [ concatenated_measurement_set, sourcedb_target_path,
                         monetdb_hostname, monetdb_port, monetdb_name,
                         monetdb_user, monetdb_password, assoc_theta,
                         parmdb_executable, host_slice_map, parmdb_suffix,
                         init_script, working_directory,
                         makesourcedb_path, source_list_path]
            jobs.append(ComputeJob(host, node_command, arguments))

        # Hand over the job(s) to the pipeline scheduler
        self._schedule_jobs(jobs)

        #Collect the output of the node scripts write to (map) files
        sky_files = []
        parmdbs = []

        # now parse the node output append to list
        for job in jobs:
            host = job.host
            if job.results.has_key("sky"):
                sky_files.append((host, job.results["sky"]))
            else:
                self.logger.warn("Warning failed ImagerCreateDBs run detected: No "
                                 "skymap file created, {0} continue".format(host))

            if job.results.has_key("parmdbms"):
                parmdbs.append((host, job.results["parmdbms"]))
            else:
                self.logger.warn("Failed ImagerCreateDBs run detected: No "
                                 "parmdbms created{0} continue".format(host))

        # write the mapfiles     
        store_data_map(self.inputs["sky_path"], sky_files)
        store_data_map(self.inputs["parmdbs_path"], parmdbs)

        # Set the outputs
        self.outputs['sky_path'] = self.inputs["sky_path"]
        self.outputs['parmdbs_path'] = self.inputs["parmdbs_path"]
        return 0


if __name__ == "__main__":
    sys.exit(imager_create_dbs().main())
