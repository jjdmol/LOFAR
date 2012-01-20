#                                                         LOFAR IMAGING PIPELINE
#
#  Example recipe with simple job distribution
#                                                          Wouter Klijn, 2012
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------
# python bbs_imager.py ~/build/preparation/output.map --job imager_create_dbs --config ~/build/preparation/pipeline.cfg --initscript /opt/cep/LofIm/daily/lofar/lofarinit.sh --parset ~/build/preparation/parset.par --working-directory "/data/scratch/klijn" --executable /opt/cep/LofIm/daily/lofar/bin/bbs_imager -d
# the measurement set with input should be located in the working directory

import os
import sys
import collections
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.group_data import load_data_map

class imager_create_dbs(BaseRecipe, RemoteCommandRecipeMixIn):
    """

    """
    inputs = {
        # common inputs
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
            default = ".imager_create_dbs",
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
            '--slice_paths_mapfile',
            help = "Location of the mapfile containing the slice paths"
        ),
        'parmdb_suffix': ingredient.StringField(
            '--parmdb-suffix',
            help = "suffix of the to be created paramdbs"
        ),
        }


    def __init__(self):
        super(imager_create_dbs, self).__init__()

    def go(self):
        super(imager_create_dbs, self).go()
        suffix = self.inputs["suffix"]

        # collect and assign the parameters for the         

        # Monet database parameters
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

        #Parse the mapfile containing the timeslices: get the actual
        #paths and collect in an array. Supply nodescript with repr if this
        slice_paths_mapfile = self.inputs["slice_paths_mapfile"]
        slice_paths_dict = eval(open(slice_paths_mapfile).read())
        slice_paths = []
        for idx, path in slice_paths_dict:
            slice_paths.append(path)
        slice_paths = repr(slice_paths)

        parmdb_suffix = self.inputs["parmdb_suffix"]

        # Parse the input map
        input_map = eval(open(self.inputs['args'][0]).read())


        # Compile the command to be executed on the remote machine
        node_command = "bash -c '. ${APS_LOCAL}/login/loadpackage.bash LofIm "\
           "; . ${APS_LOCAL}/login/loadpackage.bash MonetDB; " \
           " export PYTHONPATH=~/build/gnu_debug/installed/lib/python2.6/dist-packages/lofar:$PYTHONPATH ; python %s'" % (self.__file__.replace("master", "nodes"))

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
                         parmdb_executable, slice_paths, parmdb_suffix]
            jobs.append(ComputeJob(host, node_command, arguments))

        # Hand over the job(s) to the pipeline scheduler
        self._schedule_jobs(jobs)

        # Test for errors
        #TODO: Er moeten nog tests worden gegeven.
        #TODO: Er is nog geen output
        if self.error.isSet():
            self.logger.warn("Failed imager_create_dbs run detected")
            return 1
        else:
            return 0


if __name__ == "__main__":
    sys.exit(imager_create_dbs().main())
