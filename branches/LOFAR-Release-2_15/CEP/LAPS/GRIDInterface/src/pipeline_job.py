"""
DPU pipeline jobs that can be used by the DPU XML interface.
"""

import subprocess
import time

"""
Base class containing the main functionality for a pipeline job.
"""
class pipeline_job:

    def __init__(self, command='', parset={}, name=''):
        self.command = command
        self.parset_as_dict = parset
        self.name = name
        
    def execute(self):
        pass


"""
Pipeline job that can be used to run on the CEP cluster. It assumes that the
data is already stored on the node where the job is running and that an
appropriate startPython.sh script is available to start pipeline runs. 
"""
class cep_pipeline_job(pipeline_job):

    def execute(self):
        f = open(self.name, "w")
        for key,value in self.parset_as_dict.items():
            f.write(key + "=" + str(value) + "\n")
        f.close()
        
        #time.sleep(15)
        
        p = subprocess.Popen("startPython.sh " + self.command + " " + self.name + " 0 0 0", stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
        self.stdout, self.stderr = p.communicate()
        # Clean up statefile etc
       
