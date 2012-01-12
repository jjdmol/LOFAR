## LOFAR AWIMAGER RECIPE
## Wouter Klijn 2012
## klijn@astron.nl
## ------------------------------------------------------------------------------
#
from __future__ import with_statement
import os
import sys
import errno
#import tempfile
import subprocess
#import shutil
#import os.path 
#import math
#
from lofarpipe.support.lofarnode import LOFARnodeTCP
#from lofarpipe.support.pipelinelogging import CatchLog4CPlus
#from lofarpipe.support.pipelinelogging import log_time
#from lofarpipe.support.utilities import patch_parset
#from lofarpipe.support.utilities import get_parset
#from lofarpipe.support.utilities import read_initscript
#from lofarpipe.support.utilities import catch_segfaults
#from lofar.parameterset import parameterset #@UnresolvedImport #marcel new im
import pyrap.tables as pt                   #@UnresolvedImport
#from subprocess import CalledProcessError 
from lofarpipe.support.pipelinelogging import log_process_output

template_parmdb = """
create tablename="{0}"
adddef Gain:0:0:Ampl  values=1.0
adddef Gain:1:1:Ampl  values=1.0
adddef Gain:0:0:Real  values=1.0
adddef Gain:1:1:Real  values=1.0
adddef DirectionalGain:0:0:Ampl  values=1.0
adddef DirectionalGain:1:1:Ampl  values=1.0
adddef DirectionalGain:0:0:Real  values=1.0
adddef DirectionalGain:1:1:Real  values=1.0
adddef AntennaOrientation values=5.497787144
quit
"""


class bbs_imager(LOFARnodeTCP):
#    def run(self, executable, init_script, parset, working_dir,
#            measurement_set, parmdb):
#        self.logger.info("Start bbs_imager run: client")
#
#        group_dir = "group_sets"  #TODO constant!!
#        # measurement_set: is dit er wel maar eentje??
#        return 0
    def run(self, input):

#        self.logger.error("***********************************************")
#        self.logger.error(dir(bbs_imager))
#        self.logger.error("***********************************************")
        return 0

    def _field_of_view(self, measurement_set):
        """
        field_of_view tests if the supplied measurement set contains
        either LBA or HBA in the antenna name and return the correct 
        field of view. Throws exception if incorrect antenna name
        """
        # open ms
        t = pt.table(measurement_set)

        # get antenna table and next the name
        antenna = pt.table(t.getkeyword("ANTENNA"))
        antenna_name = antenna.getcell('NAME', 0)
        antenna.close()
        t.close()

        # ascertain the fov
        fov = None
        if antenna_name.count('HBA'):
            fov = 6  #(degrees)
        elif antenna_name.count('LBA'):
            fov = 3
        else:
            self.logger.error('unknown antenna type for antenna: {0}'.format(\
                                                                antenna_name))
            raise Exception("Unknonw antenna type encountered in Measurement set")

        return fov

    def _create_parmdb(self, parmdb_executable, target_dir_path):
        """
        _create_parmdb, creates a parmdb_executable at the target_dir_path using the
        suplied executable. Does not test for exsistence of target parent dir       
        returns 1 if parmdb_executable failed 0 otherwise
        """
        #Format the template string by inserting the target dir
        formatted_template = template_parmdb.format(target_dir_path)
        try:
            #spawn a subprocess and connect the pipelines
            parmdbm_process = subprocess.Popen(
                parmdb_executable,
                stdin = subprocess.PIPE,
                stdout = subprocess.PIPE,
                stderr = subprocess.PIPE
            )
            #send formatted template on stdin
            sout, serr = parmdbm_process.communicate(formatted_template)

            #log the output
            log_process_output("parmdbm", sout, serr, self.logger)
        except OSError, e:
            self.logger.error("Failed to spawn parmdbm: %s" % str(e))
            return 1

        return 0

    def _create_parmdb_for_timeslices(self, parmdb_executable, path,
                                      n_time_slices, suffix):
        """
        _create_parmdb_for_timeslices 
        Creates for each of the time slices a paramdb in the suplied path 
        based on the "original filename" + suffix
        "original filename" is a constant: time_slice_{0}.dppp.ms
        """
        for idx_time_slice in xrange(n_time_slices):
            #Create the directory name and construct the path
            ms_parmdb = "time_slice_{0}.dppp.ms{1}".format(idx_time_slice, suffix)
            ms_parmdb_path = os.path.join(path, ms_parmdb)
            #call parmdb return one of a single create failed 
            if self._create_parmdb(parmdb_executable, ms_parmdb_path) != 0:
                return 1

        return 0


#    def _create_monat_db_connection(self):
#
#    def _create_sourcedb(self):
#        #/opt/cep/LofIm/daily/lofar/lib/python2.6/dist-packages/lofar/gsmutils.py
#        pass





if __name__ == "__main__":
    #pass
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(bbs_imager(jobid, jobhost, jobport).run_with_stored_arguments())

