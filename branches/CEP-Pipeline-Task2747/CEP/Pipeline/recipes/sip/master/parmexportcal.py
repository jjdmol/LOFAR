#                                                         LOFAR IMAGING PIPELINE
#
#                                  Master recipe to export calibration solutions
#                                                             Marcel Loose, 2011
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

import sys

import lofarpipe.support.lofaringredient as ingredient

from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.group_data import load_data_map
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob


class ParmExportCal(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Recipe to export calibration solutions, using the program `parmexportcal`.
    The main purpose of this program is to strip off the time axis information
    from a instrument model (a.k.a ParmDB)
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
        'input_mapfile' : ingredient.FileField(
            '--input-mapfile',
            help="Full path to the mapfile containing the locations of the "
                 "input files to be read"
        ),
        'output_mapfile' : ingredient.FileField(
            '--output-mapfile',
            help="Full path to the mapfile containing the locations of the "
                 "output files to be written"
#            default=None
        )
    }


    def go(self):
        self.logger.info("Starting ParmExportCal run")
        super(ParmExportCal, self).go()

        #                            Load file <-> output node mapping from disk
        # ----------------------------------------------------------------------
        self.logger.debug("Loading input-mapfile: %s" %
                          self.inputs['input_mapfile'])
        in_list = load_data_map(self.inputs['input_mapfile'])
        self.logger.debug("Loading output-mapfile: %s" %
                          self.inputs['output_mapfile'])
        out_list = load_data_map(self.inputs['output_mapfile'])

        # Sanity checks on input- and output map files
        if not len(in_list) == len(out_list):
            self.logger.error("Number of input- and output files must be equal")
            return 1
        if not [x[0] for x in in_list] == [x[0] for x in out_list]:
            self.logger.error("Input- and output file must be on the same node")
            return 1

        # Gerenate a list of tuples (host, input, output)
        io_list = [(in_list[i][0], in_list[i][1], out_list[i][1])
                   for i in range(len(in_list))]

        command = "python %s" % (self.__file__.replace('master', 'nodes'))
        jobs = []
        for host, infile, outfile in io_list:
            jobs.append(
                ComputeJob(
                    host, command,
                    arguments=[
                        infile,
                        outfile,
                        self.inputs['executable'],
                        self.inputs['initscript']
                    ]
                )
            )
        self._schedule_jobs(jobs)#, max_per_node=self.inputs['nproc'])

        if self.error.isSet():
            return 1


if __name__ == '__main__':
    sys.exit(ParmExportCal().main())
