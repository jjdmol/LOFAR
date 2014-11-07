import os
import sys

from lofarpipe.support.parset import Parset
from lofarpipe.support.control import control

from lofarpipe.support.loggingdecorators import mail_log_on_exception, duration
from lofarpipe.support.data_map import DataMap, DataProduct
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
        validator = GenericPipelineParsetValidation(self.parset)
        if not validator.validate_pipeline():
            self.usage()
            exit(1)
        if not validator.validate_steps():
            self.usage()
            exit(1)

        print self.parset

        py_parset = self.parset.makeSubset(
            self.parset.fullModuleName('pipeline') + '.')

        #set up directories for variables
        job_dir = self.config.get("layout", "job_directory")
        parset_dir = os.path.join(job_dir, "parsets")
        mapfile_dir = os.path.join(job_dir, "mapfiles")
        # Create directories for temporary parset- and map files
        create_directory(parset_dir)
        create_directory(mapfile_dir)

        stepparsetlist = []
        stepnamelist = py_parset.getStringVector('steps')

        for step in stepnamelist:
            stepparsetlist.append(self.parset.makeSubset(self.parset.fullModuleName(str(step)) + '.'))
            print step

        stepcontrollist = []
        stepargparsetlist = {}
        step_parset_files = {}
        mapfiles = {}

        for step, stepname in zip(stepparsetlist, stepnamelist):
            stepcontrollist.append(step.makeSubset(step.fullModuleName('control') + '.'))
            if step.fullModuleName('parsetarg'):
                stepargparsetlist[stepname]=step.makeSubset(step.fullModuleName('parsetarg') + '.')
            else:
                print "no parset given"

        #save parsets
        for k, v in stepargparsetlist.iteritems():
            try:
                step_parset = v.getString('parset')
            except:
                step_parset = os.path.join(parset_dir, k + '.parset')
                v.writeFile(step_parset)
            step_parset_files[k]=step_parset

        testmapfile = self._create_mapfile_from_folder('/home/zam/sfroehli/testpipeline/data/MS')
        testmapfile_name = os.path.join(mapfile_dir, 'measurements.mapfile')
        testmapfile.save(testmapfile_name)
        mapfiles['input'] = testmapfile_name
        print testmapfile
        print 'parset HERE: ',step_parset_files
        resultdicts = {}#{'first': {'mapfile': 'dummy'}}
        for step, stepname in zip(stepcontrollist, stepnamelist):
            # common
            try:
                input_mapfile = mapfiles[step.getString('mapfilefromstep')]
            except:
                input_mapfile = mapfiles.values()[-1]  # input mapfile: last in list. added from last step.

            # recipes
            if step.getString('kind') == 'recipe':
                #inputdict = {"mapfile": os.path.join(mapfile_dir, stepname + '.mapfile')}
                inputdict={}
                if stepname in step_parset_files:
                    print 'ADDED PARSET'
                    inputdict['parset']=step_parset_files[stepname]

                self._construct_input(inputdict, step, resultdicts)

                with duration(self, stepname):
                    if stepname is 'vdsreader':
                        resultdict = self.run_task(
                        step.getString('type'),
                        **inputdict
                        )
                    else:
                        resultdict = self.run_task(
                        step.getString('type'),
                        input_mapfile,
                        **inputdict
                        )
                    if 'mapfile' in resultdict:
                        mapfiles[stepname] = resultdict['mapfile']
                    resultdicts[stepname] = resultdict

            # plugins
            if step.getString('kind') == 'plugin':
                loader.call_plugin(step.getString('type'), py_parset.getString('pluginpath'),
                                   step.getString('type'), input_mapfile)

    def _construct_input(self, indict, controlparset, resdicts):
        argsparset = controlparset.makeSubset(controlparset.fullModuleName('args') + '.')
        for k in argsparset.keys():
            print 'THIS IS K: ',k
            if argsparset.getString(k).__contains__('.output.'):
                step, outvar = argsparset.getString(k).split('.output.')
                print "FOUND RESULTREQUEST FROM %s FOR VALUE %s: %s" % (step, outvar, resdicts[step][outvar])
            else:
                indict[k] = argsparset.getString(k)
        return indict

    def _construct_step_parsets(self, steplistparsets):

        parsetdict = {}
        return parsetdict

    def _create_mapfile_from_folder(self, folder):
        #here comes the creation of a data mapfile (what MS lays where)
        #list of dicts [{host: string, file: string, skip: bool}, {next MS}]
        #just a string of the filename?!
        #mapfile = 'testmap.txt'
        maps = DataMap()
        measurements = os.listdir(folder)
        measurements.sort()
        for ms in measurements:
            maps.data.append(DataProduct('localhost', folder + '/' + ms, False))
        #maps.save(mapfile)
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