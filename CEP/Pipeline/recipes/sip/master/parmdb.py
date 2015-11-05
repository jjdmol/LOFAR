#                                                         LOFAR IMAGING PIPELINE
#
#                                                                  parmdb recipe
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

import os
import sys
import subprocess
import shutil
import tempfile

from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.group_data import load_data_map, store_data_map
from lofarpipe.support.group_data import validate_data_maps
from lofarpipe.support.pipelinelogging import log_process_output
import lofarpipe.support.lofaringredient as ingredient

template = """
create tablename="%s"
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

class parmdb(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Add a parameter database to input MeasurementSets.

    This recipe is called by the :class:`bbs.bbs` recipe; it may also be used
    standalone.

    **Arguments**

    A mapfile describing the data to be processed.
    """
    inputs = {
        'executable': ingredient.ExecField(
            '--executable',
            help="Full path to parmdbm executable",
        ),
        'nproc': ingredient.IntField(
            '--nproc',
            help="Maximum number of simultaneous processes per compute node",
            default=8
        ),
        'suffix': ingredient.StringField(
            '--suffix',
            help="Suffix of the table name of the empty parmameter database",
            default=".parmdb"
        ),
        'working_directory': ingredient.StringField(
            '-w', '--working-directory',
            help="Working directory used on output nodes. "
                 "Results will be written here."
        ),
        'mapfile': ingredient.StringField(
            '--mapfile',
            help="Full path of mapfile to produce; it will contain "
                 "a list of the generated empty parameter database files"
        )
    }

    outputs = {
        'mapfile': ingredient.FileField()
    }

    def go(self):
        self.logger.info("Starting parmdb run")
        super(parmdb, self).go()

        self.logger.info("Generating template parmdb")
        pdbdir = tempfile.mkdtemp(
            dir=self.config.get("layout", "job_directory")
        )
        pdbfile = os.path.join(pdbdir, self.inputs['suffix'])

        try:
            parmdbm_process = subprocess.Popen(
                [self.inputs['executable']],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
            sout, serr = parmdbm_process.communicate(template % pdbfile)
            log_process_output("parmdbm", sout, serr, self.logger)
        except OSError, err:
            self.logger.error("Failed to spawn parmdbm: %s" % str(err))
            return 1

        #                     try-finally block to always remove temporary files
        # ----------------------------------------------------------------------
        try:
            #                       Load file <-> compute node mapping from disk
            # ------------------------------------------------------------------
            args = self.inputs['args']
            self.logger.debug("Loading input-data mapfile: %s" % args[0])
            indata = load_data_map(args[0])
            if len(args) > 1:
                self.logger.debug("Loading output-data mapfile: %s" % args[1])
                outdata = load_data_map(args[1])
                if not validate_data_maps(indata, outdata):
                    self.logger.error(
                        "Validation of input/output data mapfiles failed"
                    )
                    return 1
            else:
                outdata = [
                    (host,
                     os.path.join(
                        self.inputs['working_directory'],
                        self.inputs['job_name'],
                        os.path.basename(infile) + self.inputs['suffix'])
                    ) for host, infile in indata
                ]
                
            command = "python %s" % (self.__file__.replace('master', 'nodes'))
            jobs = []
            for host, outfile in outdata:
                jobs.append(
                    ComputeJob(
                        host,
                        command,
                        arguments=[
                            pdbfile,
                            outfile
                        ]
                    )
                )
            self._schedule_jobs(jobs, max_per_node=self.inputs['nproc'])

        finally:
            self.logger.debug("Removing template parmdb")
            shutil.rmtree(pdbdir, ignore_errors=True)

        if self.error.isSet():
            self.logger.warn("Detected failed parmdb job")
            return 1
        else:
            self.logger.debug("Writing parmdb map file: %s" %
                              self.inputs['mapfile'])
            store_data_map(self.inputs['mapfile'], outdata)
            self.outputs['mapfile'] = self.inputs['mapfile']
            return 0


if __name__ == '__main__':
    sys.exit(parmdb().main())
