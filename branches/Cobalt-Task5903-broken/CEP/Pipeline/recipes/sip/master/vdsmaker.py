#                                                         LOFAR IMAGING PIPELINE
#
#                                     New vdsmaker recipe: fixed node allocation
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

from __future__ import with_statement
import sys
import os
import subprocess

import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.utilities import create_directory
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.data_map import DataMap
from lofarpipe.support.pipelinelogging import log_process_output

class vdsmaker(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Generate a GVDS file (and, optionally, individual VDS files per subband;
    see the ``unlink`` input parameter) describing a collection of
    MeasurementSets.

    1. Load data from disk, create the output vds paths
    2. Call the vdsmaker node script to generate the vds files
    3. Combine the vds files in a gvds file (master side operation)
    
    **Command line arguments**

    A mapfile describing the measurementsets to be processed.
    """
    inputs = {
        'gvds': ingredient.StringField(
            '-g', '--gvds',
            help="File name for output GVDS file"
        ),
        'directory': ingredient.DirectoryField(
            '--directory',
            help="Directory for output GVDS file"
        ),
        'makevds': ingredient.ExecField(
            '--makevds',
            help="Full path to makevds executable"
        ),
        'combinevds': ingredient.ExecField(
            '--combinevds',
            help="Full path to combinevds executable"
        ),
        'unlink': ingredient.BoolField(
            '--unlink',
            help="Unlink VDS files after combining",
            default=True
        ),
        'nproc': ingredient.IntField(
            '--nproc',
            help="Maximum number of simultaneous processes per compute node",
            default=8
        )
    }

    outputs = {
        'gvds': ingredient.FileField()
    }

    def go(self):
        """
        Contains functionality of the vdsmaker
        """
        super(vdsmaker, self).go()
        # **********************************************************************
        # 1. Load data from disk create output files
        args = self.inputs['args']
        self.logger.debug("Loading input-data mapfile: %s" % args[0])
        data = DataMap.load(args[0])

        # Skip items in `data` that have 'skip' set to True
        data.iterator = DataMap.SkipIterator

        # Create output vds names
        vdsnames = [
            os.path.join(
                self.inputs['directory'], os.path.basename(item.file) + '.vds'
            ) for item in data
        ]

        # *********************************************************************
        # 2. Call vdsmaker 
        command = "python %s" % (self.__file__.replace('master', 'nodes'))
        jobs = []
        for inp, vdsfile in zip(data, vdsnames):
            jobs.append(
                ComputeJob(
                    inp.host, command,
                    arguments=[
                        inp.file,
                        self.config.get('cluster', 'clusterdesc'),
                        vdsfile,
                        self.inputs['makevds']
                    ]
                )
            )
        self._schedule_jobs(jobs, max_per_node=self.inputs['nproc'])
        vdsnames = [
            vds for vds, job in zip(vdsnames, jobs) 
            if job.results['returncode'] == 0
        ]
        if not vdsnames:
            self.logger.error("All makevds processes failed. Bailing out!")
            return 1

        # *********************************************************************
        # 3. Combine VDS files to produce GDS
        failure = False
        self.logger.info("Combining VDS files")
        executable = self.inputs['combinevds']
        gvds_out = self.inputs['gvds']
        # Create the gvds directory for output files, needed for combine
        create_directory(os.path.dirname(gvds_out))

        try:
            command = [executable, gvds_out] + vdsnames
            combineproc = subprocess.Popen(
                command,
                close_fds=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
            sout, serr = combineproc.communicate()
            log_process_output(executable, sout, serr, self.logger)
            if combineproc.returncode != 0:
                raise subprocess.CalledProcessError(
                    combineproc.returncode, command
                )
            self.outputs['gvds'] = gvds_out
            self.logger.info("Wrote combined VDS file: %s" % gvds_out)
        except subprocess.CalledProcessError, cpe:
            self.logger.exception(
                "combinevds failed with status %d: %s" % (cpe.returncode, serr)
            )
            failure = True
        except OSError, err:
            self.logger.error("Failed to spawn combinevds (%s)" % str(err))
            failure = True
        finally:
            if self.inputs["unlink"]:
                self.logger.debug("Unlinking temporary files")
                for name in vdsnames:
                    os.unlink(name)
            self.logger.info("vdsmaker done")

        if failure:
            self.logger.info("Error was set, exit vds maker with error state")
            return 1
        elif not self.outputs.complete():
            self.logger.info("Outputs incomplete")
        else:
            return 0


if __name__ == '__main__':
    sys.exit(vdsmaker().main())
