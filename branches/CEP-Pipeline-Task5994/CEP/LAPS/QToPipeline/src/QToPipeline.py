#!/usr/bin/python
# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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

import sys
import os
import laps.MsgBus

print " setup connection "

msgbus = laps.MsgBus.Bus()
workdir="/data/scratch/lofarsys/regression_test_runner/"
worker="msss_calibrator_pipeline"
workspace="/cep/lofar_build/lofar/release/"

# get the Parset and the filename
message, filename = msgbus.get()

while message:
  	print "received :"
	f = open(filename,"wr")
	f.write(message)
	f.close() 

	parsetvals={}
	index=0
	for line in message.split('\n'):
		nme,val = line.partition("=")[::2]
		parsetvals[nme.strip()]=val.strip()
		#print "got %s : %s nvpair" %(nme,val) 
		#print "got line %d : %s " %(index,line)
		#index=index+1
	pythonprogram = parsetvals["ObsSW.Observation.ObservationControl.PythonControl.pythonProgram"] 
	os.system('python "%s/bin/%s.py" "%s/%s.parset" -c "%s/pipeline.cfg" -d' %( workspace,pythonprogram,workdir,pythonprogram,workdir))
	#os.system("startPython.sh %s %s >> logfile.txt 2>&1" %(pythonprogram,filename))
  	msgbus.ack()
	message, subject = msgbus.get()


