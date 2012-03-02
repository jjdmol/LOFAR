#!/usr/bin/env python
#                                                         LOFAR IMAGING PIPELINE
#
#                                                         Imager Pipeline recipe
#                                                             Marcel Loose, 2012
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

import os
import sys
import errno

from lofarpipe.support.control import control
from lofar.parameterset import parameterset #@UnresolvedImport
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.utilities import create_directory


class imager_pipeline(control):
    """
    The MSSS imager pipeline can be used to generate MSSS images.

    MSSS images are compiled from a number of so-called slices. Each slice
    comprises a short (approx. 10 min) observation of a field (an area on the
    sky) containing 80 subbands. The number of slices will be different for LBA
    observations (typically 9) and HBA observations (typically 2), due to
    differences in sensitivity.

    One MSSS observation will produce a number of images (typically 8), one for
    each so-called subband-group (SBG). Each SBG consists of the same number
    of consecutive subbands (typically 10).
    
    Each image will be compiled on a different cluster node to balance the
    processing load. The input- and output- files and locations are determined
    by the scheduler and specified in the parset-file.

    This pipeline will perform the following operations:
    - Copy the preprocessed MS's from the different compute nodes to the nodes
      where the images will be compiled (the prepare phase).
    - Flag the long baselines using DPPP
    - Concatenate the MS's of the different slices as one virtual MS for
      imaging.
    - Generate a local sky model (LSM) from the global sky model (GSM) for the
      sources that are in the field-of-view (FoV).
    - Repeat until convergence (3 times for the time being):
      - Per slice: solve and correct for phases using BBS with TEC enabled
      - Run the awimager.
      - Run the source finder (PyBDSM) and update the local sky model (LSM).
      
    Per subband-group, the following output products will be delivered:
    - Calibration solutions and corrected visibilities
    - An image
    - A source list
    """


    inputs = {
        'input_mapfile': ingredient.FileField(
            '--input-mapfile',
            help = "mapfile with inputs specific to this node script"
        )
    }


    def __init__(self):
        control.__init__(self)
        self.parset = parameterset()
        self.input_data = {}
        self.output_data = {}



    def pipeline_logic(self):
        """
        Define the individual tasks that comprise the current pipeline.
        This method will be invoked by the base-class's `go()` method.
        """
        self.logger.info("Starting imager pipeline: Parset used:")
        self.logger.info(self.parset.keys())

        # (1) ******************************************************************
        # prepare phase: copy and collect the ms
        prepare_phase_output = self._prepare_phase(self.inputs['input_mapfile'],
                                                   skip_prepare = True)


        # (2) ******************************************************************
        # Create dbs and sky model
        parmdbs_path, sky_path = self._create_dbs(
                        prepare_phase_output, skip_create_dbs = False)


        # (3) *******************************************************************#
        # bbs_imager recipe
        # TODO: input from previous steps
        bbs_output = self._bbs()
        return 0
        # (4) ******************************************************************
        # Get parameters awimager from the prepare_parset and inputs 
        awimager_output_mapfile = self._aw_imager(prepare_phase_output, skip = True)

        # (5) *****************************************************************
        # Source finding
        found_sources_list = self._source_finding()

        return 0


    def usage(self):
        print >> sys.stderr, "Usage: %s <parset-file>  [options]" % sys.argv[0]
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
        super(imager_pipeline, self).go()

    def _source_finding(self, skip = True):
        if skip:
            return [('lce068', '/data/scratch/klijn/temp_sourcelist')]
        else:
            return self.run_task("imager_source_finding", "/home/klijn/build/preparation/bdsm_input.map",
                       job = "imager_source_finding",
                       initscript = "/opt/cep/LofIm/daily/lofar/lofarinit.sh",
                       bdsm_parset_file_run1 = "/home/klijn/build/preparation/bdsm_parameters.map",
                       bdsm_parset_file_run2x = "/home/klijn/build/preparation/bdsm_parameters_2x.map",
                       catalog_output_path = "/data/scratch/klijn/bdsm_output_cat"
                        )["sourlist_file"]


    def _bbs(self):
        parset = self.parset.makeSubset("Bbs.")
        parset_path = self._write_parset_to_file(parset, "bbs")

        # temporary mapfile until the single treaded bbs is ready
        new_bbs_input = "/home/klijn/build/preparation/new_bbs.input.map"
        bbs_imager_mapfile = \
            self.run_task("imager_bbs",
                          new_bbs_input,
                          job = "Pipeline",
                          parset = "/home/klijn/build/preparation/bbs_new.par",
                          instrument_mapfile = "/home/klijn/build/preparation/new_bbs_instrument.map",
                          sky_mapfile = "/home/klijn/build/preparation/new_bbs_sky.map",
                          kernel_exec = "/opt/cep/LofIm/daily/lofar/bin/KernelControl",
                          control_exec = "/opt/cep/LofIm/daily/lofar/bin/GlobalControl",
                          db_name = "klijn",
                          db_host = "ldb002",
                          db_user = "postgres",
                          db_key = "new_bbs",
                          runtime_directory = "/home/klijn/runtime_directory/jobs/Pipeline",
                          new_bbs_path = "/home/klijn/build/gnu_debug/installed/lib/python2.6/dist-packages/lofarpipe/recipes/master/new_bbs.py",
                          gvds_path = os.path.join(self.config.get("DEFAULT", "default_working_directory"), "time_slices") # TODO deze dir bestaat niet op de nodes en moet worden aangemaakt
                          )

        self.logger.info(open("/home/klijn/build/preparation/new_bbs_output.map").read())
        self.logger.error("bbs_imager_mapfile")
        return None

    def _aw_imager(self, prepare_phase_output, skip = False):
      awimager_output_mapfile = None
      if skip:
        awimager_output_mapfile = ""
      else:
        parset = self.parset.makeSubset("awimager.")
        parset_path = self._write_parset_to_file(parset, "awimager")

        self.logger.info(parset_path)
        #run the awimager recipe
        awimager_output_mapfile = \
            self.run_task("imager_awimager", prepare_phase_output,
                          parset = parset_path,
                          executable = parset.getString("executable"))

      return awimager_output_mapfile



    def _prepare_phase(self, input_ms_map_path, skip_prepare = False):
        # get the parameters, create a subset for ndppp, save
        prepare_parset = self.parset.makeSubset("Prepare.")
        ndppp_parset = prepare_parset.makeSubset("Ndppp.")
        ndppp_parset_path = self._write_parset_to_file(ndppp_parset,
                                                       "prepare_imager_ndppp")

        # Run the prepare phase script 
        # TODO: wrapping to allow for skipping
        prepare_phase_output = None
        if skip_prepare:
          prepare_phase_output = "/home/klijn/build/preparation/actual_output.map"
        else:
          prepare_phase_output = \
            self.run_task("imager_prepare", input_ms_map_path,
                ndppp_path = prepare_parset.getString("ndppp_path"),
                parset = ndppp_parset_path,
                output_products_mapfile = prepare_parset.getString("output_products_mapfile"),
                slices_per_image = prepare_parset.getInt("slices_per_image"),
                subbands_per_image = prepare_parset.getInt("subbands_per_image"),
                mapfile = prepare_parset.getString("mapfile"))['mapfile']

        self.logger.info(prepare_phase_output)
        return prepare_phase_output


    def _create_dbs(self, input_map_path, skip_create_dbs = False):
        """
        The executable called by this script do not consume parsets:
        Parse the parset entries and pass as argument       
        """
        parset = self.parset.makeSubset("Create_dbs.")
        parset_path = self._write_parset_to_file(parset, "create_dbs")

        recipe_output = None
        if skip_create_dbs:
            recipe_output = {}
            recipe_output["parmdbs_path"] = None
            recipe_output["sky_path"] = None
        else:
            recipe_output = \
                self.run_task("imager_create_dbs", input_map_path,
                        parset = parset_path,
                        monetdb_hostname = parset.getString("monetdb_hostname"),
                        monetdb_port = parset.getInt("monetdb_port"),
                        monetdb_name = parset.getString("monetdb_name"),
                        monetdb_user = parset.getString("monetdb_user"),
                        monetdb_password = parset.getString("monetdb_password"),
                        assoc_theta = parset.getString("assoc_theta"),
                        suffix = parset.getString("suffix"),
                        sourcedb_target_path = parset.getString("sourcedb_target_path"),
                        parmdb_executable = parset.getString("parmdb_executable"),
                        slice_paths_mapfile = parset.getString("slice_paths_mapfile"),
                        parmdb_suffix = parset.getString("parmdb_suffix"),
                        monetdb_path = parset.getString("monetdb_path"),
                        gsm_path = parset.getString("gsm_path"),
                        makesourcedb_path = parset.getString("makesourcedb_path"),
                        parmdbs_path = parset.getString("parmdbs_path"),
                        sky_path = parset.getString("sky_path")
                        )

        self.logger.info(recipe_output["parmdbs_path"])
        self.logger.info(recipe_output["sky_path"])
        return recipe_output["parmdbs_path"], recipe_output["sky_path"]




    def _write_parset_to_file(self, parset, parset_name):
        """
        Write the suplied the suplied parameterset to the parameter set 
        directory in the jobs dir with the filename suplied in parset_name.
        Return the full path to the created file.
        
        """
        # todo make a subset parset and supply to the prepareimager        
        parset_dir = os.path.join(
            self.config.get("layout", "job_directory"), "parsets")
        #create the parset dir if it does not exist
        create_directory(parset_dir)

        #write the content to a new parset file
        prepare_imager_parset_file = os.path.join(parset_dir,
                         "{0}.parset".format(parset_name))
        parset.writeFile(prepare_imager_parset_file)

        return prepare_imager_parset_file

if __name__ == '__main__':
    sys.exit(imager_pipeline().main())


#        # bbs_imager recipe
#        # imputs moeten nog dynamics en dan klaar!
#        new_bbs_input = "/home/klijn/build/preparation/new_bbs.input.map"
#        bbs_imager_mapfile = \
#            self.run_task("new_bbs",
#                          new_bbs_input,
#                          initscript = "/opt/cep/LofIm/daily/lofar/lofarinit.sh",
#                          parset = "/home/klijn/build/preparation/bbs_new.par",
#                          instrument_mapfile = "/home/klijn/build/preparation/new_bbs_instrument.map",
#                          data_mapfile = "/home/klijn/build/preparation/new_bbs_output.map",
#                          kernel_exec = "/opt/cep/LofIm/daily/Fri/lofar_build/install/gnu_opt/bin/KernelControl",
#                          control_exec = "/opt/cep/LofIm/daily/Fri/lofar_build/install/gnu_opt/bin/GlobalControl",
#                          sky_mapfile = "/home/klijn/build/preparation/new_bbs_sky.map",
#                          db_name = "klijn",
#                          db_host = "ldb002",
#                          db_user = "postgres",
#                          db_key = "new_bbs")
#
#
#        return 0


