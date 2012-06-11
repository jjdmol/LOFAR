"""Module image.

Instances of class Image are a primary data-holders for all PyBDSM
operations. They store the image itself together with some meta-information
(such as headers), options for processing modules and all data generated during
processing. A few convenience methods are also defined here for interactive
use: to allow viewing and output of the most important data, to allow listing
and setting of options, and to allow re-processing of Images (these methods are
used by the interactive IPython shell made by pybdsm.py).

This module also defines class Op, which is used as a base class for all PyBDSM
operations.
"""

import numpy as N
from opts import *

class Image(object):
    """Image is a primary data container for PyBDSM.

    All the run-time data (such as image data, mask, etc.) 
    is stored here. A number of type-checked properties 
    are defined for the most basic image attributes, such
    as image data, mask, header, user options.

    There is little sense in declaring all possible attributes
    right here as it will introduce unneeded dependencies 
    between modules, thus most other attributes (like island lists,
    gaussian lists, etc) are inserted at run-time by the specific
    PyBDSM modules.
    """
    opts   = Instance(Opts, doc="User options")
    image  = NArray(doc="Image data, Stokes I")
    ch0    = NArray(doc="Channel-collapsed image data, Stokes I")
    ch0_Q  = NArray(doc="Channel-collapsed image data, Stokes Q")
    ch0_U  = NArray(doc="Channel-collapsed image data, Stokes U")
    ch0_V  = NArray(doc="Channel-collapsed image data, Stokes V")
    header = Any(doc="Image header")
    mask   = NArray(doc="Image mask (if present and attribute masked is set)")
    masked = Bool(False, doc="Flag if mask is present")
    basedir = String('DUMMY', doc="Base directory for output files")
    completed_Ops = List(String(), doc="List of completed operations")


    def __init__(self, opts):
        self.opts = Opts(opts)
        self.extraparams = {}

    def list_pars(self):
        """List parameter values."""
        import interface
        interface.list_pars(self)
        
    def set_pars(self, **kwargs):
        """Set parameter values."""
        import interface
        try:
            interface.set_pars(self, **kwargs)
        except RuntimeError, err:
            print '\n\033[31;1mERROR\033[0m: ' + str(err)

    def process(self, **kwargs):
        """Process Image object"""
        import interface
        success = interface.process(self, **kwargs)
        return success
    
    def save_pars(self, savefile=None):
        """Save parameter values."""
        import interface
        interface.save_pars(self, savefile)

    def load_pars(self, loadfile=None):
        """Load parameter values."""
        import interface
        import os
        if loadfile == None or loadfile == '':
            loadfile = self.opts.filename + '.pybdsm.sav'
        if os.path.exists(loadfile):
            timg = interface.load_pars(loadfile)
            if timg != None:
                orig_filename = self.opts.filename
                self.opts = timg.opts
                self.opts.filename = orig_filename # reset filename to original
            else:
                print "\n\033[31;1mERROR\033[0m: '"+\
                    loadfile+"' is not a valid parameter save file."
        else:
            print "\n\033[31;1mERROR\033[0m: File '"+\
                loadfile+"' not found."

    def show_fit(self, **kwargs):
        """Show results of the fit."""
        import plotresults
        if not hasattr(self, 'nisl'):
            print 'Image has not been processed. Please run process_image first.'
            return False
        plotresults.plotresults(self, **kwargs)
        return True
        
    def export_image(self, **kwargs):
          """Export an internal image to a file."""
          import interface
          interface.export_image(self, **kwargs)
          
    def write_catalog(self, **kwargs):
        """Write the Gaussian, source, or shapelet list to a file"""
        import interface
        interface.write_catalog(self, **kwargs)
    write_gaul = write_catalog # for legacy scripts
    

class Op(object):
    """Common base class for all PyBDSM operations.

    At the moment this class is empty and only defines placeholder
    for method __call__, which should be redefined in all derived
    classes.
    """
    def __call__(self, img):
        raise NotImplementedError("This method should be redefined")
