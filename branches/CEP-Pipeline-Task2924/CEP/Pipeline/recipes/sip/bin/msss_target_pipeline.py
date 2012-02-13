#!/usr/bin/env python
#                                                         LOFAR IMAGING PIPELINE
#
#                                                     Calibrator Pipeline recipe
#                                                             Marcel Loose, 2011
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

import os
import sys

from lofarpipe.support.control import control
from lofarpipe.support.lofarexceptions import PipelineException
from lofarpipe.support.group_data import store_data_map, validate_data_maps
from lofarpipe.support.utilities import create_directory
from lofar.parameterset import parameterset
from lofar.mstools import findFiles

class msss_target_pipeline(control):
    """
    The target pipeline can be used to calibrate the observation of a "target"
    source using an instrument database that was previously determined using
    the calibrator_pipeline.

    This pipeline will perform the following operations:
    - DPPP: flagging, using standard parset
    - Demix the relevant A-team sources (for now using python script, later
      to use DPPP), using the A-team sourcedb.
    - Run BBS to correct for instrumental effects using the instrument database
      from an earlier calibrator_pipeline run.
    """

    def __init__(self):
        control.__init__(self)
        self.parset = parameterset()
        self.input_data = {}
        self.output_data = {}
        self.io_data_mask = []


    def usage(self):
        print >> sys.stderr, "Usage: %s [options] <parset-file>" % sys.argv[0]
        return 1


    def _get_io_product_specs(self):
        """
        Get input- and output-data product specifications from the
        parset-file, and do some sanity checks.
        """
        odp = self.parset.makeSubset('ObsSW.Observation.DataProducts.')
        self.input_data['data'] = [
            tuple(''.join(x).split(':')) for x in zip(
                odp.getStringVector('Input_Correlated.locations', []),
                odp.getStringVector('Input_Correlated.filenames', []))
        ]
        self.logger.debug("%d Input_Correlated data products specified" %
                          len(self.input_data['data']))
        self.input_data['instrument'] = [
            tuple(''.join(x).split(':')) for x in zip(
                odp.getStringVector('Input_InstrumentModel.locations', []),
                odp.getStringVector('Input_InstrumentModel.filenames', []))
        ]
        self.logger.debug("%d Input_InstrumentModel data products specified" %
                          len(self.input_data['instrument']))
        self.output_data['data'] = [
            tuple(''.join(x).split(':')) for x in zip(
                odp.getStringVector('Output_Correlated.locations', []),
                odp.getStringVector('Output_Correlated.filenames', []))
        ]
        self.logger.debug("%d Output_Correlated data products specified" %
                          len(self.output_data['data']))
        # Sanity checks on input- and output data product specifications
        if not validate_data_maps(
            self.input_data['data'], 
            self.input_data['instrument'], 
            self.output_data['data']
        ):  raise PipelineException(
                "Validation of input/output data product specification failed!"
            )
        # Validate input data, by searching the cluster for files
        self._validate_input_data()
        # Update input- and output-data product specifications
        self.input_data['data'] = [f for (f,m) 
            in zip(self.input_data['data'], self.io_data_mask) if m
        ]
        self.input_data['instrument'] = [f for (f,m) 
            in zip(self.input_data['instrument'], self.io_data_mask) if m
        ]
        self.output_data['data'] = [f for (f,m) 
            in zip(self.output_data['data'], self.io_data_mask) if m
        ]


    def _validate_input_data(self):
        """
        Search for the requested input files and mask the files in
        `self.input_data{}` that could not be found on the system.
        """
        # First determine the directories to search. Get unique directory
        # names from our input_data by creating a set first.
        dirs = list(set(os.path.dirname(d[1]) for d in self.input_data['data']))

        # Compose the filename glob-pattern to use (see LOFAR-USG-ICD-005).
        ms_pattern = ' '.join(
            os.path.join(d, 'L*_SAP???_SB???_uv.MS') for d in dirs
        )

        # Search the files on the cluster; turn them into a list of tuples.
        self.logger.debug("Searching for files: %s" % ms_pattern)
        found_files = zip(*findFiles(ms_pattern, '-1d'))
        
        # Create a mask containing True if file exists, False otherwise
        data_mask = [f in found_files for f in self.input_data['data']]

        # Log a warning if not all input data files were found.
        if not all(data_mask):
            self.logger.warn(
                "The following input data files were not found: %s" %
                ' '.join(
                    ':'.join(f) for (f,m) in zip(
                        self.input_data['data'], data_mask
                    ) if not m
                )
            )

        dirs = list(
            set(os.path.dirname(d[1]) for d in self.input_data['instrument'])
        )
        ms_pattern = ' '.join(
            os.path.join(d, 'L*_SAP???_SB???_inst.INST') for d in dirs
        )
        self.logger.debug("Searching for files: %s" % ms_pattern)
        found_files = zip(*findFiles(ms_pattern, '-1d'))
        inst_mask = [f in found_files for f in self.input_data['instrument']]
        if not all(inst_mask):
            self.logger.warn(
                "The following input instrument files were not found: %s" %
                ' '.join(
                    ':'.join(f) for (f,m) in zip(
                        self.input_data['instrument'], inst_mask
                    ) if not m
                )
            
            )

        # Set the IO data mask
        self.io_data_mask = [x and y for (x,y) in zip(data_mask, inst_mask)]


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
        super(msss_target_pipeline, self).go()


    def pipeline_logic(self):
        """
        Define the individual tasks that comprise the current pipeline.
        This method will be invoked by the base-class's `go()` method.
        """

        # Create a parameter-subset containing only python-control stuff.
        py_parset = self.parset.makeSubset(
            'ObsSW.Observation.ObservationControl.PythonControl.')

        # Get input/output-data products specifications.
        self._get_io_product_specs()
        
        job_dir = self.config.get("layout", "job_directory")
        parset_dir = os.path.join(job_dir, "parsets")
        mapfile_dir = os.path.join(job_dir, "mapfiles")

        # Write input- and output data map-files.
        create_directory(parset_dir)
        create_directory(mapfile_dir)
        
        data_mapfile = os.path.join(mapfile_dir, "data.mapfile")
        store_data_map(data_mapfile, self.input_data['data'])
        self.logger.debug(
            "Wrote input data mapfile: %s" % data_mapfile
        )
        instrument_mapfile = os.path.join(mapfile_dir, "instrument.mapfile")
        store_data_map(instrument_mapfile, self.input_data['instrument'])
        self.logger.debug(
            "Wrote input instrument mapfile: %s" % instrument_mapfile
        )
        corrected_mapfile = os.path.join(mapfile_dir, "corrected_data.mapfile")
        store_data_map(corrected_mapfile, self.output_data['data'])
        self.logger.debug(
            "Wrote output corrected data mapfile: %s" % corrected_mapfile
        )
        
        if len(self.input_data['data']) == 0:
            self.logger.warn("No input data files to process. Bailing out")
            return 0

        self.logger.debug("Processing: %s" % self.input_data['data'])
            
