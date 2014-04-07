#                                                         LOFAR IMAGING PIPELINE
#
#                                        DPPP (Data Pre-Procesing Pipeline) node
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

from __future__ import with_statement
from subprocess import CalledProcessError
import logging
import os
import tempfile
import shutil
import sys

from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.pipelinelogging import log_time
from lofarpipe.support.parset import patched_parset
from lofarpipe.support.utilities import read_initscript
from lofarpipe.support.utilities import create_directory
from lofarpipe.support.utilities import catch_segfaults
from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofar.parameterset import parameterset

class dppp(LOFARnodeTCP):

    def run(
        self, infile, outfile, parmdb, sourcedb,
        parsetfile, executable, initscript, demix_sources,
        start_time, end_time, nthreads, clobber
    ):
        # Put arguments that we need to pass to some private methods in a dict
        kwargs = {
            'infile' : infile,
            'outfile' : outfile,
            'parmdb' : parmdb,
            'sourcedb' : sourcedb,
            'parsetfile' : parsetfile,
            'demix_sources' : demix_sources,
            'start_time' : start_time,
            'end_time' : end_time
        }

        if not nthreads:
            nthreads = 1

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

            # Check if DPPP executable is present
            if not os.access(executable, os.X_OK):
                self.logger.error("Executable %s not found" % executable)
                return 1

            if clobber:
                self.logger.info("Removing previous output %s" % outfile)
                shutil.rmtree(outfile, ignore_errors=True)

            # Initialise environment. Limit number of threads used.
            env = read_initscript(self.logger, initscript)
            env['OMP_NUM_THREADS'] = str(nthreads)
            self.logger.debug("Using %s threads for NDPPP" % nthreads)

            # Prepare for the actual DPPP run.
            with patched_parset(
                parsetfile, self._prepare_steps(**kwargs) #, unlink=False
            ) as temp_parset_filename:

                self.logger.debug("Created temporary parset file: %s" % 
                    temp_parset_filename
                )
                try:
                    working_dir = tempfile.mkdtemp()
                    cmd = [executable, temp_parset_filename, '1']

                    with CatchLog4CPlus(
                        working_dir,
                        self.logger.name + "." + os.path.basename(infile),
                        os.path.basename(executable),
                    ) as logger:
                        # Catch NDPPP segfaults (a regular occurance), and retry
                        if outfile != infile:
                            cleanup_fn = lambda : shutil.rmtree(outfile, ignore_errors=True)
                        else:
                            cleanup_fn = lambda : None
                        catch_segfaults(
                            cmd, working_dir, env, logger, cleanup=cleanup_fn
                        )
                except CalledProcessError, err:
                    # CalledProcessError isn't properly propagated by IPython
                    self.logger.error(str(err))
                    return 1
                except Exception, err:
                    self.logger.error(str(err))
                    return 1
                finally:
                    shutil.rmtree(working_dir)

        # We need some signal to the master script that the script ran ok.
        self.outputs['ok'] = True
        return 0


    def _prepare_steps(self, **kwargs):
        """
        Prepare for running the NDPPP program. This means, for one thing,
        patching the parsetfile with the correct input/output MS names,
        start/end times if availabe, etc. If a demixing step must be performed,
        some extra work needs to be done.
        
        Returns: patch dictionary that must be applied to the parset.
        """
        self.logger.debug(
            "Time interval: %s %s" % (kwargs['start_time'], kwargs['end_time'])
        )
        # Create output directory for output MS.
        create_directory(os.path.dirname(kwargs['outfile']))

        patch_dictionary = {
            'msin': kwargs['infile'],
            'msout': kwargs['outfile'],
            'uselogger': 'True'
        }
        if kwargs['start_time']:
            patch_dictionary['msin.starttime'] = kwargs['start_time']
        if kwargs['end_time']:
            patch_dictionary['msin.endtime'] = kwargs['end_time']
            
        # If we need to do a demixing step, we have to do some extra work.
        # We have to read the parsetfile to check this.
        parset = parameterset(kwargs['parsetfile'])
        for step in parset.getStringVector('steps'):
            if parset.getString(step + '.type', '').startswith('demix'):
                patch_dictionary.update(
                    self._prepare_demix_step(step, **kwargs)
                )
                
        # Return the patch dictionary that must be applied to the parset.
        return patch_dictionary
        
        
    def _prepare_demix_step(self, stepname, **kwargs):
        """
        Prepare for a demixing step. This requires the setting of some
        extra keys in the parset, as well as testing which A-team sources
        must actually be demixed.
        Parameters: 
            `stepname`: name of the demixing step in the parset.
            `kwargs`  : dict of extra arguments.
        Returns: patch_dictionary that must be applied to the parset.
        """
        # Add demix directory to sys.path before importing find_a_team module.
        sys.path.insert(0, os.path.join(os.path.dirname(sys.argv[0]), "demix"))
        from find_a_team import getAteamList
        
        patch_dictionary = {}
        if kwargs['parmdb']:
            patch_dictionary[stepname + '.instrumentmodel'] = kwargs['parmdb']
        if kwargs['sourcedb']:
            patch_dictionary[stepname + '.skymodel'] = kwargs['sourcedb']
        
        # Use heuristics to get a list of A-team sources that may need
        # to be removed. 
        ateam_list = getAteamList(
            kwargs['infile'],
            outerDistance=2.e4,
            elLimit=5.,
            verbose=self.logger.isEnabledFor(logging.DEBUG)
        )
        self.logger.debug("getAteamList returned: %s" % ateam_list)
        # If the user specified a list of candidate A-team sources to remove,
        # then determine the intersection of both lists.
        if kwargs['demix_sources']:
            ateam_list = list(
                set(kwargs['demix_sources']).intersection(ateam_list)
            )
        self.logger.info("Removing %d target(s) from %s: %s" % (
                len(ateam_list), kwargs['infile'], ', '.join(ateam_list)
            )
        )
        patch_dictionary[stepname + '.subtractsources'] = ateam_list
        
        # Return the patch dictionary.
        return patch_dictionary


if __name__ == "__main__":
    #   If invoked directly, parse command line arguments for logger information
    #                        and pass the rest to the run() method defined above
    # --------------------------------------------------------------------------
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(dppp(jobid, jobhost, jobport).run_with_stored_arguments())
