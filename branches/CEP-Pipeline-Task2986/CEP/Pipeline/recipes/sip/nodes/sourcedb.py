from __future__ import with_statement
from subprocess import Popen, CalledProcessError, PIPE, STDOUT
import errno
import os
import tempfile
import shutil
import sys

from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.utilities import log_time
from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.utilities import catch_segfaults


class sourcedb(LOFARnodeTCP):
    def run(self, executable, catalogue, skydb):
        with log_time(self.logger):
            # Create output directory if it does not yet exist.
            skydb_dir = os.path.dirname(skydb)
            try:
                os.makedirs(skydb_dir)
                self.logger.debug("Created output directory %s" % skydb_dir)
            except OSError, err:
                # Ignore error if directory already exists, otherwise re-raise
                if err[0] != errno.EEXIST:
                    raise

            # Remove any old sky database
            shutil.rmtree(skydb, ignore_errors=True)

            self.logger.info("Creating skymodel: %s" % (skydb))
            scratch_dir = tempfile.mkdtemp()
            try:
                cmd = [executable,
                       "format=<",
                       "in=%s" % (catalogue),
                       "out=%s" % (skydb)
                      ]
                with CatchLog4CPlus(
                    scratch_dir,
                    self.logger.name + "." + os.path.basename(skydb),
                    os.path.basename(executable)
                ) as logger:
                    catch_segfaults(cmd, scratch_dir, None, logger)
            except CalledProcessError, e:
                # For CalledProcessError isn't properly propagated by IPython
                # Temporary workaround...
                self.logger.error(str(e))
                return 1
            finally:
                shutil.rmtree(scratch_dir)

        return 0

if __name__ == "__main__":
    #   If invoked directly, parse command line arguments for logger information
    #                        and pass the rest to the run() method defined above
    # --------------------------------------------------------------------------
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(sourcedb(jobid, jobhost, jobport).run_with_stored_arguments())
