# -*- coding: utf-8 -*-
# Copyright (C) 2007
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id$

import subprocess

def store_parms( pdbname, parms, create_new = False) : 

   FNULL = open( '/dev/null', 'w' )
   process = subprocess.Popen( ['parmdbm'], shell = False, stdin = subprocess.PIPE, stdout = FNULL, stderr = FNULL )
   if create_new :
      process.stdin.write( "create tablename='" + pdbname + "'\n" )
   else : 
      process.stdin.write( "open tablename='" + pdbname + "'\n" )

   parmnames = parms.keys()
   for parmname in parmnames:
      v = parms[parmname]
      times = v['times']
      nt = len(times)
      freqs = v['freqs']
      nf = len(freqs)
      timewidths = v['timewidths']
      freqwidths = v['freqwidths']
      values = v['values']
      repr_values = '[' + ', '.join([repr(v1) for v1 in values.flat]) + ']'
      freq_start = freqs[0]-freqwidths[0]/2
      freq_end = freqs[-1]+freqwidths[-1]/2
      time_start = times[0] - timewidths[0]/2
      time_end = times[-1] + timewidths[-1]/2
      domain = "[%s,%s,%s,%s]" % ( freq_start, freq_end, time_start, time_end )
      process.stdin.write("add %s shape=[%i,%i], values=%s, domain=%s\n" % (parmname, nf, nt, repr_values, domain))

   process.stdin.write('quit\n')
   process.wait()
