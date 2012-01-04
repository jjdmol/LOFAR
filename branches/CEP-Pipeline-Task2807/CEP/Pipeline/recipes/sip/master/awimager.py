#                                                         LOFAR IMAGING PIPELINE
#
#                                    Example recipe with simple job distribution
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------
# python awimager.py ~/build/preparation/output.map --job awimager --config ~/build/preparation/pipeline.cfg --initscript /opt/cep/LofIm/daily/lofar/lofarinit.sh --parset ~/build/preparation/parset.par --working-directory "/data/scratch/klijn" --executable /opt/cep/LofIm/daily/lofar/bin/awimager -d
# the measurement set with input should be located in the working directory

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
        super(AWImager, self).go()
        self.logger.info("Starting AWImager run")
        
        #collect the inputs        
        # TODO:  input_map to be merged with change marcel
        input_map = eval(open(self.inputs['args'][0]).read()) 
        executable = self.inputs['executable']  
        init_script = self.inputs['initscript']
        parset = self.inputs['parset']            
        working_dir = self.inputs['working_directory']  
        job_name = self.inputs['job_name']
        suffix = self.inputs['suffix']
        
        #Get the base output fileneam from the parset
        parset_datamap = load_data_map(parset)
        
        self.logger.info(repr(parset_datamap))
        return 0
        
        # Compile the command to be executed on the remote machine
        node_command = "python %s" % (self.__file__.replace("master", "nodes"))
        
        # Create the jobs
        jobs = []
        outnames = collections.defaultdict(list)
        for host, measurement_set in input_map:
            self._validate_correct_data_location(measurement_set, working_dir,
                                                  host)
            #construct and save the output name
            outnames[host].append(os.path.join(working_dir, job_name,
                            os.path.basename(measurement_set.rstrip('/')) + 
                            suffix))
            
            
            arguments=[executable, init_script, parset, working_dir, 
                       measurement_set]
            
            jobs.append(ComputeJob(host, node_command, arguments))
        
        # Hand over the job(s) to the pipeline scheduler
        self._schedule_jobs(jobs)       

        # Test for errors
        if self.error.isSet():
            self.logger.warn("Failed AWImager run detected")
            return 1
        else:
            return 0
    
    
    def _validate_correct_data_location(self, measurement_set, working_dir, 
                                        host):
        """
        tests if the input ms is available in the working directory.
        awimager fail if this is not the case
        """  
        if not (working_dir == '/'.join( #kan worden gedaan met  os.path.basename
                                measurement_set.rstrip('/').split('/')[:-1])):
            raise Exception("{0}: Incorrect input(s): Measurement set is not"
                             " located in working directory".format(host))
        

if __name__ == "__main__":
    sys.exit(AWImager().main())
