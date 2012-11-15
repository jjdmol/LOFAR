#                                                         LOFAR IMAGING PIPELINE
#
#                                                      setupparmdb master recipe
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
#                                                             Marcel Loose, 2012
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

import copy
import os
import sys
import subprocess
import shutil
import tempfile

from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.data_map import DataMap, validate_data_maps
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
quit
"""

class setupparmdb(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Create a distributed parameter database (ParmDB) for a distributed 
    Measurement set (MS).
    
    1. Create a parmdb template at the master side of the recipe
    2. Call node side of recipe with template and possible targets
    3. Validate performance, cleanup of temp files, construct output

    **Command line arguments**

    1. A mapfile describing the data to be processed.
    2. A mapfile with output location (If provide input and output are validated)
    
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
        self.logger.info("Starting setupparmdb run")
        super(setupparmdb, self).go()

        # *********************************************************************
        # 1. Create a temporary template parmdb at the master side of the recipe
        self.logger.info("Generating template parmdb")

        # generate a temp dir
        pdbdir = tempfile.mkdtemp(
            dir=self.config.get("layout", "job_directory")
        )
        pdbfile = os.path.join(pdbdir, self.inputs['suffix'])

        # Create a template use tempdir for location 
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

        # *********************************************************************
        # 2. Call node side of recipe with template and possible targets
        #    If output location are provided as input these are validated.
        try:
            #                       Load file <-> compute node mapping from disk
            # ------------------------------------------------------------------
            args = self.inputs['args']
            self.logger.debug("Loading input-data mapfile: %s" % args[0])
            indata = DataMap.load(args[0])
            if len(args) > 1:
                # If output location provide validate the input and outputmap
                self.logger.debug("Loading output-data mapfile: %s" % args[1])
                outdata = DataMap.load(args[1])
                if not validate_data_maps(indata, outdata):
                    self.logger.error(
                        "Validation of input/output data mapfiles failed"
                    )
                    return 1
                # else output location is inputlocation+suffix
            else:
                outdata = copy.deepcopy(indata)
                for item in outdata:
                    item.file = os.path.join(
                        self.inputs['working_directory'],
                        self.inputs['job_name'],
                        os.path.basename(item.file) + self.inputs['suffix']
                    )
            #  Call the node side   
            command = "python %s" % (self.__file__.replace('master', 'nodes'))
            outdata.iterator = DataMap.SkipIterator
            jobs = []
            for outp in outdata:
                jobs.append(
                    ComputeJob(
                        outp.host,
                        command,
                        arguments=[
                            pdbfile,
                            outp.file
                        ]
                    )
                )
            self._schedule_jobs(jobs, max_per_node=self.inputs['nproc'])
            for job, outp in zip(jobs, outdata):
                if job.results['returncode'] != 0:
                    outp.skip = True

        # *********************************************************************
        # 3. validate performance, cleanup of temp files, construct output
        finally:
            self.logger.debug("Removing template parmdb")
            shutil.rmtree(pdbdir, ignore_errors=True)

        if self.error.isSet():
             # Abort if all jobs failed
            if all(job.results['returncode'] != 0 for job in jobs):
                self.logger.error("All jobs failed. Bailing out!")
                return 1
            else:
                self.logger.warn(
                    "Some jobs failed, continuing with succeeded runs"
                )
        self.logger.debug(
            "Writing parmdb map file: %s" % self.inputs['mapfile']
        )
        outdata.save(self.inputs['mapfile'])
        self.outputs['mapfile'] = self.inputs['mapfile']
        return 0


if __name__ == '__main__':
    sys.exit(setupparmdb().main())
