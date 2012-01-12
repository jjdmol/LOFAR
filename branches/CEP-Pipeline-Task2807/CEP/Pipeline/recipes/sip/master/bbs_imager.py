#                                                         LOFAR IMAGING PIPELINE
#
#  Example recipe with simple job distribution
#                                                          Wouter Klijn, 2012
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------
# python bbs_imager.py ~/build/preparation/output.map --job bbs_imager --config ~/build/preparation/pipeline.cfg --initscript /opt/cep/LofIm/daily/lofar/lofarinit.sh --parset ~/build/preparation/parset.par --working-directory "/data/scratch/klijn" --executable /opt/cep/LofIm/daily/lofar/bin/bbs_imager -d
# the measurement set with input should be located in the working directory

import os
import sys
import collections
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.group_data import load_data_map

class bbs_imager(BaseRecipe, RemoteCommandRecipeMixIn):
    """

    """
    inputs = {
        'executable': ingredient.FileField(
            '--executable',
            help = "The full path to the relevant gsmutils.py script"
        ),
        'db_host': ingredient.StringField(
            '--db-host',
            dest = "db_host",
            help = "Database host with optional port (e.g. ldb001:5432)"
        ),
              }
    def __init__(self):
        super(bbs_imager, self).__init__()

    def go(self):
        super(bbs_imager, self).go()
        self.logger.info("Starting bbs_imager run")
        self.logger.error("***********************************************")
        self.logger.info(self.inputs)
        self.logger.error("***********************************************")

        return 1
        input_map = eval(open(self.inputs['args'][0]).read())
        suffix = ".todo"
        # Compile the command to be executed on the remote machine
        node_command = "python %s" % (self.__file__.replace("master", "nodes"))

        # Create the jobs
        jobs = []
        outnames = collections.defaultdict(list)
        for host, measurement_set in input_map:
            #construct and save the output name
            outnames[host].append(measurement_set + suffix)
            arguments = ["testest"]
            self.logger.info(host)
            self.logger.info(node_command)
            self.logger.info(arguments)
            jobs.append(ComputeJob(host, node_command, arguments))

        # Hand over the job(s) to the pipeline scheduler
        self._schedule_jobs(jobs)

        # Test for errors
        #TODO: Er moeten nog tests worden gegeven.
        #TODO: Er is nog geen output
        if self.error.isSet():
            self.logger.warn("Failed bbs_imager run detected")
            return 1
        else:
            return 0


if __name__ == "__main__":
    sys.exit(bbs_imager().main())
