#                                                         LOFAR IMAGING PIPELINE
#
#                                                     Calibrator Pipeline recipe
#                                                             Marcel Loose, 2011
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

import os
import sys

from lofarpipe.support.control import control
from lofar.parameterset import parameterset
from lofarpipe.support.parset import Parset

class calibrator_pipeline(control):
    """
    The calibrator pipeline can be used to determine the instrument database
    (parmdb) from the observation of a known "calibrator" source.

    This pipeline will perform the following operations:
    - Create a empty parmdb for BBS
    - Run makesourcedb on skymodel files for calibrator source(s) and the
      Ateam, which are to be stored in a standard place ($LOFARROOT/share)
    - DPPP: flagging, using standard parset
    - Demix the relevant A-team sources (for now using python script, later
      to use DPPP), using the A-team sourcedb.
    - Run BBS to calibrate the calibrator source(s), again using standard
      parset, and the sourcedb made earlier
    """

    def __init__(self):
        control.__init__(self)
        self.parset = parameterset()


    def usage(self):
        print >> sys.stderr, "Usage: %s [options] <parset-file>" % sys.argv[0]
        return 1


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
        super(calibrator_pipeline, self).go()


    def pipeline_logic(self):
        """
        Define the individual tasks that comprise the current pipeline.
        This method will be invoked by the base-class's `go()` method.
        """

        # Create a parameter-subset containing only python-control stuff.
        py_parset = self.parset.makeSubset(
            'ObsSW.Observation.ObservationControl.PythonControl.')

        # Generate a datamap-file, which is a parset-file containing
        # key/value pairs of hostname and list of MS-files.
        data_mapfile = self.run_task(
            "cep2_datamapper",
            observation_dir=py_parset.getString('observationDirectory')
        )['mapfile']

        # Create an empty parmdb for DPPP
        parmdb_mapfile = self.run_task("parmdb", data_mapfile)['mapfile']

        # Create an empty sourcedb for DPPP
        sourcedb_mapfile = self.run_task("sourcedb", data_mapfile)['mapfile']

        # Produce a GVDS file describing the data on the compute nodes.
        gvds_file = self.run_task("vdsmaker", data_mapfile)['gvds']

        # Read metadata (start, end times, pointing direction) from GVDS.
        vdsinfo = self.run_task("vdsreader", gvds=gvds_file)

        # Create a parameter-subset for DPPP and write it to file.
        ndppp_parset = os.path.join(
            self.config.get("layout", "job_directory"),
            "parsets", "NDPPP.parset")
        py_parset.makeSubset('DPPP.').writeFile(ndppp_parset)

        # Run the Default Pre-Processing Pipeline (DPPP);
        data_mapfile = self.run_task(
            "ndppp", data_mapfile,
            data_start_time=vdsinfo['start_time'],
            data_end_time=vdsinfo['end_time'],
            parset=ndppp_parset
        )['mapfile']

        #data_map = parameterset(data_mapfile)
        #parmdb_map = parameterset(parmdb_mapfile)
        #sourcedb_map = parameterset(sourcedb_mapfile)

        #bbs_map = {}
        #for key in data_map.keys():
            #bbs_map[key] = zip(
                #data_map.getStringVector(key),
                #parmdb_map.getStringVector(key),
                #sourcedb_map.getStringVector(key)
            #)

        ## Demix the relevant A-team sources
        #self.run_task("demixing", mapfile)

        #return 0

        # Create a parameter-subset for BBS and write it to file.
        bbs_parset = os.path.join(
            self.config.get("layout", "job_directory"),
            "parsets", "BBS.parset")
        py_parset.makeSubset('BBS.').writeFile(bbs_parset)

        # Run BBS to calibrate the calibrator source(s).
        self.run_task(
            "new_bbs", data_mapfile,
            parset=bbs_parset,
            gvds_file=gvds_file,
            instrument_mapfile=parmdb_mapfile,
            sky_mapfile=sourcedb_mapfile)

if __name__ == '__main__':
    sys.exit(calibrator_pipeline().main())
