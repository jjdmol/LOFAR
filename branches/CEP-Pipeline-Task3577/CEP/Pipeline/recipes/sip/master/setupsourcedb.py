#                                                         LOFAR IMAGING PIPELINE
#
#                                                    setupsourcedb master recipe
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
#                                                             Marcel Loose, 2012
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

from __future__ import with_statement
import os
import sys

import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.group_data import load_data_map, store_data_map
from lofarpipe.support.group_data import validate_data_maps
from lofarpipe.support.remotecommand import ComputeJob

class setupsourcedb(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Create a distributed Sky Model database (SourceDB) for a distributed
    Measurement Set (MS).

    1. Load input and output mapfiles. Validate 
    2. Check if input skymodel file exists. If not, make filename empty.
    3. Call node side of recipe
    4. Validate performance and create output

    **Command line arguments**

    1. A mapfile describing the input data to be processed. 
    2. A mapfile with target location <if provided it will be validated against
       The input data>
    """
    inputs = {
        'executable': ingredient.ExecField(
            '--executable',
            help="Full path to makesourcedb executable",
        ),
        'skymodel': ingredient.StringField(
            '-s', '--skymodel',
            help="Input sky catalogue",
            default='None'
        ),
        'type': ingredient.StringField(
            '--type',
            help="Output type (casa or blob)",
            default="casa"
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
        'mapfile': ingredient.FileField(help="mapfile with created sourcedb"
         "paths")
    }


    def go(self):
        self.logger.info("Starting setupsourcedb run")
        super(setupsourcedb, self).go()

        # *********************************************************************
        # 1. Load input and output mapfiles. Validate

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

        # *********************************************************************
        # 2. Check if input skymodel file exists. If not, make filename empty.
        if not os.path.isfile(self.inputs['skymodel']):
            self.logger.warn(
                "Source catalog %s does not exist. Using an empty one." %
                self.inputs['skymodel']
            )
            self.inputs['skymodel'] = ""

        # ********************************************************************
        # 3. Call node side of script
        command = "python %s" % (self.__file__.replace('master', 'nodes'))
        jobs = []
        for host, outfile in outdata:
            jobs.append(
                ComputeJob(
                    host,
                    command,
                    arguments=[
                        self.inputs['executable'],
                        self.inputs['skymodel'],
                        outfile,
                        self.inputs['type']
                    ]
                )
            )
        self._schedule_jobs(jobs, max_per_node=self.inputs['nproc'])

        # *********************************************************************
        # 4. check performance and create output data
        if self.error.isSet():
            return 1
        else:
            self.logger.debug("Writing sky map file: %s" %
                              self.inputs['mapfile'])
            store_data_map(self.inputs['mapfile'], outdata)
            self.outputs['mapfile'] = self.inputs['mapfile']
            return 0


if __name__ == '__main__':
    sys.exit(setupsourcedb().main())
