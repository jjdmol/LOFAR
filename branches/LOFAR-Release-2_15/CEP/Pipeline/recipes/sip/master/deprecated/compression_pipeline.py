#                                                         LOFAR IMAGING PIPELINE
#
#                                                    Compression Pipeline recipe
#                                                             Marcel Loose, 2011
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

from __future__ import with_statement
import os.path
import sys

from lofarpipe.support.control import control
from lofar.parameterset import parameterset

class compression_pipeline(control):
    """
    The compression pipeline comprises all the tasks that need to be run
    to do flagging and compression in time and/or frequency of the MS data.
    """

    def __init__(self):
        control.__init__(self)
        self.parset = parameterset()

    def usage(self):
        print >> sys.stderr, "Usage: %s [options] <parset-file>" % sys.argv[0]
        return 1

    def pipeline_logic(self):
        """
        Define the individual tasks that comprise the current pipeline.
        This method will be invoked by the base-class's `go()` method.
        """

        # Create a parameter-subset containing only python-control stuff.
        py_parset = self.parset.makeSubset(
            self.parset.fullModuleName('PythonControl') + '.'
        )

        # Generate a datamap-file, which is a parset-file containing
        # key/value pairs of hostname and list of MS-files.
        mapfile = self.run_task(
                "cep2_datamapper",
                observation_dir=py_parset.getString('observationDirectory')
            )['mapfile']

        # Produce a GVDS file describing the data on the compute nodes.
        self.run_task("vdsmaker", mapfile)

        # Read metadata (start, end times, pointing direction) from GVDS.
        vdsinfo = self.run_task("vdsreader")

        # Run the Default Pre-Processing Pipeline (DPPP);
        # create a NDPPP.parset file first, containing only DPPP keys.
        ndppp_parset = os.path.join(
            self.config.get("layout", "job_directory"),
                            "parsets", "NDPPP.parset")
        py_parset.makeSubset('DPPP.').writeFile(ndppp_parset)
        self.run_task("ndppp", mapfile,
                      data_start_time=vdsinfo['start_time'],
                      data_end_time=vdsinfo['end_time'],
                      parset=ndppp_parset
                     )

    def go(self):
        """
        Read the parset-file that was given as input argument, and set the
        jobname before calling the base-class's `go()` method.
        """
        try:
            parset_file = self.inputs['args'][0]
        except IndexError:
            return self.usage()
        self.parset.adoptFile(parset_file)
        # Set job-name to basename of parset-file w/o extension, if it's not
        # set on the command-line with '-j' or '--job-name'
        if not self.inputs.has_key('job_name'):
            self.inputs['job_name'] = (
                os.path.splitext(os.path.basename(parset_file))[0]
                )
        super(compression_pipeline, self).go()

if __name__ == '__main__':
    sys.exit(compression_pipeline().main())
