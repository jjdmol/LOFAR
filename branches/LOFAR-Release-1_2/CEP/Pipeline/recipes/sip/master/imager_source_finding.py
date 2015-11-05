from __future__ import with_statement
import os
import sys

from lofarpipe.support.baserecipe import BaseRecipe
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.group_data import load_data_map, store_data_map

class imager_source_finding(BaseRecipe, RemoteCommandRecipeMixIn):
    inputs = {
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
        'catalog_output_path': ingredient.StringField(
            '--catalog-output-path',
            help = "Path to write the catalog created by bdsm)"
        ),
        'mapfile': ingredient.StringField(
            '--mapfile',
            help = "Full path of mapfile; containing the succesfull generated"
            "source list"
        ),
        'working_directory': ingredient.StringField(
            '--working-directory',
            help = "Working directory used by the nodes: local data"
        ),
        'sourcedb_target_path': ingredient.StringField(
            '--sourcedb-target-path',
            help = "Target path for the sourcedb created based on the found sources"
        ),
        'makesourcedb_path': ingredient.ExecField(
             '--makesourcedb-path',
             help = "Path to makesourcedb executable."
        ),
        'sourcedb_map_path': ingredient.StringField(
            '--sourcedb-map-path',
            help = "Full path of mapfile; containing the succesfull generated"
            "sourcedbs"
        ),

    }

    outputs = {
        'mapfile': ingredient.StringField(
        help = "Full path of mapfile; containing the succesfull generated"
            ),
        'sourcedb_map_path': ingredient.StringField(
        help = "Full path of mapfile; containing the succesfull generated sourcedbs"
            )
    }

    def go(self):
        self.logger.info("Starting imager_source_finding run")
        super(imager_source_finding, self).go()

        input_map = load_data_map(self.inputs['args'][0])

        bdsm_parset_file_run1 = self.inputs["bdsm_parset_file_run1"]
        bdsm_parset_file_run2x = self.inputs["bdsm_parset_file_run2x"]
        catalog_output_path = self.inputs["catalog_output_path"]

        # TODO FIXME: This output path will be, in the testing phase a 
        # subdirectory of the actual output image.
        # This might be a bug: dunno
        image_output_path = os.path.join(
            self.inputs["working_directory"], "bdsm_output.img"
        )
        node_command = " python %s" % (self.__file__.replace("master", "nodes"))
        jobs = []
        created_sourcelists = []
        created_sourcedbs = []
        for host, data in input_map:
            arguments = [data,
                         bdsm_parset_file_run1,
                         bdsm_parset_file_run2x,
                         catalog_output_path,
                         image_output_path,
                         self.inputs['sourcedb_target_path'],
                         self.inputs['initscript'],
                         self.inputs['working_directory'],
                         self.inputs['makesourcedb_path']
                        ]
            created_sourcelists.append((host, catalog_output_path))
            created_sourcedbs.append((host, self.inputs['sourcedb_target_path']))
            jobs.append(ComputeJob(host, node_command, arguments))

        # Hand over the job(s) to the pipeline scheduler
        self._schedule_jobs(jobs)

        # Test for errors
        if self.error.isSet():
            self.logger.warn("Failed imager_source_finding run detected")
            return 1

        self.logger.info(created_sourcelists)
        store_data_map(self.inputs['mapfile'], created_sourcelists)
        self.logger.debug("Wrote datamap with created sourcelists: {0}".format(
                                                self.inputs['mapfile']))

        store_data_map(self.inputs['sourcedb_map_path'], created_sourcedbs)
        self.logger.debug("Wrote datamap with created sourcedbs: {0}".format(
                                             self.inputs['sourcedb_map_path']))

        self.outputs["mapfile"] = self.inputs['mapfile']
        self.outputs["sourcedb_map_path"] = self.inputs['sourcedb_map_path']

if __name__ == '__main__':
    sys.exit(imager_source_finding().main())