#        # Create a sourcedb based on sourcedb's input argument "skymodel"
#        # (see, e.g., tasks.cfg file).
#        sourcedb_mapfile = self.run_task("sourcedb", data_mapfile)['mapfile']

        # Produce a GVDS file describing the data on the compute nodes.
        gvds_file = self.run_task("vdsmaker", data_mapfile)['gvds']

        # Read metadata (e.g., start- and end-time) from the GVDS file.
        vdsinfo = self.run_task("vdsreader", gvds=gvds_file)

        # Create a parameter-subset for DPPP and write it to file.
        ndppp_parset = os.path.join(parset_dir, "NDPPP[0].parset")
        py_parset.makeSubset('DPPP[0].').writeFile(ndppp_parset)

        # Run the Default Pre-Processing Pipeline (DPPP);
        dppp_mapfile = self.run_task("ndppp", 
            data_mapfile,
            data_start_time=vdsinfo['start_time'],
            data_end_time=vdsinfo['end_time'],
            parset=ndppp_parset
        )['mapfile']

        # Demix the relevant A-team sources
        demix_mapfile = self.run_task("demixing", dppp_mapfile)['mapfile']

        # Create a parameter-subset for BBS and write it to file.
        bbs_parset = os.path.join(parset_dir, "BBS.parset")
        py_parset.makeSubset('BBS.').writeFile(bbs_parset)

        # Run BBS to calibrate the target source(s).
        bbs_mapfile = self.run_task("new_bbs", 
            demix_mapfile,
            parset=bbs_parset,
            instrument_mapfile=instrument_mapfile,
            sky_mapfile=sourcedb_mapfile
        )['mapfile']

        # Create another parameter-subset for a second DPPP run.
        ndppp_parset = os.path.join(parset_dir, "NDPPP[1].parset")
        py_parset.makeSubset('DPPP[1].').writeFile(ndppp_parset)

        # Do a second run of DPPP, just to flag NaN's from the MS. Store the
        # results in the files specified in the corrected data map-file
        # WARNING: This will create a new MS with a DATA column containing the
        # CORRECTED_DATA column of the original MS.
        self.run_task("ndppp", 
            (bbs_mapfile, corrected_mapfile),
            clobber=False,
            suffix='',
            parset=ndppp_parset
        )


if __name__ == '__main__':
    sys.exit(msss_target_pipeline().main())
