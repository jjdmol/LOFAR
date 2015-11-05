#                                                         LOFAR IMAGING PIPELINE
#
#                                                    get_metadata: master script
#                                                             Marcel Loose: 2012
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------
import sys
import os
import time
import xml.dom.minidom as _xml

import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.data_map import DataMap
from lofarpipe.recipes.helpers import metadata
from lofar.parameterset import parameterset
from lofarpipe.support.utilities import create_directory

class get_metadata(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Get the metadata from the given data products and return them as a LOFAR
    parameterset.
    
    1. Parse and validate inputs
    2. Load mapfiles
    3. call node side of the recipe
    4. validate performance
    5. Create the parset-file 
    6. if this a metaInformation file create this information
    7. and write it to disk.  
    
    **Command line arguments**

    A mapfile describing the data to be processed.
    """
    inputs = {
        'product_type': ingredient.StringField(
            '--product-type',
            help = "Data product type",
        ),
        'parset_file': ingredient.StringField(
            '--parset-file',
            help = "Path to the output parset file"
        ),
        'parset_prefix': ingredient.StringField(
            '--parset-prefix',
            help = "Prefix for each key in the output parset file",
            default = ''
        ),
        'xml_log':ingredient.StringField(
        '--xml-log',
        help = "(optional) xml encoded string containing the default pipeline-xml log",
        default = '')

    }

    outputs = {
    }

    # List of valid data product types.
    valid_product_types = ["Correlated",
                           "InstrumentModel",
                           "SkyImage",
                           "PipelineMeta"]


    def go(self):
        super(get_metadata, self).go()
        # ********************************************************************
        # 1. Parse and validate inputs
        args = self.inputs['args']
        product_type = self.inputs['product_type']
        global_prefix = self.inputs['parset_prefix']
        xml_log_string = self.inputs['xml_log']

        # Add a trailing dot (.) if not present in the prefix.
        if global_prefix and not global_prefix.endswith('.'):
            global_prefix += '.'

        if not product_type in self.valid_product_types:
            self.logger.error(
                "Unknown product type: %s\n\tValid product types are: %s" %
                (product_type, ', '.join(self.valid_product_types))
        )

        if not product_type == "PipelineMeta":
            # ********************************************************************
            # 2. Load mapfiles
            self.logger.debug("Loading input-data mapfile: %s" % args[0])
            data = DataMap.load(args[0])

            # ********************************************************************
            # 3. call node side of the recipe
            command = "python %s" % (self.__file__.replace('master', 'nodes'))
            data.iterator = DataMap.SkipIterator
            jobs = []
            for inp in data:
                jobs.append(
                    ComputeJob(
                        inp.host, command,
                        arguments = [
                            inp.file,
                            self.inputs['product_type']
                        ]
                    )
                )
            self._schedule_jobs(jobs)
            for job, inp in zip(jobs, data):
                if job.results['returncode'] != 0:
                    inp.skip = True

            # ********************************************************************
            # 4. validate performance
            # 4. Check job results, and create output data map file
            if self.error.isSet():
                # Abort if all jobs failed
                if all(job.results['returncode'] != 0 for job in jobs):
                    self.logger.error("All jobs failed. Bailing out!")
                    return 1
                else:
                    self.logger.warn(
                        "Some jobs failed, continuing with succeeded runs"
                    )
            self.logger.debug("Updating data map file: %s" % args[0])
            data.save(args[0])

            # ********************************************************************
            # 5. Create the parset-file
            parset = parameterset()
            prefix = "Output_%s_" % product_type
            parset.replace('%snrOf%s' % (global_prefix, prefix), str(len(jobs)))
            prefix = global_prefix + prefix
            for idx, job in enumerate(jobs):
                self.logger.debug("job[%d].results = %s" % (idx, job.results))
                parset.adoptCollection(
                    metadata.to_parset(job.results), '%s[%d].' % (prefix, idx)
                )

        # ********************************************************************
        # 6. if product_type = PipelineMeta
        # We only need to do master level stuf no access to the indiv. nodes
        else:
            parset = parameterset()

            # If there is an xml log
            if xml_log_string != '':
                xml_node = _xml.parseString(xml_log_string)

                # ************************************************************
                # Get the start time from the lognode. Could use named child
                # but this is a pipeline log file
                start_time = float(
                    xml_node.firstChild.getAttribute("time_info_start"))
                end_time = time.time()
                duration = end_time - start_time
                parset.adoptCollection(
                metadata.to_parset({global_prefix + 'duration':str(duration)})
                    )

                #**************************************************************
                # Demixer information
                # demixed_sources_meta_information
                demix_meta_node = xml_node.getElementsByTagName(
                   "demixed_sources_meta_information").item(0)  # Get the node
                                        # there should only be one node
                if demix_meta_node != None:
                    demix_meta_dict = {}

                # extract the information
                    demix_meta_dict[global_prefix + 'demixer.' + 'modelsources'] = \
                        demix_meta_node.getAttribute("modelsources")

                    demix_meta_dict[global_prefix + 'demixer.' + 'othersources'] = \
                        demix_meta_node.getAttribute("othersources")

                    demix_meta_dict[global_prefix + 'demixer.' + 'subtractsources'] = \
                        demix_meta_node.getAttribute("subtractsources")

                # add to the parset
                    parset.adoptCollection(metadata.to_parset(demix_meta_dict))

                # *************************************************************
                # Skymodel information
                skymodel_node = xml_node.getElementsByTagName(
                   "skymodel_meta_information").item(0)  # Get the node
                                        # there should only be one node
                if skymodel_node != None:

                    skymodel_dict = {}

                # extract the information
                    skymodel_dict[global_prefix + 'skymodel.' + 'path'] = \
                        skymodel_node.getAttribute("skymodel")

                    skymodel_dict[global_prefix + 'demixer.' + 'userSupplied'] = \
                        skymodel_node.getAttribute("userSupplied")

                # add to the parset
                    parset.adoptCollection(metadata.to_parset(skymodel_dict))
            else:
                self.logger.error("No xml log supplied, this is needed for " +
                                  "Pipeline meta information")


        # ********************************************************************
        # 7. Write the parset to disk
        try:
            create_directory(os.path.dirname(self.inputs['parset_file']))
            parset.writeFile(self.inputs['parset_file'])
        except RuntimeError, err:
            self.logger.error("Failed to write meta-data: %s" % str(err))
            return 1
        return 0


if __name__ == '__main__':
    sys.exit(get_metadata().main())
