from __future__ import with_statement
import collections

from lofarpipe.support.baserecipe import BaseRecipe
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn

class imager_source_finding(BaseRecipe, RemoteCommandRecipeMixIn):
    inputs = {
        'job': ingredient.StringField(
            '--job',
            help = "Job name: used for storing  (intermediate) data products"
        ),
        'initscript': ingredient.FileField(
            '--initscript',
            help = "Initscript to source (ie, lofarinit.sh)"
        ),
        'bdsm_parset_file_run1': ingredient.FileField(
            '--bdsm-parset-file-run1',
            help = "Path to bdsm parameter set for the first sourcefinding run"
        ),
        'bdsm_parset_file_run2x': ingredient.FileField(
            '--bdsm-parset-file-run2x',
            help = "Path to bdsm parameter set for the second and later" \
                   " sourcefinding runs"
        ),
        'catalog_output_path': ingredient.FileField(
            '--catalog-output-path',
            help = "Path to write the catalog created by bdsm)"
        ),
    }


    def go(self):
        self.logger.info("Starting imager_source_finding run")
        super(imager_source_finding, self).go()
        outnames = collections.defaultdict(list)

        input_map = [("locus040", "/data/scratch/klijn/TestImage.restored")]


        bdsm_parameter_run1_path = "/home/klijn/build/preparation/bdsm_parameters.map"
        bdsm_parameter_run2x_path = "/home/klijn/build/preparation/bdsm_parameters_2x.map"
        catalog_output_path = "/data/scratch/klijn/bdsm_output_cat"

        # TODO FIXME: This output path will be, in the testing phase a 
        # subdirectory of the actual output image.
        # This is the cropped image!!!
        image_output_path = "/data/scratch/klijn/bdsm_output.img" #This is not a



        node_command = " python %s" % (self.__file__.replace("master", "nodes"))
        jobs = []
        for host, data in input_map:
            #construct and save the output name
            input_image = "/data/scratch/klijn/TestImage.restored.cropped" # data
            outnames[host].append(data)
            arguments = [data,
                         bdsm_parameter_run1_path,
                         bdsm_parameter_run2x_path,
                         catalog_output_path,
                         image_output_path
                        ]
            jobs.append(ComputeJob(host, node_command, arguments))

        # Hand over the job(s) to the pipeline scheduler
        self._schedule_jobs(jobs)


        # Test for errors
        if self.error.isSet():
            self.logger.warn("Failed ImagerCreateDBs run detected")
            return 1
        else:
            return 0



if __name__ == '__main__':
    sys.exit(imager_source_finding().main())
