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

# *** XML parset combiner prototype ***
# author: Alwin de Jong (jong@astron.nl)
#
# description:
# Combines input parset files to one XML file that can be fed to DPU for processing
# with the optional -o switch the output file name can be specified. If this swtich is omitted the output file is called output.xml
# 
# syntax:
# pcombine file1 file2 [file3 ..] [-o output_file.xml]

import sys, getopt, xml.dom.minidom, re
import LAPS.MsgBus

def generate_data_xml(io, parset):
    if io == 'i':
        regexp=re.compile("ObsSW\.Observation\.DataProducts\.Input_.*\.enabled=true")
        io_tag = 'input'
    else:
        regexp=re.compile("ObsSW\.Observation\.DataProducts\.Output_.*\.enabled=true")
        io_tag = 'output'
    enabled_data=[line for line in parset if regexp.findall(line)]
    xmlFileInfo = ''
    for t in enabled_data:
        dataProductLine = t[:t.rfind('.')]
        dataProduct = dataProductLine[dataProductLine.rfind('.')+1:]
        filenames =  dataProductLine + '.filenames'
        locations = dataProductLine + '.locations'
        skip = dataProductLine + '.skip'
        xmlFileInfo += """<%s>\
<filenames>%s</filenames>\
<locations>%s</locations>\
<skip>%s</skip>\
</%s>""" % (io_tag, filenames, locations, skip, io_tag)
    return xmlFileInfo
    
    
def getPredecessors(parset):
    #return parset.split('\n', 1)[0]
    try:
        for line in parset.split('\n'):
	    if 'predecessors' in line.split('=',1)[0]:
                 predecessorline = line.split('=',1)[1].rstrip('\n').strip('[]').split(',')
		 return ','.join([l.strip('MSO') for l in predecessorline]).replace(' ','')
        #predecessorLine = [line for line in parset if line.split('=',1)[0] == 'predecessors']
        #p = predecessorLine[0].split('=')[1].rstrip('\n').strip('[]').split(',')
        #return ','.join([l.strip('MSO') for l in p]).replace(' ','')
    except:
        raise Exception('\033[91m' + 'could not get predecessors from predecessor line:' + str(predecessorline) + '\033[0m')
   
def create_xml(input_files):
    # generate pipeline xml block
    pipeline_xml = ''
    for parset,fileName in input_files:
        # generate predecessors xml tag
        predecessors = getPredecessors(parset)
        # generate inputs
        inputs = generate_data_xml('i', parset)
        # generate outputs
        outputs = generate_data_xml('o', parset)
        obsID = int(fileName.replace('Observation',''))
        pipeline_xml += """\
<pipeline>\
<pipeline_id>%s</pipeline_id>\
<predecessors>%s</predecessors>\
<inputs>%s</inputs>\
<outputs>%s</outputs>\
<parset><![CDATA[%s]]></parset>\
</pipeline>""" % (obsID, predecessors, inputs, outputs, "".join(parset))

    document = """<pipeline_block>\
<pipeline_block_id>%s</pipeline_block_id>\
<dpu_block_config></dpu_block_config>\
<comment></comment>%s</pipeline_block>\
""" % ('GUID', pipeline_xml)

    return xml.dom.minidom.parseString(document)


def print_usage():
   print 'pcombine.py [-o <outputfile.xml>] parset [parset2 parset3 ..]'


def write_output_xml(dom ):
    # ofile = open(outputfile, 'w')
    # print >>ofile, dom.toprettyxml()
    return dom.toprettyxml()


def main(argv):

    parsetqueue = LAPS.MsgBus.Bus("LAPS.resolved.parsets")
    xmlqueue = LAPS.MsgBus.Bus("LAPS.DPUservice.incoming")
    while True:
       parsets=[]

       parsets.append( parsetqueue.get() )
       parsets.append( parsetqueue.get() )

       dom = create_xml(parsets)
       xmlout = write_output_xml(dom)

       xmlqueue.send(xmlout,"XMLout")
       parsetqueue.ack()


if __name__ == "__main__":
   main(sys.argv[1:])
