#                                                         LOFAR IMAGING PIPELINE
#
#                  vdsreader recipe: extract filenames + metadata from GVDS file
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

import lofarpipe.support.utilities as utilities
import lofarpipe.support.lofaringredient as ingredient

from lofarpipe.support.baserecipe import BaseRecipe
from lofar.parameterset import parameterset


class vdsreader(BaseRecipe):
    """
    Read a GVDS file and return a list of the MS filenames referenced therein
    together with selected metadata.
    
    This recipe performs it's functionality at the master side of the recipe:
    
    1. Open the gvds file as a parameterset
    2. Convert all part FileNames to mss
    3. Parse start and end time and pointing information

    **no command line arguments:**

    """
    inputs = {
        'gvds': ingredient.FileField(
            '-g', '--gvds',
            help="GVDS file to process"
        )
    }

    outputs = {
        'data': ingredient.ListField(help="List of MeasurementSet paths"),
        'start_time': ingredient.StringField(help="Start time of observation"),
        'end_time': ingredient.StringField(help="End time of observation"),
        'pointing': ingredient.DictField(help="Observation pointing direction")
    }

    def go(self):
        self.logger.info("Starting vdsreader run")
        super(vdsreader, self).go()

        # *********************************************************************
        # 1. Open the gvds file as a parameterset
        try:
            gvds = parameterset(self.inputs['gvds'])
        except:
            self.logger.error("Unable to read G(V)DS file")
            raise

        self.logger.info("Building list of measurementsets")

        # **********************************************************************
        # 2. convert al partx.FileName values to ms
        ms_names = [
            gvds.getString("Part%d.FileName" % (part_no,))
            for part_no in xrange(gvds.getInt("NParts"))
        ]
        self.logger.debug(ms_names)

        self.outputs['data'] = ms_names

        # **********************************************************************\
        # 3. parse start and end time and pointing information
        try:
            self.outputs['start_time'] = gvds.getString('StartTime')
            self.outputs['end_time'] = gvds.getString('EndTime')
        except:
            self.logger.warn("Failed to read start/end time from GVDS file")
        try:
            self.outputs['pointing'] = {
                'type': gvds.getStringVector('Extra.FieldDirectionType')[0],
                'dec': gvds.getStringVector('Extra.FieldDirectionDec')[0],
                'ra': gvds.getStringVector('Extra.FieldDirectionRa')[0]
            }
        except:
            self.logger.warn("Failed to read pointing information from GVDS file")
        return 0
