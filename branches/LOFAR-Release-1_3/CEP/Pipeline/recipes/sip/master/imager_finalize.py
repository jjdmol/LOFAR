
from __future__ import with_statement
import sys

import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.utilities import create_directory
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.group_data import load_data_map, validate_data_maps, store_data_map
from lofarpipe.support.pipelinelogging import log_process_output

class imager_finalize(BaseRecipe, RemoteCommandRecipeMixIn):
    """

    """
    inputs = {
        'initscript': ingredient.FileField(
            '--initscript',
            help='''The full path to an (Bourne) shell script which will\
             intialise the environment (ie, ``lofarinit.sh``)'''
        ),
        'awimager_output_map': ingredient.FileField(
            '--awimager-output-mapfile',
            help=""""Mapfile containing (host, path) pairs of created sky
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
        'target_mapfile': ingredient.FileField(
            '--target-mapfile',
            help='''Mapfile containing (host, path) pairs to the concatenated and
            combined measurement set, the source for the actual sky image'''
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
            help='''location of mapfile with proced and correctly placed, hdf5 images'''
        )
    }

    outputs = {
        'placed_image_mapfile': ingredient.StringField()
    }

    def go(self):
        super(imager_finalize, self).go()

        awimager_output_map = load_data_map(self.inputs["awimager_output_map"])
        raw_ms_per_image_map = load_data_map(self.inputs["raw_ms_per_image_map"])
        sourcelist_map = load_data_map(self.inputs["sourcelist_map"])
        target_mapfile = load_data_map(self.inputs["target_mapfile"])
        output_image_mapfile = load_data_map(self.inputs["output_image_mapfile"])
        processed_ms_dir = self.inputs["processed_ms_dir"]
        fillRootImageGroup_exec = self.inputs["fillrootimagegroup_exec"]
        # The input mapfiles might nog be of the same length:
        # host_source are unique and can be used to match the entries!
        # Final step is the source_finder: use this mapfile as 'source'
        awimager_output_map_new = []
        raw_ms_per_image_map_new = []
        target_map_new = []
        output_image_map_new = []
        for host_source, path_source in sourcelist_map:
            for host_comp, path_comp in awimager_output_map:
                if host_comp == host_source:
                    awimager_output_map_new.append((host_comp, path_comp))

            for host_comp, path_comp in raw_ms_per_image_map:
                if host_comp == host_source:
                    raw_ms_per_image_map_new.append((host_comp, path_comp))

            for host_comp, path_comp in target_mapfile:
                if host_comp == host_source:
                    target_map_new.append((host_comp, path_comp))

            for host_comp, path_comp in output_image_mapfile:
                if host_comp == host_source:
                    output_image_map_new.append((host_comp, path_comp))

        # chech validity of the maps: all on same node with the same length
        if not validate_data_maps(awimager_output_map_new, raw_ms_per_image_map_new,
                sourcelist_map, target_map_new, output_image_map_new):
            self.logger.error("The suplied datamaps for the imager_finalize"
                              "are incorrect.")
            self.logger.error("awimager_output_map: {0}".format(
                                                        awimager_output_map_new))
            self.logger.error("raw_ms_per_image_map: {0}".format(
                                                        raw_ms_per_image_map_new))
            self.logger.error("sourcelist_map: {0}".format(
                                                        sourcelist_map))
            self.logger.error("target_mapfile: {0}".format(
                                                        target_map_new))
            self.logger.error("output_image_mapfile: {0}".format(
                                                        output_image_map_new))
            return 1

        nodeCommand = " python %s" % (self.__file__.replace("master", "nodes"))
        jobs = []
        for  (awimager_output_pair, raw_ms_per_image_pair, sourcelist_pair,
              target_pair, output_image_pair) in zip(
                awimager_output_map_new, raw_ms_per_image_map_new, sourcelist_map,
                target_map_new, output_image_map_new):
            # collect the data
            (host, awimager_output) = awimager_output_pair
            (host, raw_ms_per_image) = raw_ms_per_image_pair
            (host, sourcelist) = sourcelist_pair
            (host, target) = target_pair
            (host, output_image) = output_image_pair

            arguments = [awimager_output, raw_ms_per_image, sourcelist,
                        target, output_image, self.inputs["minbaseline"],
                        self.inputs["maxbaseline"], processed_ms_dir,
                        fillRootImageGroup_exec]
            self.logger.info(arguments)
            jobs.append(ComputeJob(host, nodeCommand, arguments))
        self._schedule_jobs(jobs)

        placed_image_ = []
        for job in  jobs:
            if job.results.has_key("hdf5"):
                placed_image_.append((job.host, job.results["image"]))

        if self.error.isSet():
            self.logger.warn("Failed finalizer node run detected")
            return 1

        store_data_map(self.inputs['placed_image_mapfile'], placed_image_)
        self.logger.debug("Wrote mapfile containing placed hdf5 images: {0}".format(
                           self.inputs['placed_image_mapfile']))
        self.outputs["placed_image_mapfile"] = self.inputs['placed_image_mapfile']

        return 0


if __name__ == '__main__':
    sys.exit(imager_finalize().main())
