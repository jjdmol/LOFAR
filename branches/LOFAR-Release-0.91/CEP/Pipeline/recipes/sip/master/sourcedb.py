#                                                         LOFAR IMAGING PIPELINE
#
#                                                                sourcedb recipe
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

from __future__ import with_statement
import os
import collections

import lofarpipe.support.utilities as utilities
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.clusterlogger import clusterlogger
from lofarpipe.support.group_data import load_data_map
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.parset import Parset

class sourcedb(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Add a source database to input MeasurementSets.

    This recipe is called by the :class:`bbs.bbs` recipe; it may also be used
    standalone.

    **Arguments**

    A mapfile describing the data to be processed.
    """
    inputs = {
        'executable': ingredient.ExecField(
            '--executable',
            help="Full path to makesourcedb executable",
        ),
        'skymodel': ingredient.FileField(
            '-s', '--skymodel',
            help="Input sky catalogue"
        ),
        'mapfile': ingredient.StringField(
            '--mapfile',
            help="Full path of mapfile to produce; it will contain "
                 "a list of the generated sky-model files"
        ),
        'nproc': ingredient.IntField(
            '--nproc',
            help="Maximum number of simultaneous processes per compute node",
            default=8
        ),
        'suffix': ingredient.StringField(
            '--suffix',
            help="Suffix of the table name of the sky model",
            default=".sky"
        ),
        'working_directory': ingredient.StringField(
            '-w', '--working-directory',
            help="Working directory used on output nodes. "
                 "Results will be written here."
        )
    }

    outputs = {
        'mapfile': ingredient.FileField()
    }

    def go(self):
        self.logger.info("Starting sourcedb run")
        super(sourcedb, self).go()

        #                           Load file <-> compute node mapping from disk
        # ----------------------------------------------------------------------
        self.logger.debug("Loading map from %s" % self.inputs['args'][0])
        data = load_data_map(self.inputs['args'][0])

        command = "python %s" % (self.__file__.replace('master', 'nodes'))
        outnames = collections.defaultdict(list)
        jobs = []
        for host, ms in data:
            outnames[host].append(
                os.path.join(
                    self.inputs['working_directory'],
                    self.inputs['job_name'],
                    os.path.basename(ms) + self.inputs['suffix']
                )
            )
            jobs.append(
                ComputeJob(
                    host,
                    command,
                    arguments=[
                        self.inputs['executable'],
                        self.inputs['skymodel'],
                        outnames[host][-1]
                    ]
                )
            )
        self._schedule_jobs(jobs, max_per_node=self.inputs['nproc'])

        if self.error.isSet():
            return 1
        else:
            self.logger.debug("Writing sky map file: %s" % 
                              self.inputs['mapfile'])
            Parset.fromDict(outnames).writeFile(self.inputs['mapfile'])
            self.outputs['mapfile'] = self.inputs['mapfile']
            return 0

if __name__ == '__main__':
    sys.exit(sourcedb().main())
