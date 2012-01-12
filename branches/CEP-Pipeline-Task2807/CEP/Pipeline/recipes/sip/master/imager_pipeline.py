
import os
import sys
import errno

from lofarpipe.support.control import control
from lofar.parameterset import parameterset #@UnresolvedImport
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.utilities import create_directory

class imager_pipeline(control):
    """
    Description 
    """
    inputs = {
        'input_mapfile': ingredient.FileField(
            '--input-mapfile',
            help = "mapfile with inputs specific to this node script"
        )
    }

    def __init__(self):
        control.__init__(self)
        self.parset = parameterset()


    def usage(self):
        print >> sys.stderr, "Usage: %s <parset-file>  [options]" % sys.argv[0]
        return 1


    def go(self):
        """
        Read the parset-file that was given as input argument, and set the
        jobname before calling the base-class's `go()` method.
        """
        try:
            parset_file = self.inputs['args'][0]
        except IndexError:
            return self.usage()
        self.parset.adoptFile(parset_file)
        # Set job-name to basename of parset-file w/o extension, if it's not
        # set on the command-line with '-j' or '--job-name'
        if not self.inputs.has_key('job_name'):
            self.inputs['job_name'] = (
                os.path.splitext(os.path.basename(parset_file))[0]
            )
        super(imager_pipeline, self).go()


    def pipeline_logic(self):
        """
        Define the individual tasks that comprise the current pipeline.
        This method will be invoked by the base-class's `go()` method.
        """
        #*****************************************************************
        #Get parameters prepare imager from the parset and inputs
        raw_ms_mapfile = self.inputs['input_mapfile']

        prepare_imager_parset = self.parset.makeSubset("prepare_imager.")

        ndppp = prepare_imager_parset.getString("ndppp")
        initscript = prepare_imager_parset.getString("initscript")
        working_directory = self.config.get("DEFAULT", "default_working_directory") #.#get("working_directory","error")
        output_mapfile = prepare_imager_parset.getString("output_mapfile")
        slices_per_image = prepare_imager_parset.getInt("slices_per_image")
        subbands_per_image = prepare_imager_parset.getInt("subbands_per_image")
        mapfile = prepare_imager_parset.getString("mapfile")

        #write subset of parameters to file
        prepare_imager_parset_file = \
            self._write_parset_to_file(prepare_imager_parset, "prepare_imager")

        #run the prepare imager
        prepare_imager_output_mapfile = None
        skip_prepare = False
        if skip_prepare:
            prepare_imager_output_mapfile = "/home/klijn/build/preparation/actual_output.map"
        else:
            prepare_imager_output_mapfile = \
                    self.run_task("prepare_imager", raw_ms_mapfile,
                        ndppp = ndppp,
                        initscript = initscript,
                        parset = prepare_imager_parset_file,
                        working_directory = working_directory, #TODO: deze parameters hoeven dus echt niet meer mee.
                        output_mapfile = output_mapfile,
                        slices_per_image = slices_per_image,
                        subbands_per_image = subbands_per_image,
                        mapfile = mapfile)['mapfile']

        #*****************************************************************
        #Get parameters awimager from the parset and inputs        
        awimager_parset = self.parset.makeSubset("awimager.")
        executable = awimager_parset.getString("executable")

        awimager_parset = \
            self._write_parset_to_file(awimager_parset, "awimager")

        #run the awimager recipe
        awimager_output_mapfile = \
            self.run_task("awimager", prepare_imager_output_mapfile,
                          parset = awimager_parset,
                          executable = executable)

        #run the awimager recipe
#        awimager_output_mapfile = \
#            self.run_task("bbs_imager", #prepare_imager_output_mapfile,
#                          parset = "/home/klijn/build/preparation/parset.par",
#                          executable = "/opt/cep/LofIm/daily/lofar/lib/python2.6/dist-packages/lofar/gsmutils.py",
#                          initscript = "/opt/cep/LofIm/daily/lofar/lofarinit.sh")


        return 0




    def _write_parset_to_file(self, parset, parset_name):
        """
        Write the suplied the suplied parameterset to the parameter set 
        directory in the jibs dir with the filename suplied in parset_name.
        Return the full path to the created file.
        
        """
        # todo make a subset parset and supply to the prepareimager        
        parset_dir = os.path.join(
            self.config.get("layout", "job_directory"), "parsets")
        #create the parset dir if it does not exist
        create_directory(parset_dir)

        #write the content to a new parset file
        prepare_imager_parset_file = os.path.join(parset_dir,
                         "{0}.parset".format(parset_name))
        parset.writeFile(prepare_imager_parset_file)

        return prepare_imager_parset_file

if __name__ == '__main__':
    sys.exit(imager_pipeline().main())
