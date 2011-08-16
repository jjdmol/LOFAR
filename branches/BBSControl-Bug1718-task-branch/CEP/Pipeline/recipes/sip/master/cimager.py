#                                                         LOFAR IMAGING PIPELINE
#
#                                                  cimager (ASKAP imager) recipe
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

from __future__ import with_statement
from contextlib import contextmanager

import os
import sys
import time
import threading
import collections
import subprocess
import tempfile
import signal

from pyrap.quanta import quantity

import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.pipelinelogging import log_time, log_process_output
from lofarpipe.support.clusterlogger import clusterlogger
from lofarpipe.support.pipelinelogging import log_process_output
from lofarpipe.support.remotecommand import ProcessLimiter
from lofarpipe.support.remotecommand import run_remote_command
from lofarpipe.support.remotecommand import threadwatcher
from lofarpipe.support.parset import Parset
from lofarpipe.support.parset import get_parset
from lofarpipe.support.parset import patched_parset, patch_parset
from lofarpipe.support.utilities import spawn_process
from lofarpipe.support.lofarexceptions import PipelineException
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.lofaringredient import LOFARoutput, LOFARinput

class ParsetTypeField(ingredient.StringField):
    """
    Input field which accepts the string values either "cimager" or
    "mwimager". Enables specification of type of parset supplied to the
    cimager recipe.
    """
    def is_valid(self, value):
        if value == "cimager" or value == "mwimager":
            return True
        else:
            return False


