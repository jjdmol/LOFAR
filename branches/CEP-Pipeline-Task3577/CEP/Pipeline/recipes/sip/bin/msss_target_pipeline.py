#!/usr/bin/env python
#                                                         LOFAR IMAGING PIPELINE
#
#                                          Target Pre-Processing Pipeline recipe
#                                                             Marcel Loose, 2011
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

import os
import sys

from lofarpipe.support.control import control
from lofarpipe.support.lofarexceptions import PipelineException
from lofarpipe.support.group_data import store_data_map, validate_data_maps, \
        load_data_map
from lofarpipe.support.group_data import tally_data_map
from lofarpipe.support.utilities import create_directory
from lofar.parameterset import parameterset

class msss_target_pipeline(control):
    """
    The target pipeline can be used to calibrate the observation of a "target"
    source using an instrument database that was previously determined using
    the calibrator_pipeline.

    This pipeline will perform the following operations:
    
    1. Prepare phase, collect data from parset and input mapfiles
    2. Copy the instrument files to the correct node, create new file with
       succesfull copied mss.
    3. Create database needed for performing work: 
       Vds, descibing data on the nodes sourcedb, For skymodel (A-team)
       parmdb for outputtting solutions
    4. Run NDPPP to demix the A-Team sources
    5. Run bss using the instrument file from the target observation, to correct for instrumental effects
    6. Second dppp run for  flaging NaN's in the MS. 
    7. Create feedback file for further processing by the LOFAR framework (MAC)

    **Per subband-group, the following output products will be delivered:**

    1. A new MS with a DATA column containing calibrated data

    """

    def __init__(self):
        control.__init__(self)
        self.parset = parameterset()
        self.input_data = {}
        self.output_data = {}
        self.io_data_mask = []
        self.parset_feedback_file = None


    def usage(self):
        """
        Display usage information
        """
        print >> sys.stderr, "Usage: %s [options] <parset-file>" % sys.argv[0]
        return 1


    def _get_io_product_specs(self):
        """
        Get input- and output-data product specifications from the
        parset-file, and do some sanity checks.
        """
        odp = self.parset.makeSubset('ObsSW.Observation.DataProducts.')
        self.input_data['data'] = [
            tuple(os.path.join(location, filename).split(':'))
                for location, filename, skip in zip(
                    odp.getStringVector('Input_Correlated.locations'),
                    odp.getStringVector('Input_Correlated.filenames'),
                    odp.getBoolVector('Input_Correlated.skip'))
                if not skip
        ]
        self.logger.debug("%d Input_Correlated data products specified" %
                          len(self.input_data['data']))
        self.input_data['instrument'] = [
            tuple(os.path.join(location, filename).split(':'))
                for location, filename, skip in zip(
                    odp.getStringVector('Input_InstrumentModel.locations'),
                    odp.getStringVector('Input_InstrumentModel.filenames'),
                    odp.getBoolVector('Input_InstrumentModel.skip'))
                if not skip
        ]
        self.logger.debug("%d Input_InstrumentModel data products specified" %
                          len(self.input_data['instrument']))
        self.output_data['data'] = [
            tuple(os.path.join(location, filename).split(':'))
                for location, filename, skip in zip(
                    odp.getStringVector('Output_Correlated.locations'),
                    odp.getStringVector('Output_Correlated.filenames'),
                    odp.getBoolVector('Output_Correlated.skip'))
                if not skip
        ]
        self.logger.debug("%d Output_Correlated data products specified" %
                          len(self.output_data['data']))


    def _validate_io_product_specs(self):
        """
        Sanity checks on input- and output data product specifications
        """
        if not validate_data_maps(
            self.input_data['data'],
            self.input_data['instrument'],
            self.output_data['data']
        ):  raise PipelineException(
                "Validation of input/output data product specification failed!"
            )
        # Validate input data, by searching the cluster for files
        self._validate_input_data()
        # Update input- and output-data product specifications if needed.
        if not all(self.io_data_mask):
            self.logger.info("Updating input/output product specifications")
            self.input_data['data'] = [f for (f, m)
                in zip(self.input_data['data'], self.io_data_mask) if m
            ]
            self.input_data['instrument'] = [f for (f, m)
                in zip(self.input_data['instrument'], self.io_data_mask) if m
            ]
            self.output_data['data'] = [f for (f, m)
                in zip(self.output_data['data'], self.io_data_mask) if m
            ]


    def _validate_input_data(self):
        """
        Search for the requested input files and mask the files in
        `self.input_data{}` that could not be found on the system.
        """
        # Use filename glob-pattern as defined in LOFAR-USG-ICD-005.
        data_mask = tally_data_map(
            self.input_data['data'], 'L*_SB???_uv.MS', self.logger
        )
        # Log a warning if not all input data files were found.
        if not all(data_mask):
            self.logger.warn(
                "The following input data files were not found: %s" %
                ', '.join(
                    ':'.join(f) for (f, m) in zip(
                        self.input_data['data'], data_mask
                    ) if not m
                )
            )
        # Use filename glob-pattern as defined in LOFAR-USG-ICD-005.
        inst_mask = tally_data_map(
            self.input_data['instrument'], 'L*_SB???_inst.INST', self.logger
        )
        # Log a warning if not all input instrument files were found.
        if not all(inst_mask):
            self.logger.warn(
                "The following input instrument files were not found: %s" %
                ', '.join(
                    ':'.join(f) for (f, m) in zip(
                        self.input_data['instrument'], inst_mask
                    ) if not m
                )
            )

        # Set the IO data mask
        self.io_data_mask = [x and y for (x, y) in zip(data_mask, inst_mask)]


    def _create_target_map_for_instruments(self, instrument_map,
                                           input_data_map):
        """
        Create a mapfile with target locations: based on the host found in the 
        input_data_map, the name of the instrument file and the working \
        directory + job name
        """
        scratch_dir = os.path.join(
            self.inputs['working_directory'], self.inputs['job_name'])

        target_locations = []
        for instrument_pair, data_pair in zip(instrument_map, input_data_map):
            host_instr, path_instr = instrument_pair
            host_data, path_data = data_pair
            # target location == working dir instrument file name
            target_path = os.path.join(scratch_dir, os.path.basename(path_instr))
            target_locations.append((host_data, target_path))

        return target_locations


    def _copy_instrument_files(self, instrument_map, input_data_map,
                                mapfile_dir):
        # For the copy recipe a target mapfile is needed
        # create target map based on the node and the dir in the input_data_map
        # with the filename based on the
        copier_map_path = os.path.join(mapfile_dir, "copier")
        create_directory(copier_map_path)
        target_map = self._create_target_map_for_instruments(instrument_map,
                                                     input_data_map)

        #Write the two needed maps to file
        source_path = os.path.join(copier_map_path, "source_instruments.map")
        store_data_map(source_path, instrument_map)

        target_path = os.path.join(copier_map_path, "target_instruments.map")
        store_data_map(target_path, target_map)

        copied_files_path = os.path.join(copier_map_path, "copied_instruments.map")

        # The output of the copier is a mapfile containing all the host, path
        # of succesfull copied files.
        copied_instruments_mapfile = self.run_task("copier",
                      mapfile_source=source_path,
                      mapfile_target=target_path,
                      mapfiles_dir=copier_map_path,
                      mapfile=copied_files_path)['mapfile_target_copied']

        # Some copy action might fail, these files need to be removed from
        # both the data and the instrument file!!
        copied_instruments_map = load_data_map(copied_instruments_mapfile)
        new_instrument_map = []
        new_input_data_map = []
        for instrument_pair, input_data_pair in zip(target_map, input_data_map):
            if instrument_pair in copied_instruments_map:
                new_instrument_map.append(instrument_pair)
                new_input_data_map.append(input_data_pair)
            # else: Do not process further in the recipe

        return new_instrument_map, new_input_data_map

    def go(self):
        """
        Read the parset-file that was given as input argument, and set the
        jobname before calling the base-class's `go()` method.
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
                os.path.splitext(os.path.basename(parset_file))[0]
            )
        return super(msss_target_pipeline, self).go()


    def pipeline_logic(self):
        """
        Define the individual tasks that comprise the current pipeline.
        This method will be invoked by the base-class's `go()` method.
        """
        # *********************************************************************
        # 1. Prepare phase, collect data from parset and input mapfiles
        # Create a parameter-subset containing only python-control stuff.
        py_parset = self.parset.makeSubset(
            'ObsSW.Observation.ObservationControl.PythonControl.')

        # Get input/output-data products specifications.
        self._get_io_product_specs()

        # Create some needed directories
        job_dir = self.config.get("layout", "job_directory")
        mapfile_dir = os.path.join(job_dir, "mapfiles")
        create_directory(mapfile_dir)
        parset_dir = os.path.join(job_dir, "parsets")
        create_directory(parset_dir)

        # *********************************************************************
        # 2. Copy the instrument files to the correct node
        # The instrument files are currently located on the wrong nodes
        # Copy to correct nodes and assign the instrument table the now
        # correct data

        # Copy the instrument files to the corrent nodes: failures might happen
        # update both intrument and datamap to contain only successes!
        self.input_data['instrument'], self.input_data['data'] = \
            self._copy_instrument_files(self.input_data['instrument'],
                                    self.input_data['data'], mapfile_dir)


        # Write input- and output data map-files.
        data_mapfile = os.path.join(mapfile_dir, "data.mapfile")
        store_data_map(data_mapfile, self.input_data['data'])
        copied_instrument_mapfile = os.path.join(mapfile_dir, "copied_instrument.mapfile")
        store_data_map(copied_instrument_mapfile,
                       self.input_data['instrument'])
        self.logger.debug(
            "Wrote input data mapfile: %s" % data_mapfile
        )

        # Save copied files to a new mapfile
        corrected_mapfile = os.path.join(mapfile_dir, "corrected_data.mapfile")
        store_data_map(corrected_mapfile, self.output_data['data'])
        self.logger.debug(
            "Wrote output corrected data mapfile: %s" % corrected_mapfile
        )

        # Validate number of copied files, abort on zero files copied
        if len(self.input_data['data']) == 0:
            self.logger.warn("No input data files to process. Bailing out!")
            return 0

        self.logger.debug("Processing: %s" %
            ', '.join(':'.join(f) for f in self.input_data['data'])
        )

        # *********************************************************************
        # 3. Create database needed for performing work: 
        #    Vds, descibing data on the nodes
        #    sourcedb, For skymodel (A-team)
        #    parmdb for outputtting solutions
        # Produce a GVDS file describing the data on the compute nodes.
        gvds_file = self.run_task("vdsmaker", data_mapfile)['gvds']

        # Read metadata (e.g., start- and end-time) from the GVDS file.
        vdsinfo = self.run_task("vdsreader", gvds=gvds_file)

        # Create an empty parmdb for DPPP
        parmdb_mapfile = self.run_task("setupparmdb", data_mapfile)['mapfile']

        # Create a sourcedb to be used by the demixing phase of DPPP
        # The path to the A-team sky model is currently hard-coded.
        sourcedb_mapfile = self.run_task(
            "setupsourcedb", data_mapfile,
            skymodel=os.path.join(
                self.config.get('DEFAULT', 'lofarroot'),
                'share', 'pipeline', 'skymodels', 'Ateam_LBA_CC.skymodel'
            )
        )['mapfile']

        # *********************************************************************
        # 4. Run NDPPP to demix the A-Team sources
        # Create a parameter-subset for DPPP and write it to file.
        ndppp_parset = os.path.join(parset_dir, "NDPPP[0].parset")
        py_parset.makeSubset('DPPP[0].').writeFile(ndppp_parset)

        # Run the Default Pre-Processing Pipeline (DPPP);
        dppp_mapfile = self.run_task("ndppp",
            data_mapfile,
            data_start_time=vdsinfo['start_time'],
            data_end_time=vdsinfo['end_time'],
            parset=ndppp_parset,
            parmdb_mapfile=parmdb_mapfile,
            sourcedb_mapfile=sourcedb_mapfile,
            mapfile=os.path.join(mapfile_dir, 'dppp[0].mapfile')
        )['mapfile']

        demix_mapfile = dppp_mapfile


#        # Demix the relevant A-team sources
#        demix_mapfile = self.run_task("demixing", dppp_mapfile)['mapfile']

        # ********************************************************************
        # 5. Run bss using the instrument file from the target observation
        # Create an empty sourcedb for BBS
        sourcedb_mapfile = self.run_task(
            "setupsourcedb", data_mapfile
        )['mapfile']

        # Create a parameter-subset for BBS and write it to file.
        bbs_parset = os.path.join(parset_dir, "BBS.parset")
        py_parset.makeSubset('BBS.').writeFile(bbs_parset)

        # Run BBS to calibrate the target source(s).
        bbs_mapfile = self.run_task("bbs_reducer",
            dppp_mapfile,
            parset=bbs_parset,
            instrument_mapfile=copied_instrument_mapfile,
            sky_mapfile=sourcedb_mapfile
        )['mapfile']

        # *********************************************************************
        # 6. Second dppp run for  flaging NaN's in the MS.  
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
            parset=ndppp_parset,
            mapfile=os.path.join(mapfile_dir, 'dppp[1].mapfile')
        )

        # 7. Create feedback file for further processing by the LOFAR framework
        # (MAC)
        # Create a parset-file containing the metadata for MAC/SAS
        self.run_task("get_metadata", corrected_mapfile,
            parset_file=self.parset_feedback_file,
            parset_prefix=(
                self.parset.getString('prefix') +
                self.parset.fullModuleName('DataProducts')
            ),
            product_type="Correlated")


if __name__ == '__main__':
    sys.exit(msss_target_pipeline().main())
