#                                                         LOFAR IMAGING PIPELINE
#
#                                  Master recipe to export calibration solutions
#                                                             Marcel Loose, 2011
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------
from __future__ import with_statement
import os
import sys
import copy
import lofarpipe.support.lofaringredient as ingredient

from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.data_map import DataMap, validate_data_maps


class gainoutliercorrection(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Recipe to correct outliers in the gain solutions of an parmdb,
    using the program `parmexportcal`   
    The main purpose of this program is to strip off the time axis information
    from a instrument model (a.k.a ParmDB)
    -or-
    a minimal implementation of the edit_parmdb program. Search all gains for
    outliers and swap these for the median

    1. Validate input
    2. load mapfiles, validate if a target output location is provided
    3. Call node side of the recipe
    4. validate performance, return corrected files

    **Command line arguments**

    1. A mapfile describing the data to be processed.
    2. A mapfile with target location <mapfiles are validated if present>
    
    """
    inputs = {
        'executable': ingredient.StringField(
            '--executable',
            default="",
            help="Full path to the `parmexportcal` executable, not settings this"
            " results in edit_parmdb behaviour"
        ),
        'suffix': ingredient.StringField(
            '--suffix',
            help="Suffix of the table name of the instrument model",
            default=".instrument"
        ),
        'working_directory': ingredient.StringField(
            '-w', '--working-directory',
            help="Working directory used on output nodes. "
                 "Results will be written here."
        ),
        'mapfile': ingredient.StringField(
            '--mapfile',
            help="Full path of mapfile to produce; it will contain "
                 "a list of the generated instrument-model files"
        ),
        'sigma': ingredient.FloatField(
            '--sigma',
            default=1.0,
            help="Clip at sigma * median: (not used by parmexportcal"
        ),
        'export_instrument_model': ingredient.FloatField(
            '--use-parmexportcal',
            default=False,
            help="Select between parmexportcal and edit parmdb"
        )
    }

    outputs = {
        'mapfile': ingredient.FileField(help="mapfile with corrected parmdbs")
    }


    def go(self):
        super(gainoutliercorrection, self).go()
        self.logger.info("Starting gainoutliercorrection run")
        # ********************************************************************
        # 1. Validate input
        # if sigma is none use default behaviour and use executable: test if
        # It excists
        executable = self.inputs['executable']
        if executable == "":
            pass
        elif not os.access(executable, os.X_OK):
            self.logger.warn(
                "No parmexportcal excecutable is not found on the suplied"
                "path: {0}".format(self.inputs['executable']))
            self.logger.warn("Defaulting to edit_parmdb behaviour")

        # ********************************************************************
        # 2. load mapfiles, validate if a target output location is provided
        args = self.inputs['args']
        self.logger.debug("Loading input-data mapfile: %s" % args[0])
        indata = DataMap.load(args[0])
        if len(args) > 1:
            self.logger.debug("Loading output-data mapfile: %s" % args[1])
            outdata = DataMap.load(args[1])
            if not validate_data_maps(indata, outdata):
                self.logger.error(
                    "Validation of input/output data mapfiles failed"
                )
                return 1
        else:
            outdata = copy.deepcopy(indata)
            for item in outdata:
                item.file = os.path.join(
                    self.inputs['working_directory'],
                    self.inputs['job_name'],
                    (os.path.splitext(os.path.basename(item.file))[0] +
                     self.inputs['suffix'])
                )

        # Update the skip fields of the two maps. If 'skip' is True in any of
        # these maps, then 'skip' must be set to True in all maps.
        for x, y in zip(indata, outdata):
            x.skip = y.skip = (x.skip or y.skip)

        # ********************************************************************
        # 3. Call node side of the recipe
        command = "python %s" % (self.__file__.replace('master', 'nodes'))
        indata.iterator = outdata.iterator = DataMap.SkipIterator
        jobs = []
        for inp, outp in zip(indata, outdata):
            jobs.append(
                ComputeJob(
                    outp.host,
                    command,
                    arguments=[
                        inp.file,
                        outp.file,
                        self.inputs['executable'],
                        self.environment,
                        self.inputs['sigma'],
                        self.inputs['export_instrument_model']
                     ]
                )
            )
        self._schedule_jobs(jobs)
        for job, outp in zip(jobs, outdata):
            if job.results['returncode'] != 0:
                outp.skip = True

        # ********************************************************************
        # 4. validate performance, return corrected files
        if self.error.isSet():
            self.logger.warn("Detected failed gainoutliercorrection job")
            return 1
        else:
            self.logger.debug("Writing instrument map file: %s" %
                              self.inputs['mapfile'])
            outdata.save(self.inputs['mapfile'])
            self.outputs['mapfile'] = self.inputs['mapfile']
            return 0


if __name__ == '__main__':
    sys.exit(gainoutliercorrection().main())
