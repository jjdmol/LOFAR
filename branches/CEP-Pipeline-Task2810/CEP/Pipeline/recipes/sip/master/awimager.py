#                                                         LOFAR IMAGING PIPELINE
#
#                                    Example recipe with simple job distribution
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

import os
import sys
import collections
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.group_data import load_data_map

class AWImager(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Run the AWImager on the nodes and the data files suplied in the mapfile
    **Arguments**
    A mapfile containing node->datafile pairs

    """
    inputs = {
        'executable': ingredient.ExecField(
            '--executable',
            help="The full path to the relevant AWImager executable"
        ),
        'initscript': ingredient.FileField(
            '--initscript',
            help='''The full path to an (Bourne) shell script which will\
             intialise the environment (ie, ``lofarinit.sh``)'''
        ),
        'parset': ingredient.FileField(
            '-p', '--parset',
            help="The full path to a AWImager configuration parset."
        ),
        'working_directory': ingredient.StringField(
            '-w', '--working-directory',
            help="Working directory used on outpuconfigt nodes. Results location"
        ),
        'suffix': ingredient.StringField(
            '--suffix',
            default=".awimager",
            help="Added to the input filename to generate the output filename"
        )  
    }
                  
    def go(self): 
        self.logger.info("Starting AWImager run")     
        super(AWImager, self).go()
              
        # Load datafile containing node --> dataset pairs     
        self.logger.debug("Loading map from %s" % self.inputs['args'])
        data = load_data_map(self.inputs['args'][0])  
        
        # Output
        outnames = collections.defaultdict(list)
        
        # Compile the command to be executed on the remote machine
        node_command = "python %s" % (self.__file__.replace("master", "nodes"))
        
        # Environment type variables
        executable = self.inputs['executable']  #absolute path, daily build
        init_script = self.inputs['initscript']
        parset = self.inputs['parset']                  #Using parameter set             
        working_dir = self.inputs['working_directory']     
             

        # Create the jobs
        jobs = []
        for host, measurement_set in data:
            #construct and save the output name
            outnames[host].append(os.path.join(
                            self.inputs['working_directory'],
                            self.inputs['job_name'],
                            os.path.basename(measurement_set.rstrip('/')) + 
                            self.inputs['suffix']))
            
            arguments=[executable, init_script, parset, working_dir, \
                       measurement_set]
            
            jobs.append(ComputeJob(host, node_command, arguments))
        
        # Hand over the job(s) to the pipeline scheduler
        self._schedule_jobs(jobs)
        
        #todo: What to do with the output??
        
        # Test for errors
        if self.error.isSet():
            self.logger.warn("Failed AWImager run detected")
            return 1
        else:
            return 0

if __name__ == "__main__":
    sys.exit(AWImager().main())
