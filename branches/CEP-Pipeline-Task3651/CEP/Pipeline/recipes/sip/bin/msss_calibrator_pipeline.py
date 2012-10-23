#!/usr/bin/env python
#                                                     LOFAR CALIBRATION PIPELINE
#
#                                                     Calibrator Pipeline recipe
#                                                        Marcel Loose, 2011-2012
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

import os
import sys

from lofarpipe.support.control import control
from lofarpipe.support.lofarexceptions import PipelineException
from lofarpipe.support.data_map import DataMap
from lofarpipe.support.group_data import validate_data_maps
from lofarpipe.support.group_data import tally_data_map
from lofarpipe.support.utilities import create_directory
from lofar.parameterset import parameterset


class msss_calibrator_pipeline(control):
    """
    The calibrator pipeline can be used to determine the instrument database
    (parmdb) from the observation of a known "calibrator" source. It creates an
    instrument model of the current LOFAR instrument (As sum of instrumental
    properties and Ionospere disturbances TODOW). The output of this toplevel
    pipeline recipe is this instrument model. Which can be used in a later
    target pipeline calibrate target data. 

    **This pipeline will perform the following operations:**

    1. Preparations, Parse and validate input and set local variables
    2. Create database (files), A sourcedb with A-Team sources, a vds file
       describing the nodes, a parmdb for calibration solutions
    3. DPPP. flagging, using standard parset
       Demix the relevant A-team sources), using the A-team sourcedb.
    4. Run BBS to calibrate the calibrator source(s), again using standard
       parset, and the sourcedb made earlier
    5. Perform gain correction on the created instrument table
    6. Create output for consumption by the LOFAR framework

    **Per subband-group, the following output products will be delivered:**

    1. An parmdb with instrument calibration solution to be applied to a target
       measurement set  in the target pipeline

    """

    def __init__(self):
        control.__init__(self)
        self.parset = parameterset()
        self.input_data = []
        self.output_data = []
        self.io_data_mask = []
        self.parset_feedback_file = None


    def usage(self):
        """
        Display usage
        """
        print >> sys.stderr, "Usage: %s [options] <parset-file>" % sys.argv[0]
        return 1


    def _get_io_product_specs(self):
        """
        Get input- and output-data product specifications from the
        parset-file, and do some sanity checks.
        """
        dps = self.parset.makeSubset(
            self.parset.fullModuleName('DataProducts') + '.'
        )
        self.input_data = DataMap([
            tuple(os.path.join(location, filename).split(':'))
                for location, filename, skip in zip(
                    dps.getStringVector('Input_Correlated.locations'),
                    dps.getStringVector('Input_Correlated.filenames'),
                    dps.getBoolVector('Input_Correlated.skip'))
                if not skip
        ])
        self.logger.debug("%d Input_Correlated data products specified" %
                          len(self.input_data))
        self.output_data = DataMap([
            tuple(os.path.join(location, filename).split(':'))
                for location, filename, skip in zip(
                    dps.getStringVector('Output_InstrumentModel.locations'),
                    dps.getStringVector('Output_InstrumentModel.filenames'),
                    dps.getBoolVector('Output_InstrumentModel.skip'))
                if not skip
        ])
        self.logger.debug("%d Output_InstrumentModel data products specified" %
                          len(self.output_data))
        # Sanity checks on input- and output data product specifications
        if not validate_data_maps(self.input_data, self.output_data):
            raise PipelineException(
                "Validation of input/output data product specification failed!"
            )
        # Validate input data, by searching the cluster for files
        self._validate_input_data()
        # Update input- and output-data product specifications if needed
        if not all(self.io_data_mask):
            self.logger.info("Updating input/output product specifications")
            self.input_data = [
                f for (f, m) in zip(self.input_data, self.io_data_mask) if m
            ]
            self.output_data = [
                f for (f, m) in zip(self.output_data, self.io_data_mask) if m
            ]


    def _validate_input_data(self):
        """
        Search for the requested input files and mask the files in
        `self.input_data[]` that could not be found on the system.
        """
        # Use filename glob-pattern as defined in LOFAR-USG-ICD-005.
        self.io_data_mask = tally_data_map(
            self.input_data, 'L*_SB???_uv.MS', self.logger
        )
        # Log a warning if not all input data files were found.
        if not all(self.io_data_mask):
            self.logger.warn(
                "The following input data files were not found: %s" %
                ', '.join(
                    str(f) for (f, m) in zip(
                        self.input_data, self.io_data_mask
                    ) if not m
                )
            )


    def go(self):
        """
        Read the parset-file that was given as input argument;
        set jobname, and input/output data products before calling the
        base-class's `go()` method.
        """
        try:
            parset_file = os.path.abspath(self.inputs['args'][0])
        except IndexError:
            return self.usage()
        self.parset.adoptFile(parset_file)
        self.parset_feedback_file = parset_file + "_feedback"

        # Set job-name to basename of parset-file w/o extension, if it's not
        # set on the command-line with '-j' or '--job-name'
        if not self.inputs.has_key('job_name'):
            self.inputs['job_name'] = (
                os.path.splitext(os.path.basename(parset_file))[0])

        # Call the base-class's `go()` method.
        return super(msss_calibrator_pipeline, self).go()


    def pipeline_logic(self):
        """
        Define the individual tasks that comprise the current pipeline.
        This method will be invoked by the base-class's `go()` method.
        """
        # *********************************************************************
        # 1. Get input from parset, validate and cast to pipeline 'data types'
        #    Only perform work on existing files
        #    Created needed directories 
        # Create a parameter-subset containing only python-control stuff.
        py_parset = self.parset.makeSubset(
            self.parset.fullModuleName('PythonControl') + '.')

        # Get input/output-data products specifications.
        self._get_io_product_specs()

        job_dir = self.config.get("layout", "job_directory")
        parset_dir = os.path.join(job_dir, "parsets")
        mapfile_dir = os.path.join(job_dir, "mapfiles")

        # Create directories for temporary parset- and map files
        create_directory(parset_dir)
        create_directory(mapfile_dir)

        # Write input- and output data map-files
        data_mapfile = os.path.join(mapfile_dir, "data.mapfile")
        self._store_data_map(data_mapfile, self.input_data, "inputs")
        instrument_mapfile = os.path.join(mapfile_dir, "instrument.mapfile")
        self._store_data_map(instrument_mapfile, self.output_data, "output")

        if len(self.input_data) == 0:
            self.logger.warn("No input data files to process. Bailing out!")
            return 0

        self.logger.debug("Processing: %s" %
            ', '.join(str(f) for f in self.input_data))
        # *********************************************************************
        # 2. Create database needed for performing work: 
        #    Vds, descibing data on the nodes
        #    sourcedb, For skymodel (A-team)
        #    parmdb for outputtting solutions
        # Produce a GVDS file describing the data on the compute nodes.
        gvds_file = self.run_task("vdsmaker", data_mapfile)['gvds']

        # Read metadata (start, end times, pointing direction) from GVDS.
        vdsinfo = self.run_task("vdsreader", gvds=gvds_file)

        # Create an empty parmdb for DPPP
        parmdb_mapfile = self.run_task(
            "setupparmdb", data_mapfile,
            mapfile=os.path.join(mapfile_dir, 'dppp.parmdb.mapfile'),
            suffix='.dppp.parmdb')['mapfile']

        # Create a sourcedb to be used by the demixing phase of DPPP
        # The path to the A-team sky model is currently hard-coded.
        # Run makesourcedb on skymodel files for calibrator source(s) and the
        # Ateam, which are to be stored in a standard place ($LOFARROOT/share)
        sourcedb_mapfile = self.run_task(
            "setupsourcedb", data_mapfile,
            skymodel=os.path.join(
                self.config.get('DEFAULT', 'lofarroot'),
                'share', 'pipeline', 'skymodels', 'Ateam_LBA_CC.skymodel'),
                                         # TODO: LBA skymodel!! 
            mapfile=os.path.join(mapfile_dir, 'dppp.sourcedb.mapfile'),
            suffix='.dppp.sourcedb',
            type='blob')['mapfile']

        # *********************************************************************
        # 3. Run NDPPP to demix the A-Team sources
        #    TODOW: Do flagging?
        # Create a parameter-subset for DPPP and write it to file.
        ndppp_parset = os.path.join(parset_dir, "NDPPP.parset")
        py_parset.makeSubset('DPPP.').writeFile(ndppp_parset)

        # Run the Default Pre-Processing Pipeline (DPPP);
        dppp_mapfile = self.run_task("ndppp",
            data_mapfile,
            data_start_time=vdsinfo['start_time'],
            data_end_time=vdsinfo['end_time'],
            parset=ndppp_parset,
            parmdb_mapfile=parmdb_mapfile,
            sourcedb_mapfile=sourcedb_mapfile)['mapfile']

        demix_mapfile = dppp_mapfile

