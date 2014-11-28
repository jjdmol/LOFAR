import os
import sys

from lofarpipe.support.parset import Parset
from lofarpipe.support.control import control

from lofarpipe.support.loggingdecorators import duration
from lofarpipe.support.data_map import DataMap, DataProduct, validate_data_maps
from lofarpipe.support.lofarexceptions import PipelineException
from lofarpipe.support.utilities import create_directory

import loader


class GenericPipeline(control):

    def __init__(self):
        control.__init__(self)
        self.parset = Parset()
        self.input_data = {}
        self.output_data = {}
        self.parset_feedback_file = None

    def usage(self):
        """
        Display usage
        """
        print >> sys.stderr, "Usage: %s [options] <parset-file>" % sys.argv[0]
        print >> sys.stderr, "Parset structure should look like:\n" \
                             "NYI"
        return 1

    def go(self):
        #"""
        #Read the parset-file that was given as input argument, and set the
        #jobname before calling the base-class's `go()` method.
        #"""
        try:
            parset_file = os.path.abspath(self.inputs['args'][0])
        except IndexError:
            return self.usage()

        # Set job-name to basename of parset-file w/o extension, if it's not
        # set on the command-line with '-j' or '--job-name'
        if not 'job_name' in self.inputs:
            self.inputs['job_name'] = (
                os.path.splitext(os.path.basename(parset_file))[0])

        # Call the base-class's `go()` method.
        return super(GenericPipeline, self).go()

    def pipeline_logic(self):
        try:
            parset_file = os.path.abspath(self.inputs['args'][0])
        except IndexError:
            return self.usage()
        try:
            self.parset.adoptFile(parset_file)
            self.parset_feedback_file = parset_file + "_feedback"
        except RuntimeError:
            print >> sys.stderr, "Error: Parset file not found!"
            return self.usage()

        # just a reminder that this has to be implemented
        validator = GenericPipelineParsetValidation(self.parset)
        if not validator.validate_pipeline():
            self.usage()
            exit(1)
        if not validator.validate_steps():
            self.usage()
            exit(1)

        #set up directories
        job_dir = self.config.get("layout", "job_directory")
        parset_dir = os.path.join(job_dir, "parsets")
        mapfile_dir = os.path.join(job_dir, "mapfiles")
        # Create directories for temporary parset- and map files
        create_directory(parset_dir)
        create_directory(mapfile_dir)

        # *********************************************************************
        # maybe we dont need a subset but just a steplist
        # at the moment only a list with stepnames is given for the pipeline.steps parameter
        # pipeline.steps=[vdsmaker,vdsreader,setupparmdb1,setupsourcedb1,ndppp1,....]
        # the names will be the prefix for parset subsets
        py_parset = self.parset.makeSubset(
            self.parset.fullModuleName('pipeline') + '.')

        # *********************************************************************
        # forward declaration of things. just for better overview and understanding whats in here.
        # some of this might be removed in upcoming iterations, or stuff gets added.
        step_parset_list = []
        step_name_list = py_parset.getStringVector('steps')
        step_control_list = []
        step_parsetarg_list = {}
        step_parset_files = {}

        for step in step_name_list:
            step_parset_list.append(self.parset.makeSubset(self.parset.fullModuleName(str(step)) + '.'))

        for step, stepname in zip(step_parset_list, step_name_list):
            step_control_list.append(step.makeSubset(step.fullModuleName('control') + '.'))
            if step.fullModuleName('parsetarg'):
                step_parsetarg_list[stepname] = step.makeSubset(step.fullModuleName('parsetarg') + '.')

        # *********************************************************************
        # save parsets
        # either a filename is given in the main parset
        # or files will be created from subsets with stepnames.parset as filenames
        for name, parset in step_parsetarg_list.iteritems():
            try:
                step_parset = parset.getString('parset')
            except:
                step_parset = os.path.join(parset_dir, name + '.parset')
                parset.writeFile(step_parset)
            step_parset_files[name] = step_parset

        # initial parameters to be saved in resultsdict so that recipes have access to this step0
        resultdicts = {'input': {
            #'mapfile': input_correlated_mapfile,
             #                    'output_instrument_mapfile': output_instrument_mapfile,
              #                   'output_correlated_mapfile': output_correlated_mapfile,
                                 'parset': parset_file,
                                 'parsetobj': self.parset,
                                 'job_dir': job_dir,
                                 'parset_dir': parset_dir,
                                 'mapfile_dir': mapfile_dir}}

        # *********************************************************************
        # main loop
        # there is a distinction between recipes and plugins for user scripts.
        # plugins are not used at the moment and might better be replaced with master recipes
        for step, stepname in zip(step_control_list, step_name_list):
            inputdict = {}
            inputargs = []
            resultdict = {}
            print 'RESULTS: ',resultdicts
            self._construct_cmdline(inputargs, step, resultdicts)

            if stepname in step_parset_files:
                inputdict['parset'] = step_parset_files[stepname]

            self._construct_input(inputdict, step, resultdicts)

            # recipes
            if step.getString('kind') == 'recipe':

                with duration(self, stepname):
                    resultdict = self.run_task(
                        step.getString('type'),
                        inputargs,
                        **inputdict
                    )

            # plugins
            if step.getString('kind') == 'plugin':
                with duration(self, stepname):
                    resultdict = loader.call_plugin(step.getString('type'), py_parset.getString('pluginpath'),
                                                    inputargs,
                                                    **inputdict)
            resultdicts[stepname] = resultdict

    # *********************************************************************
    # build the inputs for the master recipes.
    # args are for the commandline arguments and the dict for the input parameters
    # values are added from normal parset subsets or from output of earlier steps
    # parset would look something like
    # ----- bbsreducer.control.opts.sky_mapfile=setupsourcedb2.output.mapfile -----
    # 'mapfile' is the name of value in the outputdict from step setupsourcedb2.
    # that value gets assigned to 'sky_mapfile' of step with the name bbsreducer
    # code is somewhat double... need to think of some fancy method with reusabilty
    def _construct_input(self, inoutdict, controlparset, resdicts):
        argsparset = controlparset.makeSubset(controlparset.fullModuleName('opts') + '.')
        for k in argsparset.keys():
            if argsparset.getString(k).__contains__('.output.'):
                step, outvar = argsparset.getString(k).split('.output.')
                inoutdict[k] = resdicts[step][outvar]
            else:
                inoutdict[k] = argsparset.getString(k)

    def _construct_cmdline(self, inoutargs, controlparset, resdicts):
        argsparset = controlparset.makeSubset(controlparset.fullModuleName('cmdline') + '.')
        for k in argsparset.keys():
            if argsparset.getString(k).__contains__('.output.'):
                step, outvar = argsparset.getString(k).split('.output.')
                inoutargs.append(resdicts[step][outvar])
            else:
                inoutargs.append(argsparset.getString(k))

    # helper function
    def _create_mapfile_from_folder(self, folder):
        maps = DataMap([])
        measurements = os.listdir(folder)
        measurements.sort()
        for ms in measurements:
            maps.data.append(DataProduct('localhost', folder + '/' + ms, False))
        return maps


class GenericPipelineParsetValidation():

    def __init__(self,parset):
        self.parset = parset
        #self.validate_pipeline()
        #self.validate_steps()

    def validate_pipeline(self):
        try:
            self.parset.getStringVector('pipeline.steps')
            return True
        except:
            print "Error: No pipeline steps defined"
            return None

    def validate_steps(self):
        try:
            print 'NYI: validate_steps'
            return True
        except:
            print "Error: Steps validation failed"
            return None


if __name__ == '__main__':
    sys.exit(GenericPipeline().main())