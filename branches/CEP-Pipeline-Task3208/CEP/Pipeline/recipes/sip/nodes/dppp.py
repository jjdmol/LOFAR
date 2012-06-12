#                                                         LOFAR IMAGING PIPELINE
#
#                                        DPPP (Data Pre-Procesing Pipeline) node
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

from __future__ import with_statement
from subprocess import CalledProcessError
import sys
import os.path
import tempfile
import shutil

from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.pipelinelogging import log_time
from lofarpipe.support.parset import patched_parset
from lofarpipe.support.utilities import read_initscript
from lofarpipe.support.utilities import create_directory
from lofarpipe.support.utilities import catch_segfaults
from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.lofarexceptions import ExecutableMissing
from lofar.parameterset import parameterset

class dppp(LOFARnodeTCP):
    def run(
        self, infile, outfile, parmdb, sourcedb,
        parsetfile, executable, initscript, demix_sources,
        start_time, end_time, nthreads, clobber
    ):
        # Debugging info
        self.logger.debug("infile        = %s" % infile)
        self.logger.debug("outfile       = %s" % outfile)
        self.logger.debug("parmdb        = %s" % parmdb)
        self.logger.debug("sourcedb      = %s" % sourcedb)
        self.logger.debug("parsetfile    = %s" % parsetfile)
        self.logger.debug("executable    = %s" % executable)
        self.logger.debug("initscript    = %s" % initscript)
        self.logger.debug("demix_sources = %s" % demix_sources)
        self.logger.debug("start_time    = %s" % start_time)
        self.logger.debug("end_time      = %s" % end_time)
        self.logger.debug("nthreads      = %s" % nthreads)
        self.logger.debug("clobber       = %s" % clobber)
        
        # Time execution of this job
        with log_time(self.logger):
            if os.path.exists(infile):
                self.logger.info("Processing %s" % (infile))
            else:
                self.logger.error("Dataset %s does not exist" % (infile))
                return 1

            if clobber:
                self.logger.info("Removing previous output %s" % outfile)
                shutil.rmtree(outfile, ignore_errors=True)

            self.logger.debug("Time interval: %s %s" % (start_time, end_time))

            #                                             Initialise environment
            #                 Limit number of threads used, per request from GvD
            # ------------------------------------------------------------------
            env = read_initscript(self.logger, initscript)
            if not nthreads: 
                nthreads = 1
            self.logger.debug("Using %s threads for NDPPP" % nthreads)
            env['OMP_NUM_THREADS'] = str(nthreads)

            #    Create output directory for output MS, if it doesn't exist yet.
            # ------------------------------------------------------------------
            if outfile != infile:
                create_directory(os.path.dirname(outfile))

            #       Patch the parset with the correct input/output MS names and,
            #                                   if available, start & end times.
            #                            The uselogger option enables log4cplus.
            # ------------------------------------------------------------------
            patch_dictionary = {
                'msin': infile,
                'msout': outfile,
                'uselogger': 'True'
            }
            if start_time:
                patch_dictionary['msin.starttime'] = start_time
            if end_time:
                patch_dictionary['msin.endtime'] = end_time
            # If we need to do a demixing step, we need to set some extra keys.
            # We have to read the parsetfile to check this.
            parset = parameterset(parsetfile)
            for step in parset.getStringVector('steps'):
                if parset.getString(step + '.type').startswith('demix'):
                    if parmdb:
                        patch_dictionary[step + '.instrumentmodel'] = parmdb
                    if sourcedb:
                        patch_dictionary[step + '.skymodel'] = sourcedb
                        
            with patched_parset(parsetfile, patch_dictionary, unlink=False) as temp_parset_filename:
                self.logger.debug(
                    "Created temporary parset file: %s" % temp_parset_filename
                )
#                raise Exception("Intentionally aborted")
                try:
                    if not os.access(executable, os.X_OK):
                        raise ExecutableMissing(executable)

                    working_dir = tempfile.mkdtemp()
                    cmd = [executable, temp_parset_filename, '1']

                    with CatchLog4CPlus(
                        working_dir,
                        self.logger.name + "." + os.path.basename(infile),
                        os.path.basename(executable),
                    ) as logger:
                        #     Catch NDPPP segfaults (a regular occurance), and retry
                        # ----------------------------------------------------------
                        if outfile != infile:
                            cleanup_fn = lambda : shutil.rmtree(outfile, ignore_errors=True)
                        else:
                            cleanup_fn = lambda : None
                        catch_segfaults(
                            cmd, working_dir, env, logger, cleanup=cleanup_fn
                        )
                except ExecutableMissing, e:
                    self.logger.error("%s not found" % (e.args[0]))
                    return 1
                except CalledProcessError, e:
                    #        CalledProcessError isn't properly propagated by IPython
                    # --------------------------------------------------------------
                    self.logger.error(str(e))
                    return 1
                except Exception, e:
                    self.logger.error(str(e))
                    return 1
                finally:
#                    os.unlink(temp_parset_filename)
                    shutil.rmtree(working_dir)

            return 0

if __name__ == "__main__":
    #   If invoked directly, parse command line arguments for logger information
    #                        and pass the rest to the run() method defined above
    # --------------------------------------------------------------------------
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(dppp(jobid, jobhost, jobport).run_with_stored_arguments())
