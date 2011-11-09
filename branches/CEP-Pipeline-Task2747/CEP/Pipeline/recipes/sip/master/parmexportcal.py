#                                                         LOFAR IMAGING PIPELINE
#
#                                  Master recipe to export calibration solutions
#                                                             Marcel Loose, 2011
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

import collections
import os
import sys

import lofarpipe.support.lofaringredient as ingredient

from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.group_data import load_data_map
from lofarpipe.support.parset import Parset
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob


class parmexportcal(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Recipe to export calibration solutions, using the program `parmexportcal`.
    The main purpose of this program is to strip off the time axis information
    from a instrument model (a.k.a ParmDB)

    **Arguments**

    A mapfile describing the data to be processed.
    """
    inputs = {
        'executable': ingredient.ExecField(
            '--executable',
            help="Full path to the `parmexportcal` executable"
        ),
        'initscript' : ingredient.FileField(
            '--initscript',
            help="The full path to an (Bourne) shell script which will "
                 "intialise the environment (i.e., ``lofarinit.sh``)"
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
        )
    }

    outputs = {
        'mapfile': ingredient.FileField()
    }


    def go(self):
        self.logger.info("Starting parmexportcal run")
        super(parmexportcal, self).go()

        #                            Load file <-> output node mapping from disk
        # ----------------------------------------------------------------------
        self.logger.debug("Loading map from %s" % self.inputs['args'][0])
        data = load_data_map(self.inputs['args'][0])

        command = "python %s" % (self.__file__.replace('master', 'nodes'))
        outnames = collections.defaultdict(list)
        jobs = []
        for host, infile in data:
            outnames[host].append(
                os.path.join(
                    self.inputs['working_directory'],
                    self.inputs['job_name'],
                    (os.path.splitext(os.path.basename(infile))[0] +
                     self.inputs['suffix'])
                )
            )
            jobs.append(
                ComputeJob(
                    host,
                    command,
                    arguments=[
                        infile,
                        outnames[host][-1],
                        self.inputs['executable'],
                        self.inputs['initscript']
                     ]
                )
            )
        self._schedule_jobs(jobs)

        if self.error.isSet():
            self.logger.warn("Detected failed parmexportcal job")
            return 1
        else:
            self.logger.debug("Writing instrument map file: %s" %
                              self.inputs['mapfile'])
            Parset.fromDict(outnames).writeFile(self.inputs['mapfile'])
            self.outputs['mapfile'] = self.inputs['mapfile']
            return 0


if __name__ == '__main__':
    sys.exit(parmexportcal().main())
