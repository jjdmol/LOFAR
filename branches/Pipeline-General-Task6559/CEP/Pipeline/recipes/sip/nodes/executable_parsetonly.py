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
from lofarpipe.support.parset import Parset


class executable_parsetonly(LOFARnodeTCP):
    """
    Call ndppp with a parset augmented with locally calculate parameters:


    """

    def run(self, infile, outfile,
            parsetfile, executable, environment):
        """
        This function contains all the needed functionality
        """
        # Debugging info
        self.logger.debug("infile          = %s" % infile)
        self.logger.debug("outfile         = %s" % outfile)
        self.logger.debug("parsetfile      = %s" % parsetfile)
        self.logger.debug("executable      = %s" % executable)
        self.logger.debug("environment     = %s" % environment)

        self.environment.update(environment)

        # ********************************************************************
        # 1. preparations. set nthreads, Validate input, clean workspace
        #
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

            # override executable from task.cfg
            # temporary solution
            try:
                parset = Parset()
                parset.adoptFile(parsetfile)
                #if parset['exec']:
                #    executable = str(parset['exec'])
                print 'Running executable %s from parset args' % executable
            except:
                print 'could not load executable from parset args'
            # Check if executable is present
            if not os.access(executable, os.X_OK):
                self.logger.error("Executable %s not found" % executable)
                return 1

            # Make sure that we start with a clean slate
            shutil.rmtree(tmpfile, ignore_errors=True)

            # *****************************************************************
            # 2. Perform house keeping, test if work is already done
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

            # *****************************************************************
            # 3. Update the parset with locally calculate information

            # Put arguments we need to pass to some private methods in a dict
            kwargs = {
                'infile': infile,
                'tmpfile': tmpfile,
                'parsetfile': parsetfile
            }

            # Prepare for the actual DPPP run.
            with patched_parset(
            # *****************************************************************
            # 4. Add ms names to the parset, start/end times if availabe, etc.
                parsetfile, self._prepare_steps(**kwargs)
            ) as temp_parset_filename:
                self.logger.debug("Created temporary parset file: %s" %
                    temp_parset_filename
                )
                try:
                    working_dir = tempfile.mkdtemp()
            # ****************************************************************
            # 5. Run
                    cmd = [executable, temp_parset_filename]# + ' '+os.environ.get('HOME')]# +' > ' + tmpfile]
                    with CatchLog4CPlus(
                        working_dir,
                        self.logger.name + "." + os.path.basename(infile),
                        os.path.basename(executable),
                    ) as logger:
                        # Catch segfaults and retry

                        catch_segfaults(
                            cmd, working_dir, self.environment, logger,
                            cleanup=lambda : shutil.rmtree(tmpfile, ignore_errors=True)
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
        # Create output directory for output MS.
        create_directory(os.path.dirname(kwargs['tmpfile']))

        patch_dictionary = {
            'msin': kwargs['infile'],
            'msout': kwargs['tmpfile'],
            'uselogger': 'True'
        }

        # Return the patch dictionary that must be applied to the parset.
        return patch_dictionary


if __name__ == "__main__":
    #   If invoked directly, parse command line arguments for logger information
    #                        and pass the rest to the run() method defined above
    # --------------------------------------------------------------------------
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(executable_parsetonly(jobid, jobhost, jobport).run_with_stored_arguments())
