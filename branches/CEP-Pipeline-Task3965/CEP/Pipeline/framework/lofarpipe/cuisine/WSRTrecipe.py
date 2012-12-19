#!/usr/bin/env python
import sys
import os.path
import parset
import logging
from optparse import OptionParser

import lofarpipe.cuisine.cook as cook
from lofarpipe.support.pipelinelogging import getSearchingLogger
from lofarpipe.support.lofaringredient import LOFARingredient
from lofarpipe.support.lofarexceptions import RecipeArgumentException

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
        Initiate the input and output with empty LOFARingredient(dict)
        Fill the default python option parser
        """
        # List of inputs, outputs
        self.inputs = LOFARingredient(None)
        self.outputs = LOFARingredient(None)

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

    def main(self):
        """
        Main function for running the recipe in standalone mode.\
        Parse options from command line.
        Calls the run function containing actual functionality.

        This is the single function called after construction of a master
        recipe in standalone mode.
        """
        # Get script name (no extention)
        self.name = os.path.splitext(os.path.basename(sys.argv[0]))[0]

        # Parse options
        status = self.prepare_run()
        if not status:
            status = self.run(self.name)
            self.finalize_run()
        else:
            self.help_text()

        logging.shutdown()
        return status

    def prepare_run(self):
        """
        Main initialization for stand alone execution, reading input from
        the command line
        """
        # The root logger has a null handler; we'll override in recipes.
        logging.getLogger().addHandler(NullLogHandler())
        self.logger = getSearchingLogger(self.name)

        # Add a stdout log handler to allow output of logging from this point
        stream_handler = logging.StreamHandler(sys.stdout)
        logformat = "%(asctime)s %(levelname)-7s %(name)s: %(message)s"
        datefmt = "%Y-%m-%d %H:%M:%S"
        formatter = logging.Formatter(logformat, datefmt)
        stream_handler.setFormatter(formatter)
        self.logger.addHandler(stream_handler)
        self.logger.setLevel(logging.DEBUG)

        # get the options from the command line
        opts = sys.argv[1:]
        try:
            # Try opening a default parset with the name recipename.parset
            my_parset = parset.Parset(self.name + ".parset")
            # Add the keys to the opts dict to allow parsing
            for key in my_parset.keys():
                opts[0:0] = "--" + key, my_parset.getString(key)
        except IOError:
            # Logger is not working here
            self.logger.debug(
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
        except RecipeArgumentException, exception_object:
            self.logger.error("Received the following argument exception:")
            self.logger.error(exception_object)
            self.help_text()
            self.outputs = None
            return 1
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

    def finalize_run(self):
        """
        Main results display for stand alone execution, displaying results
        on stdout
        """
        if self.outputs == None:
            print 'The recipe run did not create any outputs'
        else:
            print 'Results:'
            for o in self.outputs.keys():
                print str(o) + ' = ' + str(self.outputs[o])

    def go(self):
        """
        Main functionality, this empty placeholder only shows help
        """
        self.help_text()


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
