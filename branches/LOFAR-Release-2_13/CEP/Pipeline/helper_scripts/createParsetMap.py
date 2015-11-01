#!/usr/bin/python
#
# Create a ''map'' file using a parste input and MSes on the CEP2 system
# This make use of exisiting pipeline functionality and thsu requires
# the LOFAR pipeline path to be activated before execution using 'use LOFAR'

import os,sys
import lofar.parameterset
from lofarpipe.support.data_map import DataMap

sasid=sys.argv[1];
parsetfile='L'+sasid+'.parset'
outputfile='L'+sasid+'.map'

ps=lofar.parameterset.parameterset(parsetfile)

dps = ps.makeSubset(ps.fullModuleName('DataProducts') + '.')

output_data = DataMap([
            tuple(os.path.join(location, filename).split(':')) + (False,)
                for location, filename in zip(
                    dps.getStringVector('Output_Correlated.locations'),
                    dps.getStringVector('Output_Correlated.filenames'))
        ])

output_data.save(outputfile)

print "Done!"