class cimager(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Provides a convenient, pipeline-based mechanism of running the cimager on
    a dataset.

    Can ingest either an MWimager-style parset, converting to cimager format
    as required, or a cimager parset directly.

    **Arguments**

    A mapfile describing the data to be processed.
    """
    inputs = {
        'imager_exec': ingredient.ExecField(
            '--imager-exec',
            help="cimager executable"
        ),
        'convert_exec': ingredient.ExecField(
            '--convert-exec',
            help="convertimagerparset executable"
        ),
        'parset': ingredient.FileField(
            '--parset',
            help="Imager configuration parset (mwimager or cimager format)"
        ),
        'nproc': ingredient.IntField(
            '--nproc',
            help="Maximum number of simultaneous processes per compute node",
            default=8
        ),
        'timestep': ingredient.FloatField(
            '--timestep',
            help="If non-zero, multiple images will be made, each using timestep seconds of data",
            default=0.0
        ),
        'results_dir': ingredient.DirectoryField(
            '--results-dir',
            help="Directory in which resulting images will be placed",
        ),
        'parset_type': ParsetTypeField(
            '--parset-type',
            default="mwimager",
            help="cimager or mwimager"
        ),
        'makevds': ingredient.ExecField(
            '--makevds',
            help="makevds executable",
            default="/opt/LofIm/daily/lofar/bin/makevds"
        ),
        'combinevds': ingredient.ExecField(
            '--comebinevds',
            help="combinevds executable",
            default="/opt/LofIm/daily/lofar/bin/combinevds"
        )
    }

    outputs = {
        'images': ingredient.ListField()
    }

    def go(self):
        self.logger.info("Starting cimager run")
        super(cimager, self).go()
        self.outputs['images' ] = []

        #              Build a GVDS file describing all the data to be processed
        # ----------------------------------------------------------------------
        self.logger.debug("Building VDS file describing all data for cimager")
        gvds_file = os.path.join(
            self.config.get("layout", "job_directory"),
            "vds",
            "cimager.gvds"
        )
        inputs = LOFARinput(self.inputs)
        inputs['args'] = self.inputs['args']
        inputs['gvds'] = gvds_file
        inputs['unlink'] = False
        inputs['makevds'] = self.inputs['makevds']
        inputs['combinevds'] = self.inputs['combinevds']
        inputs['nproc'] = self.inputs['nproc']
        inputs['directory'] = os.path.dirname(gvds_file)
        outputs = LOFARoutput(self.inputs)
        if self.cook_recipe('new_vdsmaker', inputs, outputs):
            self.logger.warn("new_vdsmaker reports failure")
            return 1
        self.logger.debug("cimager GVDS is %s" % (gvds_file,))

        #                            Read data for processing from the GVDS file
        # ----------------------------------------------------------------------
        parset = Parset(gvds_file)

        data = []
        for part in range(parset.getInt('NParts')):
            host = parset.getString("Part%d.FileSys" % part).split(":")[0]
            vds  = parset.getString("Part%d.Name" % part)
            data.append((host, vds))

        #                                 Divide data into timesteps for imaging
        #          timesteps is a list of (start, end, results directory) tuples
        # ----------------------------------------------------------------------
        timesteps = []
        results_dir = self.inputs['results_dir']
        if self.inputs['timestep'] == 0:
            self.logger.info("No timestep specified; imaging all data")
            timesteps = [(None, None, results_dir)]
        else:
            self.logger.info("Using timestep of %s s" % self.inputs['timestep'])
            gvds = get_parset(gvds_file)
            start_time = quantity(gvds['StartTime'].get()).get('s').get_value()
            end_time = quantity(gvds['EndTime'].get()).get('s').get_value()
            step = float(self.inputs['timestep'])
            while start_time < end_time:
                timesteps.append(
                    (
                        start_time, start_time+step,
                        os.path.join(results_dir, str(start_time))
                    )
                )
                start_time += step

        #                          Run each cimager process in a separate thread
        # ----------------------------------------------------------------------
        command = "python %s" % (self.__file__.replace('master', 'nodes'))
        for label, timestep in enumerate(timesteps):
            self.logger.info("Processing timestep %d" % label)
            jobs = []
            parsets = []
            start_time, end_time, resultsdir = timestep
            for host, vds in data:
                vds_data = Parset(vds)
                frequency_range = [
                    vds_data.getDoubleVector("StartFreqs")[0],
                    vds_data.getDoubleVector("EndFreqs")[-1]
                ]
                parsets.append(
                    self.__get_parset(
                        os.path.basename(vds_data.getString('FileName')).split('.')[0],
                        vds_data.getString("FileName"),
                        str(frequency_range),
                        vds_data.getString("Extra.FieldDirectionType"),
                        vds_data.getStringVector("Extra.FieldDirectionRa")[0],
                        vds_data.getStringVector("Extra.FieldDirectionDec")[0],
                        'True', # cimager bug: non-restored image unusable
                    )
                )
                jobs.append(
                    ComputeJob(
                        host, command,
                        arguments=[
                            self.inputs['imager_exec'],
                            vds,
                            parsets[-1],
                            resultsdir,
                            start_time,
                            end_time
                        ]
                    )
                )
            self._schedule_jobs(jobs, max_per_node=self.inputs['nproc'])
            for parset in parsets:
                parset = Parset(parset)
                image_names = parset.getStringVector("Cimager.Images.Names")
                self.outputs['images'].extend(image_names)
            [os.unlink(parset) for parset in parsets]

        #                Check if we recorded a failing process before returning
        # ----------------------------------------------------------------------
        if self.error.isSet():
            self.logger.warn("Failed imager process detected")
            return 1
        else:
            return 0

    def __get_parset(
        self, name, dataset, frequency, ms_dir_type,
        ms_dir_ra, ms_dir_dec, restore
    ):
        def convert_mwimager_parset(parset):
            try:
                with patched_parset(
                    parset,
                    {
                        'dataset': dataset,
                        'Images.frequency': frequency,
                        'msDirType': ms_dir_type,
                        'msDirRa': ms_dir_ra,
                        'msDirDec': ms_dir_dec,
                        'restore': restore # cimager bug: non-restored image unusable
                    }
                ) as cimager_parset:
                    fd, converted_parset = tempfile.mkstemp(
                        dir=self.config.get("layout", "job_directory")
                    )
                    convert_process = spawn_process(
                        [
                            self.inputs['convert_exec'],
                            cimager_parset,
                            converted_parset
                        ],
                        self.logger
                    )
                    os.close(fd)
                    sout, serr = convert_process.communicate()
                    log_process_output(self.inputs['convert_exec'], sout, serr, self.logger)
                    if convert_process.returncode != 0:
                        raise subprocess.CalledProcessError(
                            convert_process.returncode, convert_exec
                        )
                    return converted_parset
            except OSError, e:
                self.logger.error("Failed to spawn convertimagerparset (%s)" % str(e))
                raise
            except subprocess.CalledProcessError, e:
                self.logger.error(str(e))
                raise

        def populate_cimager_parset(parset):
            input_parset = Parset(parset)
            patch_dictionary = {
                'Cimager.dataset': dataset,
                'Cimager.restore': restore
            }
            image_names = []
            for image_name in input_parset.getStringVector('Cimager.Images.Names'):
                image_names.append("%s_%s" % (image_name, name))
                subset = input_parset.makeSubset(
                    "Cimager.Images.%s" % image_name,
                    "Cimager.Images.%s" % image_names[-1]
                )
                patch_dictionary[
                    "Cimager.Images.%s.frequency" % image_names[-1]
                ] = frequency
                patch_dictionary[
                    "Cimager.Images.%s.direction" % image_names[-1]
                ] = "[ %s,%s,%s ]" % (ms_dir_ra, ms_dir_dec, ms_dir_type)
                for key in subset:
                    patch_dictionary[key] = subset[key].get()
            input_parset.subtractSubset('Cimager.Images.image')
            for key in input_parset:
                patch_dictionary[key] = input_parset[key].get()
            patch_dictionary['Cimager.Images.Names'] = "[ %s ]" % ", ".join(image_names)
            return patch_parset(
                None, patch_dictionary,
                self.config.get("layout", "job_directory")
            )

        try:
            if self.inputs['parset_type'] == "mwimager":
                cimager_parset = convert_mwimager_parset(self.inputs['parset'])
            elif self.inputs['parset_type'] == "cimager":
                cimager_parset = populate_cimager_parset(self.inputs['parset'])
        except Exception, e:
            self.logger.exception("Failed to generate imager parset")
            raise

        return cimager_parset

if __name__ == '__main__':
    sys.exit(cimager().main())
