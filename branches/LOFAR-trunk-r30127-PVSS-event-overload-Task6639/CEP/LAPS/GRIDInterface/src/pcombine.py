#!/usr/bin/env python

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
  try:
    predecessorLine = [line for line in parset if line.split('=')[0][-12:] == 'predecessors']
    p = predecessorLine[0].split('=')[1].rstrip('\n').strip('[]').split(',')
    return ','.join([l.strip('MSO') for l in p]).replace(' ','')
  except:
    raise Exception('\033[91m' + 'could not get predecessors from predecessor line:' + str(predecessorLine) + '\033[0m')
   
def create_xml(input_files):
  # generate pipeline xml block
  pipeline_xml = ''
  for fileName in input_files:
    # read the parset file
    parsetFile = open(fileName, 'r')
    parset = parsetFile.readlines()
    parsetFile.close()

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
  

def write_output_xml(dom, outputfile):
  ofile = open(outputfile, 'w')
  print >>ofile, dom.toprettyxml()
  

def main(argv):

  outputfile = 'output.xml'

  try:
    opts, input_files = getopt.getopt(argv,"ho:",["ofile="])
  except getopt.GetoptError:
    print_usage()
    sys.exit(2)
    
  if not input_files:
    raise Exception('\033[91m' + 'no input parset file(s) specified' + '\033[0m')
  
  for opt, arg in opts:
    if opt == '-h':
      print_usage()
      sys.exit()
    elif opt in ("-o", "--ofile"):
      outputfile = arg

  dom = create_xml(input_files)
  write_output_xml(dom, outputfile)
  
  
if __name__ == "__main__":
   main(sys.argv[1:])