#        # Old Demixing method: performed now by ndppp
#        # Demix the relevant A-team sources
#        demix_mapfile = self.run_task("demixing", dppp_mapfile)['mapfile']

#        # Do a second run of flagging, this time using rficonsole
#        self.run_task("rficonsole", demix_mapfile, indirect_read=True)

        # *********************************************************************
        # 4. Run BBS with a model of the calibrator
        #    Create a parmdb for calibration solutions
        #    Create sourcedb with known calibration solutions
        #    Run bbs with both
        # Create an empty parmdb for BBS
        parmdb_mapfile = self.run_task(
            "setupparmdb", data_mapfile,
            mapfile=os.path.join(mapfile_dir, 'bbs.parmdb.mapfile'),
            suffix='.bbs.parmdb')['mapfile']


        # Create a sourcedb based on sourcedb's input argument "skymodel"
        sourcedb_mapfile = self.run_task(
            "setupsourcedb", data_mapfile,
            skymodel=os.path.join(
                self.config.get('DEFAULT', 'lofarroot'),
                'share', 'pipeline', 'skymodels',
                py_parset.getString('Calibration.CalibratorSource') +
                    '.skymodel'),
            mapfile=os.path.join(mapfile_dir, 'bbs.sourcedb.mapfile'),
            suffix='.bbs.sourcedb')['mapfile']

        # Create a parameter-subset for BBS and write it to file.
        bbs_parset = os.path.join(parset_dir, "BBS.parset")
        py_parset.makeSubset('BBS.').writeFile(bbs_parset)

        # Run BBS to calibrate the calibrator source(s).
        self.run_task("bbs_reducer",
            dppp_mapfile,
            parset=bbs_parset,
            instrument_mapfile=parmdb_mapfile,
            sky_mapfile=sourcedb_mapfile)

        # *********************************************************************
        # 5. Perform gain outlier correction on the found calibration solutions
        #    Swapping outliers in the gains with the median 
        # Export the calibration solutions using gainoutliercorrection and store
        # the results in the files specified in the instrument mapfile.
        self.run_task("gainoutliercorrection",
                      (parmdb_mapfile, instrument_mapfile),
                      sigma=1.0) # TODO: Parset parameter

        # *********************************************************************
        # 6. Create feedback file for further processing by the LOFAR framework
        # (MAC)
        # Create a parset-file containing the metadata for MAC/SAS
        self.run_task("get_metadata", instrument_mapfile,
            parset_file=self.parset_feedback_file,
            parset_prefix=(
                self.parset.getString('prefix') +
                self.parset.fullModuleName('DataProducts')),
            product_type="InstrumentModel")


if __name__ == '__main__':
    sys.exit(msss_calibrator_pipeline().main())
