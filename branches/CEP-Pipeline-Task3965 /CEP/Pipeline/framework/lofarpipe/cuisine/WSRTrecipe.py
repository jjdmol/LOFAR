#!/usr/bin/env python
import sys
import os.path
import ingredient
import cook
import parset
import pickle

from optparse import OptionParser
from traceback import print_exc

####
# Use the standard Python logging module for flexibility.
# Standard error of external jobs goes to logging.WARN, standard output goes
# to logging.INFO.
import logging
from lofarpipe.support.pipelinelogging import getSearchingLogger


class RecipeError(cook.CookError):
    """
    Exception used to signal problems in running a recipe
    """
    pass


class NullLogHandler(logging.Handler):
    """
    A handler for the logging module, which does nothing.
    Provides a sink, so that loggers with no other handlers defined do
    nothing rather than spewing unformatted garbage.
    """
    def emit(self, record):
        pass


class WSRTrecipe(object):
    """
    Base class for recipes, pipelines are created by calling the cook_*
    methods.  Most subclasses should only need to reimplement go() and add
    inputs and outputs.  Some might need to addlogger() to messages or
    override main_results.
    """
    def __init__(self):
        """
        Initiate the input and output with empty WSRTingredient(dict)
        Fill the default python option parser
        """
        # List of inputs, outputs
        self.inputs = ingredient.WSRTingredient()
        self.outputs = ingredient.WSRTingredient()

        ## Try using the standard Python system for handling options
        self.optionparser = OptionParser(usage="usage: %prog [options]")
        self.optionparser.remove_option('-h')
        self.optionparser.add_option('-h', '--help', action="store_true")
        self.optionparser.add_option('-v', '--verbose',
                                     action="callback",
                                     callback=self.__setloglevel,
                                     help="verbose [Default: %default]")
        self.optionparser.add_option('-d', '--debug',
                                     action="callback",
                                     callback=self.__setloglevel,
                                     help="debug [Default: %default]")

        self.logger = None
        self.name = None

        self.recipe_path = ['.']

    def __setloglevel(self, option, opt_str, value, parser):
        """Callback for setting log level based on command line arguments"""
        if str(option) == '-v/--verbose':
            self.logger.setLevel(logging.INFO)
        elif str(option) == '-d/--debug':
            self.logger.setLevel(logging.DEBUG)

    def help_text(self):
        """
        Shows helptext and inputs and outputs of the recipe
        """
        print """LOFAR/WSRT pipeline framework"""

        # Print the options using the default option parser functionality
        self.optionparser.print_help()

        # _outfield is set in lofaringredient:
        if hasattr(self, '_outfields'):
            print '\nOutputs:'
            for k in self._outfields.keys():
                print '  ' + k

    def main_init(self):
        """
        Main initialization for stand alone execution, reading input from
        the command line
        """
        # The root logger has a null handler; we'll override in recipes.
        logging.getLogger().addHandler(NullLogHandler())
        self.logger = getSearchingLogger(self.name)
        opts = sys.argv[1:]

        # Try opening a default parset with the name recipename.parset
        try:
            my_parset = parset.Parset(self.name + ".parset")
            for key in my_parset.keys():
                opts[0:0] = "--" + key, my_parset.getString(key)
        except IOError:
            logging.debug(
                "Could not find (optional) default parset {0}".format(
                                self.name + ".parset"))

        # Parse the arguments using default parser
        (options, args) = self.optionparser.parse_args(opts)
        if options.help:
            return 1
        else:
            for key, value in vars(options).iteritems():
                if value is not None:
                    self.inputs[key] = value
            self.inputs['args'] = args
            return 0

    def main(self):
        """
        Main function for running the recipe in standalone mode.\
        Parse options from command line.
        Calls the run function containing actual functionality
        """
        # Get script name (no extention)
        self.name = os.path.splitext(os.path.basename(sys.argv[0]))[0]

        # Parse options
        status = self.main_init()
        if not status:
            status = self.run(self.name)
            self.main_result()
        else:
            self.help_text()

        logging.shutdown()
        return status

    def run(self, name):
        """
        This code will run if all inputs are valid, and wraps the actual
        functionality in self.go() with some exception handling, might need
        another name, like try_execute, because it's to similar to go().
        """
        self.name = name
        self.logger.info('recipe ' + name + ' started')
        try:
            status = self.go()
            if not self.outputs.complete():
                self.logger.warn("Note: recipe outputs are not complete")
        except Exception, exception_object:
            self.logger.error(exception_object)
            self.outputs = None
            return 1
        else:
            if status == 0:
                self.logger.info('recipe ' + name + ' completed')
            else:
                self.logger.warn('recipe ' + name + ' completed with errors')
            return status

    def get_run_info(self, filepath):
        # obscure function:
        try:
            fd = open(filepath + '/pipeline.pickle')
            results = pickle.load(fd)
        except:
            return None
        fd.close()
        if self.name in results.keys():
            return results[self.name]
        else:
            return None

    def set_run_info(self, filepath):
        # This function is not save to use: multiple runs share the 'state'
        try:
            fd = open(filepath + '/' + 'pipeline.pickle', 'w')
            try:
                results = pickle.load(fd)
            except:
                results = {}

            results[self.name] = {'inputs': self.inputs,
                                  'outputs': self.outputs}
            pickle.dump(results, fd)
            fd.close()
        except:
            return None

    def go(self):
        """
        Main functionality, this empty placeholder only shows help
        """
        self.help_text()

    def main_result(self):
        """
        Main results display for stand alone execution, displaying results
        on stdout
        """
        if self.outputs == None:
            print 'No results'
        else:
            print 'Results:'
            for o in self.outputs.keys():
                print str(o) + ' = ' + str(self.outputs[o])

    def cook_recipe(self, recipe, inputs, outputs):
        """
        Execute another recipe/pipeline as part of this one
        """
        c = cook.PipelineCook(recipe, inputs, outputs,
                              self.logger, self.recipe_path)

        # Start the external recipe
        c.spawn()


# Stand alone execution code ------------------------------------------
if __name__ == '__main__':
    standalone = WSRTrecipe()
    sys.exit(standalone.main())
