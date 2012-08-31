from __future__ import with_statement
import os
import sys
import xml.dom.minidom as xml

from lofarpipe.support.baserecipe import BaseRecipe
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.group_data import load_data_map, store_data_map
from lofarpipe.support.pipelinexml import add_child, get_child
from lofarpipe.support.xmllogging import master_xml_decorator

class imager_source_finding(BaseRecipe, RemoteCommandRecipeMixIn):
    inputs = {
        'initscript': ingredient.FileField(
            '--initscript',
            help="Initscript to source (ie, lofarinit.sh)"
        ),
        'bdsm_parset_file_run1': ingredient.FileField(
            '--bdsm-parset-file-run1',
            help="Path to bdsm parameter set for the first sourcefinding run"
        ),
        'bdsm_parset_file_run2x': ingredient.FileField(
            '--bdsm-parset-file-run2x',
            help="Path to bdsm parameter set for the second and later" \
                   " sourcefinding runs"
        ),
        'catalog_output_path': ingredient.StringField(
            '--catalog-output-path',
            help="Path to write the catalog created by bdsm)"
        ),
        'mapfile': ingredient.StringField(
            '--mapfile',
            help="Full path of mapfile; containing the succesfull generated"
            "source list"
        ),
        'working_directory': ingredient.StringField(
            '--working-directory',
            help="Working directory used by the nodes: local data"
        ),
        'sourcedb_target_path': ingredient.StringField(
            '--sourcedb-target-path',
            help="Target path for the sourcedb created based on the found sources"
        ),
        'makesourcedb_path': ingredient.ExecField(
             '--makesourcedb-path',
             help="Path to makesourcedb executable."
        ),
        'sourcedb_map_path': ingredient.StringField(
            '--sourcedb-map-path',
            help="Full path of mapfile; containing the succesfull generated"
            "sourcedbs"
        ),

    }

    outputs = {
        'mapfile': ingredient.StringField(
        help="Full path of mapfile; containing the succesfull generated"
            ),
        'sourcedb_map_path': ingredient.StringField(
        help="Full path of mapfile; containing the succesfull generated sourcedbs"
            )
    }

    @master_xml_decorator
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
        node_command = "python %s" % (self.__file__.replace("master", "nodes"))

        jobs = []
        self.jobs = jobs
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

        # Collect the nodes that succeeded
        source_dbs_from_nodes = []
        catalog_output_path_from_nodes = []
        for job in jobs:
            if "source_db"  in job.results:
                source_dbs_from_nodes.append((
                                        job.host, job.results["source_db"]))
                # We now also have catalog path
                catalog_output_path_from_nodes.append((
                               job.host, job.results["catalog_output_path"]))

        # Abort if none of the recipes succeeded
        if len(source_dbs_from_nodes) == 0:
            self.logger.error("None of the source finding recipes succeeded")
            self.logger.error("Exiting with a failure status")
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

        return 0


if __name__ == '__main__':
    sys.exit(imager_source_finding().main())
