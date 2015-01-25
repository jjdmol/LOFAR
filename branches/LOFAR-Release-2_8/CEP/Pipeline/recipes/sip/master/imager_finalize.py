from __future__ import with_statement
import sys

import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.data_map import DataMap, validate_data_maps, \
                                       align_data_maps

class imager_finalize(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    The Imager_finalizer performs a number of steps needed for integrating the
    msss_imager_pipeline in the LOFAR framework: It places the image on the
    output location in the correcy image type (hdf5).
    It also adds some meta data collected from the individual measurement sets
    and the found data.
   
    This recipe does not have positional commandline arguments 
    """
    inputs = {
        'awimager_output_map': ingredient.FileField(
            '--awimager-output-mapfile',
            help="""Mapfile containing (host, path) pairs of created sky
                   images """
        ),
        'raw_ms_per_image_map': ingredient.FileField(
            '--raw-ms-per-image-map',
            help='''Mapfile containing (host, path) pairs of mapfiles used
            to create image on that node'''
        ),
        'sourcelist_map': ingredient.FileField(
            '--sourcelist-map',
            help='''mapfile containing (host, path) pairs to a list of sources
            found in the image'''
        ),
        'sourcedb_map': ingredient.FileField(
            '--sourcedb_map',
            help='''mapfile containing (host, path) pairs to a db of sources
            found in the image'''
        ),
        'target_mapfile': ingredient.FileField(
            '--target-mapfile',
            help="Mapfile containing (host, path) pairs to the concatenated and"
            "combined measurement set, the source for the actual sky image"
        ),
        'minbaseline': ingredient.FloatField(
            '--minbaseline',
            help='''Minimum length of the baseline used for the images'''
        ),
        'maxbaseline': ingredient.FloatField(
            '--maxbaseline',
            help='''Maximum length of the baseline used for the images'''
        ),
        'output_image_mapfile': ingredient.FileField(
            '--output-image-mapfile',
            help='''mapfile containing (host, path) pairs with the final
            output image (hdf5) location'''
        ),
        'processed_ms_dir': ingredient.StringField(
            '--processed-ms-dir',
            help='''Path to directory for processed measurment sets'''
        ),
        'fillrootimagegroup_exec': ingredient.ExecField(
            '--fillrootimagegroup_exec',
            help='''Full path to the fillRootImageGroup executable'''
        ),
        'placed_image_mapfile': ingredient.FileField(
            '--placed-image-mapfile',
            help="location of mapfile with proced and correctly placed,"
                " hdf5 images"
        )
    }

    outputs = {
        'placed_image_mapfile': ingredient.StringField()
    }

    def go(self):
        """
        Steps:
        
        1. Load and validate the input datamaps
        2. Run the node parts of the recipe  
        3. Validate node output and format the recipe output   
        """
        super(imager_finalize, self).go()
        # *********************************************************************
        # 1. Load the datamaps
        awimager_output_map = DataMap.load(
                                self.inputs["awimager_output_map"])
        raw_ms_per_image_map = DataMap.load(
                                    self.inputs["raw_ms_per_image_map"])
        sourcelist_map = DataMap.load(self.inputs["sourcelist_map"])
        sourcedb_map = DataMap.load(self.inputs["sourcedb_map"])
        target_mapfile = DataMap.load(self.inputs["target_mapfile"])
        output_image_mapfile = DataMap.load(
                                    self.inputs["output_image_mapfile"])
        processed_ms_dir = self.inputs["processed_ms_dir"]
        fillrootimagegroup_exec = self.inputs["fillrootimagegroup_exec"]

        # Align the skip fields
        align_data_maps(awimager_output_map, raw_ms_per_image_map,
                sourcelist_map, target_mapfile, output_image_mapfile,
                sourcedb_map)

        # Set the correct iterator
        sourcelist_map.iterator = awimager_output_map.iterator = \
            raw_ms_per_image_map.iterator = target_mapfile.iterator = \
            output_image_mapfile.iterator = sourcedb_map.iterator = \
                DataMap.SkipIterator

        # *********************************************************************
        # 2. Run the node side of the recupe
        command = " python %s" % (self.__file__.replace("master", "nodes"))
        jobs = []
        for  (awimager_output_item, raw_ms_per_image_item, sourcelist_item,
              target_item, output_image_item, sourcedb_item) in zip(
                  awimager_output_map, raw_ms_per_image_map, sourcelist_map,
                  target_mapfile, output_image_mapfile, sourcedb_map):
            # collect the files as argument
            arguments = [awimager_output_item.file,
                         raw_ms_per_image_item.file,
                         sourcelist_item.file,
                         target_item.file,
                         output_image_item.file,
                         self.inputs["minbaseline"],
                         self.inputs["maxbaseline"],
                         processed_ms_dir,
                         fillrootimagegroup_exec,
                         self.environment,
                         sourcedb_item.file]

            self.logger.info(
                "Starting finalize with the folowing args: {0}".format(
                                                                    arguments))
            jobs.append(ComputeJob(target_item.host, command, arguments))

        self._schedule_jobs(jobs)

        # *********************************************************************
        # 3. Validate the performance of the node script and assign output
        succesful_run = False
        for (job, output_image_item) in  zip(jobs, output_image_mapfile):
            if not "hdf5" in job.results:
                # If the output failed set the skip to True
                output_image_item.skip = True
            else:
                succesful_run = True
                # signal that we have at least a single run finished ok.
                # No need to set skip in this case

        if not succesful_run:
            self.logger.warn("Failed finalizer node run detected")
            return 1

        output_image_mapfile.save(self.inputs['placed_image_mapfile'])
        self.logger.debug(
           "Wrote mapfile containing placed hdf5 images: {0}".format(
                           self.inputs['placed_image_mapfile']))
        self.outputs["placed_image_mapfile"] = self.inputs[
                                                    'placed_image_mapfile']

        return 0


if __name__ == '__main__':
    sys.exit(imager_finalize().main())
