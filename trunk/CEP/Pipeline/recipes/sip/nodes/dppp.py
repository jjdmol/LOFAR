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
from lofarpipe.support.utilities import create_directory
from lofarpipe.support.utilities import catch_segfaults
from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofar.parameterset import parameterset

class dppp(LOFARnodeTCP):

    def run(
        self, infile, outfile, parmdb, sourcedb,
        parsetfile, executable, environment, demix_always, demix_if_needed,
        start_time, end_time, nthreads, clobber
    ):
        # Debugging info
        self.logger.debug("infile          = %s" % infile)
        self.logger.debug("outfile         = %s" % outfile)
        self.logger.debug("parmdb          = %s" % parmdb)
        self.logger.debug("sourcedb        = %s" % sourcedb)
        self.logger.debug("parsetfile      = %s" % parsetfile)
        self.logger.debug("executable      = %s" % executable)
        self.logger.debug("environment     = %s" % environment)
        self.logger.debug("demix_always    = %s" % demix_always)
        self.logger.debug("demix_if_needed = %s" % demix_if_needed)
        self.logger.debug("start_time      = %s" % start_time)
        self.logger.debug("end_time        = %s" % end_time)
        self.logger.debug("nthreads        = %s" % nthreads)
        self.logger.debug("clobber         = %s" % clobber)

        self.environment.update(environment)
        
        if not nthreads:
            nthreads = 1
        if not outfile:
            outfile = infile
        tmpfile = outfile + '.tmp'

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

            # Make sure that we start with a clean slate
            shutil.rmtree(tmpfile, ignore_errors=True)
            if clobber:
                if outfile == infile:
                    self.logger.warn(
                        "Input and output are identical, not clobbering %s" %
                        outfile
                    )
                else:        
                    self.logger.info("Removing previous output %s" % outfile)
                    shutil.rmtree(outfile, ignore_errors=True)

            # If input and output files are different, and if output file
            # already exists, then we're done.
            if outfile != infile and os.path.exists(outfile):
                self.logger.info(
                    "Output file %s already exists. We're done." % outfile
                )
                self.outputs['ok'] = True
                return 0

            # Create a working copy if input and output are identical, to
            # avoid corrupting the original file if things go awry.
            if outfile == infile:
                self.logger.info(
                    "Creating working copy: %s --> %s" % (infile, tmpfile)
                )
                shutil.copytree(infile, tmpfile)

            # Limit number of threads used.
            self.environment['OMP_NUM_THREADS'] = str(nthreads)
            self.logger.debug("Using %s threads for NDPPP" % nthreads)

            # Put arguments we need to pass to some private methods in a dict
            kwargs = {
                'infile' : infile,
                'tmpfile' : tmpfile,
                'parmdb' : parmdb,
                'sourcedb' : sourcedb,
                'parsetfile' : parsetfile,
                'demix_always' : demix_always,
                'demix_if_needed' : demix_if_needed,
                'start_time' : start_time,
                'end_time' : end_time
            }

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
                        catch_segfaults(
                            cmd, working_dir, self.environment, logger, 
                            cleanup = lambda : shutil.rmtree(tmpfile, ignore_errors=True)
                        )
                        # Replace outfile with the updated working copy
                        shutil.rmtree(outfile, ignore_errors=True)
                        os.rename(tmpfile, outfile)
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
        create_directory(os.path.dirname(kwargs['tmpfile']))

        patch_dictionary = {
            'msin': kwargs['infile'],
            'msout': kwargs['tmpfile'],
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
        if kwargs['demix_if_needed']:
            ateam_list = list(
                set(kwargs['demix_if_needed']).intersection(ateam_list)
            )

        # Determine the complete set of sources to be demixed.
        demix_sources = list(set(kwargs['demix_always']).union(ateam_list))
        self.logger.info("Removing %d target(s) from %s: %s" % (
                len(demix_sources), kwargs['infile'], ', '.join(demix_sources)
            )
        )
        patch_dictionary[stepname + '.subtractsources'] = demix_sources
        
        # Return the patch dictionary.
        return patch_dictionary


if __name__ == "__main__":
    #   If invoked directly, parse command line arguments for logger information
    #                        and pass the rest to the run() method defined above
    # --------------------------------------------------------------------------
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(dppp(jobid, jobhost, jobport).run_with_stored_arguments())
