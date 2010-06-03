#!/usr/bin/python

import lofar.parmdb
import lofar.expion.parmdbmain
import sys
import numpy

rank = sys.argv[1]
instrument_pdbname = sys.argv[2]
ClockTEC_pdbname = sys.argv[3]
ion_pdbname = sys.argv[4]

ion_pdbname = '/'.join(instrument_pdbname.split('/')[0:-1]) + '/' + ion_pdbname

instrument_pdb = lofar.parmdb.parmdb( instrument_pdbname )
ClockTEC_pdb = lofar.parmdb.parmdb( ClockTEC_pdbname )

instrument_parms = instrument_pdb.getValuesGrid('Gain:{0:0,1:1}:*')
Clock_parms = ClockTEC_pdb.getValuesGrid('Clock*')
TEC_parms = ClockTEC_pdb.getValuesGrid('TEC*')

ionosphere_parms = instrument_parms
for parm_name in Clock_parms.keys() :
   parm_name_split = parm_name.split(':')
   pol = parm_name_split[1]
   station = parm_name_split[2]
   try :
      Clock = Clock_parms[ ':'.join( [ 'Clock', pol, station ] ) ][ 'values' ]
      TEC = TEC_parms[ ':'.join( [ 'TEC', pol, station ] ) ][ 'values' ]
   except KeyError :
      pass
   else:
      parm_real = ionosphere_parms[ ':'.join( [ 'Gain', pol, pol, 'Real', station] ) ]
      parm_imag = ionosphere_parms[ ':'.join( [ 'Gain', pol, pol, 'Imag', station] ) ]
      freqs = parm_real['freqs']
      phases = Clock*freqs*2*numpy.pi/1e9 + TEC*8e9/freqs
      phasors = numpy.sqrt(parm_real['values']**2 + parm_imag['values']**2) * numpy.exp(1j * phases)
      parm_real[ 'values' ] = phasors.real
      parm_imag[ 'values' ] = phasors.imag

lofar.expion.parmdbmain.store_parms( ion_pdbname, ionosphere_parms, create_new = True )



