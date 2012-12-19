#                                                        LOFAR IMAGING PIPELINE
#
#                                 Master recipe for copying files between nodes
#
#                                                            Wouter Kijn, 2012
#                                                               klijn@astron.nl
#------------------------------------------------------------------------------

import os
import sys

import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.data_map import DataMap

class MasterNodeInterface(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Base class for master script collecting functionality regarding master node
    communication in a single interface.

    This class contains basic functionality with a number of methods that are
    meant to be reimplemented in the derived class:
    on_failure(this) :  Called when all node recipes returned with a non-zero
                        exit status.
    on_error(this)   :  Called when some node recipes returned with a non-zero
                        exit status.
    on_success(this) :  Called when all node recipes returned with a zero
                        exit status.
    TODO: Suggested improvements
    on_partial_succes(this):   To prepare for rerun of partial runs
    on_warn(this):          To distinguish between
    """
    def __init__(self, command):
        """
        constructor, expects a string command used for calling the node script
        This class cannot be created with the base constructor. Inheriting
        should call this constructor with an command string
        """
        self.logger = None
        self._command = command
        self._jobs = []
        # call possible baseclass constructors
        super(MasterNodeInterface, self).__init__()

    def append_job(self, host, arguments):
        """
        append_job adds a job to the current job list. It expects the host,
        a list of arguments.
        """
        compute_job = ComputeJob(host, self._command, arguments)
        self._jobs.append(compute_job)

    def run_jobs(self):
        """
        Starts the set of tasks in the job lists. If all jobs succeed, 
        on_success() will be called. If some jobs fail, on_error() will be
        called. If all jobs fail, on_failure() will be called.
        An log message is displayed on the stdout or in a logger if the object
        contains one.
        """
        log_message = "Start scheduling jobs with command {0}".format(
                                                                self._command)
        if self.logger:
            self.logger.info(log_message)
        else:
            print log_message

        self._schedule_jobs(self._jobs)

        if self.error.isSet():
            if all(job.results['returncode'] != 0 for job in self._jobs):
                return self.on_failure()
            else:
                return self.on_error()
        else:
            return self.on_succes()

    def on_failure(self):
        """
        This method is called when all node recipes return with a non-zero
        exit status. It should return a value that can be cast to an integer,
        where 0 indicates success. The default behaviour is to return -1.
        This method can be overridden in the derived class.
        """
        return -1

    def on_error(self):
        """
        This method is called when some node recipes return with a non-zero
        exit status. It should return a value that can be cast to an integer,
        where 0 indicates success. The default behaviour is to return 1.
        This method can be overridden in the derived class.
        """
        return 1

    def on_succes(self):
        """
        This method is called when all node recipes return with a zero exit
        status. It should return a value that can be cast to an integer,
        where 0 indicates success. The default behaviour is to return 0.
        This method can be overridden in the derived class.
        """
        return 0


class copier(MasterNodeInterface):
    """
    The copier recipe is used to copy paths provided in the source mapfile
    to the same node as 'matched' list provided in the target mapfile.
    The primairy use is to collect data on computation nodes, which nodes is
    sometime only specified in the mapfiles.
    There are Two operations performed by this script
    1. COPY the source path to the parent directory of
    the path provided in the target mapfilem eg: Copy instrument tables
    next to the measurement sets on which they will applied.
    To Use this operation set target_dir to "" or do not specify it
    2. COLLECT information from source nodes to a central path on different
    nodes specified in the target mapfile. eg Copy instrument tables from
    the node they are produces to the same node as the measurement sets
    privided in the target file BUT place them all in the same dir. Provide
    a target_dir for this operation: all paths not starting with /, not
    absolute     will be placed in a dir with this name relative to the
    working dir.

    **Arguments**

    A mapfile describing the data to be processed.
    """
    inputs = {
        'mapfile_source': ingredient.StringField(
            '--mapfile-source',
            help="Full path of mapfile of node:path pairs of source dataset"
        ),
        'mapfile_target': ingredient.StringField(
            '--mapfile-target',
            help="Full path of mapfile of node:path pairs of target location"
        ),
        'allow_rename': ingredient.BoolField(
            '--allow-rename',
            default=True,
            help="Allow renaming of basename at target location"
        ),
        'mapfile': ingredient.StringField(
            '--mapfile',
            help="full path to mapfile containing copied paths"
        ),
    }

    outputs = {
        'mapfile_target_copied': ingredient.StringField(
            help="Path to mapfile containing all the succesfull copied"
            "target files")
    }

    def __init__(self):
        """
        Constructor sets the python command used to call node scripts
        """
        super(copier, self).__init__(
            "python {0}".format(self.__file__.replace('master', 'nodes')))
        self.source_map = DataMap()
        self.target_map = DataMap()

    def _validate_mapfiles(self, allow_rename=False):
        """
        Validation of input source and target map files. They must have equal
        length. Furthermore, if rename is not allowed, test that 'file names'
        are the same.
        """
        # Same length? If not, then fail
        if len(self.source_map) != len(self.target_map):
            self.logger.error("Number of entries in the source and target map"
                "Is not the same: \n target \n {0}\n source \n {1}".format(
                            self.target_map, self.source_map))
            return False

        for source, target in zip(self.source_map, self.target_map):
            # skip strict checking of basename equality if rename is allowed
            if not allow_rename:
                target_name = os.path.basename(target.file)
                source_name = os.path.basename(source.file)
                if not (target_name == source_name):
                    self.logger.error("One of the suplied source target pairs"
                        "contains a different 'filename': {0} != {1}\n"
                        " aborting".format(target_name, source_name))
                    return False

        return True

    def _write_mapfile(self):
        """
        Write an (updated) mapfile.
        """
        self.logger.debug("Writing mapfile: %s" % self.inputs['mapfile'])
        self.target_map.save(self.inputs['mapfile'])
        self.outputs['mapfile_target_copied'] = self.inputs['mapfile']

    def on_failure(self):
        """
        All copier jobs failed. Bailing out.
        """
        self.logger.error("All copier jobs failed. Bailing out!")
        return 1

    def on_error(self):
        """
        Some copier jobs failed. Update the target map, setting 'skip' to True
        for failed runs, and save it.
        """
        self.logger.warn(
            "Some copier jobs failed, continuing with succeeded runs"
        )
        for job, target in zip(self._jobs, self.target_map):
            if job.results['returncode'] != 0:
                target.skip = True
        self._write_mapfile()
        return 0

    def on_succes(self):
        """
        All copier jobs succeeded. Save an updated mapfile.
        """
        self.logger.info("All copier jobs succeeded")
        self._write_mapfile()
        return 0

    def go(self):
        super(copier, self).go()
        self.logger.info("Starting copier go")

        # Load data from mapfiles
        self.source_map = DataMap.load(self.inputs['mapfile_source'])
        self.target_map = DataMap.load(self.inputs['mapfile_target'])

        # validate data in mapfiles
        if not self._validate_mapfiles(self.inputs['allow_rename']):
            return 1

        # Run the compute nodes with the node specific mapfiles
        for source, target in zip(self.source_map, self.target_map):
            args = [source.host, source.file, target.file]
            self.append_job(target.host, args)

        # start the jobs, return the exit status.
        return self.run_jobs()

if __name__ == '__main__':

    sys.exit(copier().main())
