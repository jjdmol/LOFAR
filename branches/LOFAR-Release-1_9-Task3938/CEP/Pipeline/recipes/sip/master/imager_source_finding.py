from __future__ import with_statement
import os
import sys
import copy

from lofarpipe.support.baserecipe import BaseRecipe
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.data_map import DataMap


class imager_source_finding(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Master side of imager_source_finder. Collects arguments from command line
    and pipeline inputs. (for the implementation details see node):
    
    1. load mapfiles with input images and collect some parameters from
       The input ingredients.
    2. Call the node recipe.
    3. Validate performance of the node recipe and construct output value.   
    
    **CommandLine Arguments**
    
    A mapfile containing (node, image_path) pairs. The image to look for sources
    in.    
    """
    inputs = {
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
            help="Target path for the sourcedb created based on the"
                 " found sources"
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
        help="Full path of mapfile; containing the succesfull generated"
             "sourcedbs"
            )
    }

    def go(self):
        """
        """
        super(imager_source_finding, self).go()
        self.logger.info("Starting imager_source_finding run")
        # ********************************************************************
        # 1. load mapfiles with input images and collect some parameters from
        # The input ingredients       
        input_map = DataMap.load(self.inputs['args'][0])
        catalog_output_path = self.inputs["catalog_output_path"]

        # ********************************************************************
        # 2. Start the node script
        node_command = " python %s" % (self.__file__.replace("master", "nodes"))
        jobs = []

        input_map.iterator = DataMap.SkipIterator
        for item in input_map:
            arguments = [item.file,
                         self.inputs["bdsm_parset_file_run1"],
                         self.inputs["bdsm_parset_file_run2x"],
                         catalog_output_path,
                         os.path.join(
                             self.inputs["working_directory"],
                             "bdsm_output.img"),
                         self.inputs['sourcedb_target_path'],
                         self.environment,
                         self.inputs['working_directory'],
                         self.inputs['makesourcedb_path']
                        ]

            jobs.append(ComputeJob(item.host, node_command, arguments))

        # Hand over the job(s) to the pipeline scheduler
        self._schedule_jobs(jobs)

        # ********************************************************************
        # 3. Test for errors and return output
        if self.error.isSet():
            self.logger.warn("Failed imager_source_finding run detected")

        # Collect the nodes that succeeded
        source_dbs_from_nodes = copy.deepcopy(input_map)
        catalog_output_path_from_nodes = copy.deepcopy(input_map)
        source_dbs_from_nodes.iterator = \
            catalog_output_path_from_nodes.iterator = DataMap.SkipIterator

        for job, sourcedb_item, catalog_item in zip(jobs,
                                   source_dbs_from_nodes,
                                   catalog_output_path_from_nodes):

            if "source_db"  in job.results:
                succesfull_job = True
                sourcedb_item.file = job.results["source_db"]
                catalog_item.file = job.results["catalog_output_path"]
            else:
                sourcedb_item.file = "failed"
                sourcedb_item.skip = True
                catalog_item.file = "failed"
                catalog_item.skip = True
                # We now also have catalog path

        # Abort if none of the recipes succeeded
        if not succesfull_job:
            self.logger.error("None of the source finding recipes succeeded")
            self.logger.error("Exiting with a failure status")
            return 1

        self._store_data_map(self.inputs['mapfile'],
                 catalog_output_path_from_nodes,
                "datamap with created sourcelists")
        self._store_data_map(self.inputs['sourcedb_map_path'],
                source_dbs_from_nodes,
                 " datamap with created sourcedbs")

        self.outputs["mapfile"] = self.inputs['mapfile']
        self.outputs["sourcedb_map_path"] = self.inputs['sourcedb_map_path']

if __name__ == '__main__':
    sys.exit(imager_source_finding().main())

