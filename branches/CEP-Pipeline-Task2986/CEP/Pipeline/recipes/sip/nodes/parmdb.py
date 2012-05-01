from __future__ import with_statement
from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.utilities import log_time
import shutil, os.path
import sys

class parmdb(LOFARnodeTCP):
    def run(self, pdb_in, pdb_out):
        with log_time(self.logger):
            self.logger.debug("Copying parmdb: %s --> %s" % (pdb_in, pdb_out))

            # Remove any old parmdb database
            shutil.rmtree(pdb_out, ignore_errors=True)

            # And copy the new one into place
            shutil.copytree(pdb_in, pdb_out)

        return 0

if __name__ == "__main__":
    #   If invoked directly, parse command line arguments for logger information
    #                        and pass the rest to the run() method defined above
    # --------------------------------------------------------------------------
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(parmdb(jobid, jobhost, jobport).run_with_stored_arguments())
