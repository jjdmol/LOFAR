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
        'suffix': ingredient.StringField(
            '--suffix',
            default = ".ImagerCreateDBs",
            help = "Added to the input filename to generate the output filename"
        ),

        # recipe specific inputs
        'sourcedb_target_path': ingredient.StringField(
            '--sourcedb-target-path',
            default = "",
            help = "location to save the sourcedb"
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
        'parmdbs_path': ingredient.StringField(
            '--parmdbs-path',
            help = "Location of the mapfile to place the outputfile with the paths for each slice pardbm"
        ),
        'sky_path': ingredient.StringField(
            '--sky-path',
            help = "Location of the mapfile to place the outputfile with sky files (sky map)"
        ),
        'parmdb_suffix': ingredient.StringField(
            '--parmdb-suffix',
            help = "suffix of the to be created paramdbs"
        ),
        'monetdb_path': ingredient.StringField(
            '--monetdb-path',
            help = "initializing for the monetdb"
        ),
        'gsm_path': ingredient.StringField(
            '--gsm-path',
            help = "initializing for the gsm"
        ),
        'makesourcedb_path': ingredient.ExecField(
             '--makesourcedb-path',
             help = "Path to makesourcedb executable."
        ),
    }

    outputs = {
        'sky_path': ingredient.FileField(),
        'parmdbs_path': ingredient.FileField()
    }



    def __init__(self):
        super(imager_create_dbs, self).__init__()

    def go(self):
        # TODO: We need output
        super(imager_create_dbs, self).go()
        suffix = self.inputs["suffix"]

        # collect and assign the parameters for the         
        # Monet database 
        monetdb_hostname = self.inputs["monetdb_hostname"]
        monetdb_port = self.inputs["monetdb_port"]
        monetdb_name = self.inputs["monetdb_name"]
        monetdb_user = self.inputs["monetdb_user"]
        monetdb_password = self.inputs["monetdb_password"]
        monetdb_path = self.inputs["monetdb_path"]
        gsm_path = self.inputs["gsm_path"]

        if self.inputs["assoc_theta"] == "":
            assoc_theta = None
        else:
            assoc_theta = self.inputs["assoc_theta"]

        parmdb_executable = self.inputs["parmdb_executable"]

        #Parse the mapfile containing the timeslices: get the actual
        #paths and collect in an array.
        slice_paths_mapfile = self.inputs["slice_paths_mapfile"]
        slice_paths_dict = eval(open(slice_paths_mapfile).read())
        slice_paths = []
        for idx, path in slice_paths_dict:
            slice_paths.append(path)
        slice_paths = repr(slice_paths)

        parmdb_suffix = self.inputs["parmdb_suffix"]
        init_script = self.inputs["initscript"]
        working_directory = self.inputs["working_directory"]
        makesourcedb_path = self.inputs["makesourcedb_path"]
        # Parse the input map
        input_map = eval(open(self.inputs['args'][0]).read())

        # Compile the command to be executed on the remote machine
        node_command = " python %s" % (self.__file__.replace("master", "nodes"))

        # Create the jobs
        jobs = []
        outnames = collections.defaultdict(list)
        for host, concatenated_measurement_set in input_map:
            #Create the parameters depending on the input_map
            if self.inputs["sourcedb_target_path"] != "":
                sourcedb_target_path = self.inputs["sourcedb_target_path"]
            else:
                sourcedb_target_path = os.path.join(
                        concatenated_measurement_set + ".sky")


            #construct and save the output name
            outnames[host].append(concatenated_measurement_set + suffix)
            arguments = [ concatenated_measurement_set, sourcedb_target_path,
                         monetdb_hostname, monetdb_port, monetdb_name,
                         monetdb_user, monetdb_password, assoc_theta,
                         parmdb_executable, slice_paths, parmdb_suffix,
                         monetdb_path, gsm_path, init_script, working_directory,
                         makesourcedb_path]
            jobs.append(ComputeJob(host, node_command, arguments))

        # Hand over the job(s) to the pipeline scheduler
        self._schedule_jobs(jobs)

        #Collect the output of the node scripts write to (map) files
        sky_files = []
        parmdbs = []
        # now parse the output append to lits
        for job in jobs:
            host = job.host
            if job.results.has_key("sky"):
                sky_files.append([host, job.results["sky"]])
            else:
                self.logger.warn("Warning ImagerCreateDBs run detected: No "
                                 "skymap file created, {0} continue".format(host))

            if job.results.has_key("parmdbms"):
                for parmdb in job.results["parmdbms"]:
                    parmdbs.append([host, parmdb])
            else:
                self.logger.warn("Failed ImagerCreateDBs run detected: No "
                                 "parmdbms created{0} continue".format(host))

        # write the mapfiles     
        store_data_map(self.inputs["sky_path"], sky_files)
        store_data_map(self.inputs["parmdbs_path"], parmdbs)

        # return the output path names
        self.outputs['sky_path'] = self.inputs["sky_path"]
        self.outputs['parmdbs_path'] = self.inputs["parmdbs_path"]

        # Test for errors
        if self.error.isSet():
            self.logger.warn("Failed ImagerCreateDBs run detected")
            return 1
        else:
            return 0


if __name__ == "__main__":
    sys.exit(imager_create_dbs().main())
