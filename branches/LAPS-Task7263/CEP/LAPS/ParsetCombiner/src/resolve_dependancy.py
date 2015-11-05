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
# $Id: pcombine.py 29835 2014-07-29 13:16:30Z klijn $

# *** XML parset combiner prototype ***
# author: Alwin de Jong (jong@astron.nl)
#
# description:
# Combines input parset files to one XML file that can be fed to DPU for processing
# with the optional -o switch the output file name can be specified. If this swtich is omitted the output file is called output.xml
# 
# syntax:
# pcombine file1 file2 [file3 ..] [-o output_file.xml]
import sys,getopt
import LAPS.MsgBus


resolvedqueue = LAPS.MsgBus.ToBus("laps.resolved.parsets")
stagerequest = LAPS.MsgBus.ToBus("laps.staging.request")

def incoming_parset(incoming,msg):
    parset = msg.content
    parsetid = msg.subject
    print "incoming parset %s "%( parsetid )
    stagerequest.sendstr(parset,parsetid)
    incoming.ack(msg)

def incoming_staged(incoming,(incoming,msg):
    parset = msg.content
    parsetid = msg.subject
    print "incoming staged info for %s" %( parsetid )
    resolvedqueue.sendstr(parset,parsetid)
    incoming.ack(msg)

def main(argv):


    incoming = laps.MsgBus.FromBus("laps.retrieved.parsets",handler=incoming_parset)
    incoming.add("laps.staging.staged",handler=incoming_staged)
    while (True):
       incoming.handleOne()

if __name__ == "__main__":
   main(sys.argv[1:])
