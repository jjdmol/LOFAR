#! /usr/bin/env python

# XML generator prototype

VERSION = "2.15.0"
    
import sys, getopt, time
from xml.sax.saxutils import escape as XMLescape
from os import _exit as os_exit
from os.path import splitext
from datetime import datetime,timedelta
from math import pi
import re

CLOCK_MODES = ['160 MHz','200 MHz']
INSTRUMENT_FILTERS = ["10-70 MHz", "30-70 MHz", "10-90 MHz", "30-90 MHz", "110-190 MHz", "170-230 MHz", "210-250 MHz"]
ANTENNA_MODES = ["LBA Inner", "LBA Outer", "LBA Sparse Even", "LBA Sparse Odd", "LBA X", "LBA Y", "HBA Zero", "HBA Zero Inner", "HBA One", "HBA One Inner", "HBA Dual", "HBA Dual Inner", "HBA Joined", "HBA Joined Inner"]
NUMBER_OF_BITS_PER_SAMPLE = [4,8,16]
MAX_NR_SUBBANDS = [976,488,244]
WHICH_IS = ['IQUV','I']
WHICH_CS = ['IQUV','I','XXYY']
WEIGHTING_SCHEMES = ['uniform', 'superuniform', 'natural', 'briggs', 'briggsabs', 'radial']
IMAGING_PIPELINE_TYPES = ['MSSS','standard','none']
#MODES = ['Calobs','Calbeam','MultiObs']
PROCESSING = ['Preprocessing','Calibration','Pulsar','Imaging','LongBaseline','none']
CALIBRATION_MODE = ['internal','external','none']
ALL_STATIONS = 'CS001,CS002,CS003,CS004,CS005,CS006,CS007,CS011,CS013,CS017,CS021,CS024,CS026,CS028,CS030,CS031,CS032,CS101,CS103,CS201,CS301,CS302,CS401,CS501,RS106,RS205,RS208,RS210,RS305,RS306,RS307,RS310,RS406,RS407,RS409,RS503,RS508,RS509,DE601,DE602,DE603,DE604,DE605,FR606,SE607,UK608,DE609'
CORE_STATIONS = 'CS001,CS002,CS003,CS004,CS005,CS006,CS007,CS011,CS013,CS017,CS021,CS024,CS026,CS028,CS030,CS031,CS032,CS101,CS103,CS201,CS301,CS302,CS401,CS501'
SUPERTERP_STATIONS = 'CS002,CS003,CS004,CS005,CS006,CS007'
REMOTE_STATIONS = 'RS106,RS205,RS208,RS210,RS305,RS306,RS307,RS310,RS406,RS407,RS409,RS503,RS508,RS509'
INTERNATIONAL_STATIONS = 'DE601,DE602,DE603,DE604,DE605,FR606,SE607,UK608,DE609,PL610,PL611,PL612'
NL_STATIONS = 'CS001,CS002,CS003,CS004,CS005,CS006,CS007,CS011,CS013,CS017,CS021,CS024,CS026,CS028,CS030,CS031,CS032,CS101,CS103,CS201,CS301,CS302,CS401,CS501,RS106,RS205,RS208,RS210,RS305,RS306,RS307,RS310,RS406,RS407,RS409,RS503,RS508,RS509'

RED_COLOR    = '\033[91m'
NO_COLOR     = '\033[0m'
YELLOW_COLOR = '\033[93m'
CYAN_COLOR   = '\033[96m'
GREEN_COLOR  = '\033[92m'
#BLUE_COLOR   = '\033[94m'
TRUE         = ['y','Y','YES','yes','t','T','True','true']
FALSE        = ['n','N','NO','no','f','F','False','false']

class GenException(Exception):
  def __init__(self, message):
    # Call the base class constructor with the parameters it needs
    super(Exception, self).__init__(RED_COLOR + message + NO_COLOR)

def merge_dicts(*dict_args):
  '''
  Given any number of dicts, shallow copy and merge into a new dict,
  precedence goes to key value pairs in latter dicts.
  '''
  result = {}
  for dictionary in dict_args:
      result.update(dictionary)
  return result

def printMessage(message):
  print(GREEN_COLOR + message + NO_COLOR)

def printInfo(message):
  print(CYAN_COLOR + 'INFO: ' + message + NO_COLOR)

def printWarning(message):
  print(YELLOW_COLOR + 'WARNING: ' + message + NO_COLOR)

def dms2deg(dms_str):
  arr = re.findall(r'\d+', dms_str)
  while len(arr) < 4: # pad DMS string if not all of H,M,S are specified e.g. 20:10 will be 20:10:0.0
    arr.append(0)

  if dms_str[0].strip() == '-': #negative sign can only happen at the start of the string
    sign = -1
  else:
    sign = 1
  
  arr[3] = float(arr[3]) / (10**len(str(arr[3])))

  return sign * (abs(int(arr[0])) + float(arr[1]) / 60 + (float(arr[2]) + arr[3]) / 3600)

def hms2deg(hms_str):
  arr = re.findall(r'\d+', hms_str)
  while len(arr) < 4: # pad HMS string if not all of H,M,S are specified e.g. 20:10 will be 20:10:0.0
    arr.append(0)

  #FIXME Probably we shouldn't even allow negatives, hour angles should be between 0 and 24
  if hms_str[0].strip() == '-': #negative sign can only happen at the start of the string
    sign = -1
  else:
    sign = 1
  
  arr[3] = float(arr[3]) / (10**len(str(arr[3])))
    
  return sign * (abs(int(arr[0])) + float(arr[1]) / 60 + (float(arr[2]) + arr[3]) / 3600) * 15

def deg2rad(degrees):
  return float(degrees) * pi / 180
  
def rad2deg(radian):
  return float(radian) * 180 / pi

# def convertAngle(number, angle, beamName): #TODO get one convertAngle function
#   # try converting to degrees else radians else HMS
#   if angle.endswith('deg') or angle.endswith('d'): # ra specified with 'deg' ?
#     angle = angle.rstrip(' deg')
#   else:
#     try: # try radian units
#       ra_deg = rad2deg(angle);
#       angle = ra_deg
#     except: # assuming hms
#       if not (angle.endswith('s') or angle[-1].isdigit()):
#         raise GenException("unkown coordinate: %s for angle%i of %s" % angle, number, beamName)
#       angle = str(hms2deg(angle))
#   return angle

def convertAngle1(angle, beamName):
  # try converting to degrees else radians else HMS
  if angle.endswith('deg') or angle.endswith('d'): # ra specified with 'deg' ?
    angle = angle.rstrip(' deg')
  else:
    try: # try radian units
      ra_deg = rad2deg(angle);
      angle = ra_deg
    except: # assuming hms
      if not (angle.endswith('s') or angle[-1].isdigit()):
        raise GenException("unkown coordinate: %s for angle1 of %s" % angle, beamName)
      angle = str(hms2deg(angle))
  return angle

def convertAngle2(angle, beamName):
  # try converting to degrees else radians else HMS
  if angle.endswith('deg') or angle.endswith('d'): # ra specified with 'deg' ?
    angle = angle.rstrip(' deg')
  else:
    try: # try radian units
      dec_deg = rad2deg(angle);
      angle = dec_deg
    except: # assuming dms
      if not (angle.endswith('s') or angle[-1].isdigit()):
        raise GenException("unkown coordinate: %s for angle2 of %s" % angle, beamName)
      angle = str(dms2deg(angle))
  return angle

def parse_subband_list(parset_subband_list, nr_subbands):
    r'''
    Parse a subband list from a parset.

    **Parameters**

    parset_subband_list : string
        Value of Observation.Beam[0].subbandList

    **Returns**

    A list of integers containing the subband numbers.

    **Examples**


    >>> parse_subband_list('[154..163,185..194,215..224,245..254,275..284,305..314,335..344,10*374]')
    >>> parse_subband_list('[77..87,116..127,155..166,194..205,233..243,272..282,311..321]')
    >>> parse_subband_list('[]')
    '''
    stripped_subband_list = parset_subband_list.strip('[] \n\t').replace(' ', '')
    if stripped_subband_list == '':
        return []
    sub_lists = [word.strip().split('..') for word in stripped_subband_list.split(',')]
    subbands = []
    for sub_list in sub_lists:
        if len(sub_list) == 1:
            multiplication = sub_list[0].split('*')
            if len(multiplication) == 2:
                subbands += [int(multiplication[1])]*int(multiplication[0])
            else:
                subbands.append(int(sub_list[0]))
        elif len(sub_list) == 2:
            subbands += range(int(sub_list[0]), int(sub_list[1])+1)
        else:
            raise GenException(str(word) + ' is not a valid sub_range in a subband list')
            return []
    doubles = set([x for x in subbands if subbands.count(x) > 1])
    if len(doubles) > 0:
      printWarning(parset_subband_list + ' contains the following double specified subbands: %s' % sorted(doubles))
    return subbands

def verifySubbandList(keyname, parset_subband_list, nr_subbands):
  subbandListCalculated = parse_subband_list(parset_subband_list, nr_subbands)
  calcNrSubbands = len(subbandListCalculated)
  if calcNrSubbands != int(nr_subbands):
    raise GenException("%s error: calculated number of subbands (%i) is not equal to the specified number of subbands (%s)\nIs the subband list correct?") % (keyname, calcNrSubbands, nr_subbands)

def readExtraParms(keyset, lines):
  valListEsc = []
  for line in lines:
    if line.startswith(keyset + ":") or line.startswith(keyset + "="):
      line = re.sub(r"\s+;", ';', line.lstrip(keyset).lstrip(":").lstrip("=").rstrip()) # clear white-space just before ';'
      line = re.sub(r";\s+", ';', line) # clear white-space directly after ';' (this method intentionally does not clear white-space in the (string) parameters self!)
      valList = line.split(';')
      for strVal in valList:
        valListEsc.append(XMLescape(strVal))
  return valListEsc
    
def readTiedArrayBeams(lines):
  tabs = []
  stopTABsearch = False
  try:
    for line in lines:
      if line.startswith("TAB") or line.startswith("Global_TAB"):
        continue
      else:
        valList = line.lstrip().rstrip().replace(' ', '').split(';')
        if valList[0].startswith('c'):
          # angle1
          if valList[1].endswith('deg') or valList[1].endswith('d'): # degree units?
            valList[1] = deg2rad(valList[1].rstrip(' deg'))
          else: #try radian else HMS
            try: # if float conversion works assume radian
              angle1 = float(valList[1]);
              valList[1] = angle1
            except: # float conversion did not work try hms
              valList[1] = deg2rad(hms2deg(valList[1]))
          # angle2
          if valList[2].endswith('deg') or valList[2].endswith('d'): # degree units?
            valList[2] = deg2rad(valList[2].rstrip(' deg'))
          else: #try radian else HMS
            try: # if float conversion works assume radian
              angle2 = float(valList[2]);
              valList[2] = angle2
            except: # float conversion did not work try hms
              valList[2] = deg2rad(dms2deg(valList[2]))
          #if valList[2].endswith('deg') or valList[2].endswith('d'):
            #valList[2] = deg2rad(valList[2].rstrip(' deg'))
          tabs.append(valList)
        elif valList[0].startswith('i'):
          valList[1] = float(valList[1])
          tabs.append(valList)
  except:
    raise GenException("An error occurred reading the TAB specification on line '%s'" % line)
  return tabs

def hasCoherentTab(TAB):
  for i in range(0,len(TAB)):
    if TAB[i][0] == 'c':
      return True
  return False

##FIXME we will need to fill in actual values. Might need to depend on variables
def processingCluster(cluster, number_of_tasks=244):
  CEP2 = r"""  <processingCluster>
                    <name>CEP2</name>
                    <partition>/data</partition>
                    <numberOfTasks>%i</numberOfTasks>
                    <minRAMPerTask unit="byte">1000000000</minRAMPerTask>
                    <minScratchPerTask unit="byte">100000000</minScratchPerTask>    
                    <maxDurationPerTask>PT600S</maxDurationPerTask>
                    <numberOfCoresPerTask>20</numberOfCoresPerTask>
                    <runSimultaneous>true</runSimultaneous>
                  </processingCluster>"""

  CEP4 = r"""  <processingCluster>
                    <name>CEP4</name>
                    <partition>/data</partition>
                    <numberOfTasks>%i</numberOfTasks>
                    <minRAMPerTask unit="byte">1000000000</minRAMPerTask>
                    <minScratchPerTask unit="byte">100000000</minScratchPerTask>    
                    <maxDurationPerTask>PT600S</maxDurationPerTask>
                    <numberOfCoresPerTask>20</numberOfCoresPerTask>
                    <runSimultaneous>true</runSimultaneous>
                  </processingCluster>"""

  if cluster == "CEP4":
    result = CEP4 % (number_of_tasks,)
  else:
    result = r"" #CEP2 % (number_of_tasks,)
  return result

def dataProductCluster(cluster):
  CEP2 = r"""  <storageCluster>
                      <name>CEP2</name>
                      <partition>/data</partition>
                    </storageCluster>"""
  CEP4 = r"""<storageCluster>
                      <name>CEP4</name>
                      <partition>/data</partition>
                    </storageCluster>"""

  if cluster == "CEP4":
    result = CEP4
  else:
    result = r"" #CEP2
  return result

def writeXMLObs(ofile, name, descr, topo, predecessor_topo, attrname, projname, TBBpiggyBack, aartfaacPiggyBack, cordata, cohdata, incohdata, antenna, clock, instrfilt, interval, channels,
  cohdedisp, flysEye, subsperfileCS, colapseCS, downstepsCS, whichCS, subsperfileIS, colapseIS, downstepsIS, whichIS, stations, start, stop, duration, bitspersample):
  print >>ofile, r"""          <item index="0">
                <lofar:observation>
                  <name>%s</name>
                  <description>%s</description>
                  <topology>%s</topology>
                  <predecessor_topology>%s</predecessor_topology>
                  <currentStatus>
                    <mom2:openedStatus/>
                  </currentStatus>
                  <lofar:observationAttributes>
                    <observationId>
                    </observationId>
                    <name>%s</name>
                    <projectName>%s</projectName>
                    <instrument>Beam Observation</instrument>
                    <defaultTemplate>BeamObservation</defaultTemplate>
                    <tbbPiggybackAllowed>%s</tbbPiggybackAllowed>
                    <aartfaacPiggybackAllowed>%s</aartfaacPiggybackAllowed>
                    <userSpecification>
                      <correlatedData>%s</correlatedData>
                      <coherentStokesData>%s</coherentStokesData>
                      <incoherentStokesData>%s</incoherentStokesData>
                      <antenna>%s</antenna>
                      <clock mode="%s"/>
                      <instrumentFilter>%s</instrumentFilter>
                      <integrationInterval>%s</integrationInterval>
                      <channelsPerSubband>%s</channelsPerSubband>
                      <coherentDedisperseChannels>%s</coherentDedisperseChannels>
                      <tiedArrayBeams>
                        <flyseye>%s</flyseye>
                      </tiedArrayBeams>
                      <stokes>
                        <integrateChannels>false</integrateChannels>
                        <subbandsPerFileCS>%s</subbandsPerFileCS>
                        <numberCollapsedChannelsCS>%s</numberCollapsedChannelsCS>
                        <stokesDownsamplingStepsCS>%s</stokesDownsamplingStepsCS>
                        <whichCS>%s</whichCS>
                        <subbandsPerFileIS>%s</subbandsPerFileIS>
                        <numberCollapsedChannelsIS>%s</numberCollapsedChannelsIS>
                        <stokesDownsamplingStepsIS>%s</stokesDownsamplingStepsIS>
                        <whichIS>%s</whichIS>
                      </stokes>
                      <stationSet>Custom</stationSet>
                      <stations>%s</stations>
                      <timeFrame>UT</timeFrame>
                      <startTime>%s</startTime>
                      <endTime>%s</endTime>
                      <duration>%s</duration>
                      <bypassPff>false</bypassPff>
                      <enableSuperterp>false</enableSuperterp>
                      <numberOfBitsPerSample>%s</numberOfBitsPerSample>
                    </userSpecification>
                  </lofar:observationAttributes>
                  <children>""" % (
                  name, descr, topo, predecessor_topo, attrname, projname, writeBoolean(TBBpiggyBack), writeBoolean(aartfaacPiggyBack),
                  writeBoolean(cordata), writeBoolean(cohdata), writeBoolean(incohdata), antenna, clock, instrfilt, interval, channels,
                  writeBoolean(cohdedisp), writeBoolean(flysEye), subsperfileCS, colapseCS, downstepsCS, whichCS, 
                  subsperfileIS, colapseIS, downstepsIS, whichIS, stations, start, stop, duration, bitspersample)

def writeXMLBeam(ofile, name, description, topo, beamtype, target, ra, dec, subbands, flyseye, tabrings, tabringsize, tablist, dataproducts):
  print >>ofile, r"""<item index="0">
                      <lofar:measurement xsi:type="lofar:BFMeasurementType">
                        <name>%s</name>
                        <description>%s</description>
                        <topology>%s</topology>
                        <currentStatus>
                          <mom2:openedStatus/>
                        </currentStatus>
                        <lofar:bfMeasurementAttributes>
                          <measurementType>%s</measurementType>
                          <specification>
                            <targetName>%s</targetName>
                            <ra>%s</ra>
                            <dec>%s</dec>
                            <equinox>J2000</equinox>
                            <duration>0</duration>
                            <subbandsSpecification>
                              <subbands>%s</subbands>
                            </subbandsSpecification>
                          <tiedArrayBeams>
                            <flyseye>%s</flyseye>
                          <nrTabRings>%s</nrTabRings>
                          <tabRingSize>%s</tabRingSize>
                            <tiedArrayBeamList>
                              %s
                            </tiedArrayBeamList>
                          </tiedArrayBeams>
                          </specification>
                        </lofar:bfMeasurementAttributes>
                        <resultDataProducts>
                          %s
                        </resultDataProducts>   
                      </lofar:measurement>
                    </item>""" % ( name, description, topo, beamtype, target, ra, dec, subbands, writeBoolean(flyseye),
                                   tabrings, tabringsize, tablist, dataproducts )
                  
def writeXMLObsEnd(ofile):
  print >> ofile, r"""</children>
                </lofar:observation>
                </item>"""

def writeTABXML(TAB):
  strVal = r""
  for i in range(0,len(TAB)):
    if TAB[i][0] == 'c':
      strVal += r"""                <tiedArrayBeam>
                  <coherent>true</coherent>
                  <angle1>%s</angle1>
                  <angle2>%s</angle2>
                </tiedArrayBeam>
                """ % (TAB[i][1],TAB[i][2])
    else:
      strVal += r"""                <tiedArrayBeam>
                  <coherent>false</coherent>
                  <dispersionMeasure>%s</dispersionMeasure>
                </tiedArrayBeam>
                """ % (TAB[i][1])
  strVal = strVal.rstrip() # strip off the last newline
  return strVal

def writeBBSParameters(ofile, bbsParameters):
  print >> ofile, r"""            <bbsParameters>
              <baselines>%s</baselines>
              <correlations>%s</correlations>
              <beamModelEnable>%s</beamModelEnable>
              <solveParms>%s</solveParms>
              <solveUVRange>%s</solveUVRange>
              <strategyBaselines>%s</strategyBaselines>
              <strategyTimeRange>%s</strategyTimeRange>
            </bbsParameters>""" % (bbsParameters[0], bbsParameters[1], writeBoolean(bbsParameters[2]), bbsParameters[3], bbsParameters[4], bbsParameters[5], bbsParameters[6])
  ##TODO % {"baselines":, "correlations":, writeBoolean("beamenable":), "solveparms":, "solveuvrange":, "strategybaselines":, "strategytimerange":}

def writeDemixParameters(ofile, demixParameters):
  print >> ofile, r"""                  <demixingParameters>
                    <averagingFreqStep>%s</averagingFreqStep>
                    <averagingTimeStep>%s</averagingTimeStep>
                    <demixFreqStep>%s</demixFreqStep>
                    <demixTimeStep>%s</demixTimeStep>
                    <demixAlways>%s</demixAlways>
                    <demixIfNeeded>%s</demixIfNeeded>
                    <ignoreTarget>%s</ignoreTarget>
                  </demixingParameters>""" % (demixParameters[0], demixParameters[1], demixParameters[2], demixParameters[3], demixParameters[4], demixParameters[5], writeBoolean(demixParameters[6])) ##TODO writeBoolean() Might be reduntant? Should do the conversion earlier
  ##TODO % {"averagingFreqStep":, "averagingTimeStep":, "demixFreqStep":, "demixTimeStep":, writeBoolean("demixAlways":), writeBoolean("demixIfNeeded":), writeBoolean("ignoreTarget":)}

def writeXMLTargetPipeline(ofile, topo, pred_topo, name, descr, defaulttemplate, flagging,
                           duration, demixParameters, bbsParameters, uvintopo, uvinname,
                           instrintopo, instrinname, uvoutname, uvouttopo, storageCluster) :
  stor_cluster = dataProductCluster(storageCluster)
  proc_cluster = processingCluster(storageCluster)
  print >> ofile, r"""<item index="0">
                  <lofar:pipeline xsi:type="lofar:CalibrationPipelineType">
                    <topology>%s</topology>
                    <predecessor_topology>%s</predecessor_topology>
                    <name>%s</name>
                    <description>%s (%s)</description>
                    <pipelineAttributes>
                      <defaultTemplate>%s</defaultTemplate>
                      <flaggingStrategy>%s</flaggingStrategy>
                      <duration>%s</duration>""" % (topo, pred_topo, name, name, descr, defaulttemplate, flagging, duration)
  writeDemixParameters(ofile, demixParameters)
  ##TODO if bbsParameters: ??
  writeBBSParameters(ofile, bbsParameters)
  if proc_cluster:
    print >> ofile, proc_cluster
  print >> ofile, r"""</pipelineAttributes>
                    <usedDataProducts>
                      <item>
                        <lofar:uvDataProduct topology="%s">
                          <name>%s</name>
                        </lofar:uvDataProduct>
                      </item>
                      <item>
                        <lofar:instrumentModelDataProduct topology="%s">
                          <name>%s</name>
                        </lofar:instrumentModelDataProduct>
                      </item>
                    </usedDataProducts>
                    <resultDataProducts>
                        <item>
                          <lofar:uvDataProduct>
                            <name>%s</name>
                            <topology>%s</topology>
                            <status>no_data</status>
                            %s
                          </lofar:uvDataProduct>
                        </item> 
                    </resultDataProducts>               
                    </lofar:pipeline>
                  </item>""" % (uvintopo, uvinname, instrintopo, instrinname, uvoutname, uvouttopo, stor_cluster)                

def writeXMLCalPipe(ofile, topo, pred_topo, name, descr, defaulttemplate, flagging, duration, skymodel, demixParameters, 
                    bbsParameters, uvintopo, instroutname, instrouttopo, uvouttopo, storageCluster) :
  stor_cluster = dataProductCluster(storageCluster)
  proc_cluster = processingCluster(storageCluster)
  print >> ofile, r"""        <item index="0">
              <lofar:pipeline xsi:type="lofar:CalibrationPipelineType">
                <topology>%s</topology>
                <predecessor_topology>%s</predecessor_topology>
                <name>%s</name>
                <description>%s (%s)</description>
                <pipelineAttributes>
                  <defaultTemplate>%s</defaultTemplate>
                  <flaggingStrategy>%s</flaggingStrategy>
                  <duration>%s</duration>
                  <skyModelDatabase>%s</skyModelDatabase>""" % (topo, pred_topo, name, name, descr, defaulttemplate, flagging, duration, skymodel)
  writeDemixParameters(ofile, demixParameters)
  ##TODO if bbsParameters: ??
  writeBBSParameters(ofile, bbsParameters)
  if proc_cluster:
    print >> ofile, proc_cluster
  print >> ofile, r"""</pipelineAttributes>
                <usedDataProducts>
                  <item>
                    <lofar:uvDataProduct topology="%s">
                    </lofar:uvDataProduct>
                  </item>
                </usedDataProducts>
                <resultDataProducts>
                  <item>
                    <lofar:instrumentModelDataProduct>
                      <name>%s</name>
                      <topology>%s</topology>
                      <status>no_data</status>
                      %s
                    </lofar:instrumentModelDataProduct>
                  </item>
                  <item>
                    <lofar:uvDataProduct>
                      <name>%s</name>
                      <topology>%s</topology>
                      <status>no_data</status>
                      %s
                    </lofar:uvDataProduct>
                  </item>
                </resultDataProducts>
              </lofar:pipeline>
            </item>""" % (uvintopo, instroutname, instrouttopo, stor_cluster, uvouttopo, uvouttopo, stor_cluster)

def writeXMLAvgPipeline(ofile, topo, pred_topo, name, descr, defaulttemplate, flagging, duration, 
                        demixParameters, uvintopo, uvouttopo, storageCluster) :
  stor_cluster = dataProductCluster(storageCluster)
  proc_cluster = processingCluster(storageCluster)
  print >> ofile, r"""        <item index="0">
              <lofar:pipeline xsi:type="lofar:AveragingPipelineType">
                <topology>%s</topology>
                <predecessor_topology>%s</predecessor_topology>
                <name>%s</name>
                <description>%s (%s)</description>
                <pipelineAttributes>
                  <defaultTemplate>%s</defaultTemplate>
                  <flaggingStrategy>%s</flaggingStrategy>
                  <duration>%s</duration>""" % (topo, pred_topo, name, name, descr, defaulttemplate, flagging, duration)
  writeDemixParameters(ofile, demixParameters)
  if proc_cluster:
    print >> ofile, proc_cluster
  print >> ofile, r"""</pipelineAttributes>
                <usedDataProducts>
                  <item>
                    <lofar:uvDataProduct topology="%s">
                    </lofar:uvDataProduct>
                  </item>
                </usedDataProducts>
                <resultDataProducts>
                  <item>
                    <lofar:uvDataProduct>
                      <name>%s</name>
                      <topology>%s</topology>
                      <status>no_data</status>
                      %s
                    </lofar:uvDataProduct>
                  </item>
                </resultDataProducts>
              </lofar:pipeline>
            </item>""" % (uvintopo, uvouttopo, uvouttopo, stor_cluster)
                   
def writeXMLPulsarPipe(ofile, topo, pred_topo, name, descr, defaulttemplate, duration, bfintopo, pouttopo, storageCluster, _2bf2fitsExtraOpts, _8bitConversionSigma, 
            decodeNblocks, decodeSigma, digifilExtraOpts, dspsrExtraOpts, dynamicSpectrumTimeAverage, nofold, nopdmp, norfi, 
            prepdataExtraOpts, prepfoldExtraOpts, prepsubbandExtraOpts, pulsar, rawTo8bit, rfifindExtraOpts, rrats, singlePulse, 
            skipDsps, skipDynamicSpectrum, skipPrepfold, tsubint) :
  stor_cluster = dataProductCluster(storageCluster)
  proc_cluster = processingCluster(storageCluster)
  print >> ofile, r"""        <item index="0">
              <lofar:pipeline xsi:type="lofar:PulsarPipelineType">
                <topology>%s</topology>
                <predecessor_topology>%s</predecessor_topology>
                <name>%s</name>
                <description>%s (%s)</description>
                <pipelineAttributes>
                  <defaultTemplate>%s</defaultTemplate>
                  <duration>%s</duration>
                  <_2bf2fitsExtraOpts>%s</_2bf2fitsExtraOpts>
                  <_8bitConversionSigma>%s</_8bitConversionSigma>
                  <decodeNblocks>%s</decodeNblocks>
                  <decodeSigma>%s</decodeSigma>
                  <digifilExtraOpts>%s</digifilExtraOpts>
                  <dspsrExtraOpts>%s</dspsrExtraOpts>
                  <dynamicSpectrumTimeAverage>%s</dynamicSpectrumTimeAverage>
                  <nofold>%s</nofold>
                  <nopdmp>%s</nopdmp>
                  <norfi>%s</norfi>
                  <prepdataExtraOpts>%s</prepdataExtraOpts>
                  <prepfoldExtraOpts>%s</prepfoldExtraOpts>
                  <prepsubbandExtraOpts>%s</prepsubbandExtraOpts>
                  <pulsar>%s</pulsar>
                  <rawTo8bit>%s</rawTo8bit>
                  <rfifindExtraOpts>%s</rfifindExtraOpts>
                  <rrats>%s</rrats>
                  <singlePulse>%s</singlePulse>
                  <skipDsps>%s</skipDsps>
                  <skipDynamicSpectrum>%s</skipDynamicSpectrum>
                  <skipPrepfold>%s</skipPrepfold>
                  <tsubint>%s</tsubint>
                  %s
                </pipelineAttributes>
                <usedDataProducts>
                  <item>
                    <lofar:bfDataProduct topology="%s">
                    </lofar:bfDataProduct>
                  </item>
                </usedDataProducts>
                <resultDataProducts>
                  <item>
                    <lofar:pulsarDataProduct>
                      <name>%s</name>
                      <topology>%s</topology>
                      <status>no_data</status>
                      %s
                    </lofar:pulsarDataProduct>
                  </item>
                </resultDataProducts>
              </lofar:pipeline>
            </item>""" % (topo, pred_topo, name, name, descr, defaulttemplate, duration, _2bf2fitsExtraOpts, _8bitConversionSigma, 
            decodeNblocks, decodeSigma, digifilExtraOpts, dspsrExtraOpts, dynamicSpectrumTimeAverage, writeBoolean(nofold), writeBoolean(nopdmp), writeBoolean(norfi), 
            prepdataExtraOpts, prepfoldExtraOpts, prepsubbandExtraOpts, pulsar, writeBoolean(rawTo8bit), rfifindExtraOpts, writeBoolean(rrats), writeBoolean(singlePulse), 
            writeBoolean(skipDsps), writeBoolean(skipDynamicSpectrum), writeBoolean(skipPrepfold), tsubint, 
            proc_cluster, bfintopo, pouttopo, pouttopo, stor_cluster)

#nv 13okt2014: #6716 - Implement Long Baseline Pipeline     
def writeXMLLongBaselinePipe(ofile, topo, pred_topo, name, descr, defaulttemplate, duration, subbands_per_subbandgroup, subbandgroups_per_ms, uvintopo, uvouttopo, storageCluster) :
  stor_cluster = dataProductCluster(storageCluster)
  proc_cluster = processingCluster(storageCluster)
  print >> ofile, r"""        <item index="0">
              <lofar:pipeline xsi:type="lofar:LongBaselinePipelineType">
                <topology>%s</topology>
                <predecessor_topology>%s</predecessor_topology>
                <name>%s</name>
                <description>%s (%s)</description>
                <pipelineAttributes>
                  <defaultTemplate>%s</defaultTemplate>
                  <duration>%s</duration>
                  <subbandsPerSubbandGroup>%s</subbandsPerSubbandGroup>
                  <subbandGroupsPerMS>%s</subbandGroupsPerMS>
                  %s
                </pipelineAttributes>
                <usedDataProducts>
                  <item>
                    <lofar:uvDataProduct topology="%s">
                    </lofar:uvDataProduct>
                  </item>
                </usedDataProducts>
                <resultDataProducts>
                  <item>
                    <lofar:uvDataProduct>
                      <name>%s</name>
                      <topology>%s</topology>
                      <status>no_data</status>
                      %s
                    </lofar:uvDataProduct>
                  </item>
                </resultDataProducts>
              </lofar:pipeline>
            </item>""" % (topo, pred_topo, name, name, descr, defaulttemplate, duration, subbands_per_subbandgroup, subbandgroups_per_ms, 
            proc_cluster, uvintopo, uvouttopo, uvouttopo, stor_cluster)  

def writeDataProducts(dataTopo, correlatedData, coherentStokesData, incoherentStokesData, storageCluster):
  strVal = r""
  if correlatedData:
    dataTopoStr = dataTopo + '.uv.dps'
    strVal += r"""                <item>
                    <lofar:uvDataProduct>
                    <name>%s</name>
                    <topology>%s</topology>
                    <status>no_data</status>
                    %s
                    </lofar:uvDataProduct>
                  </item>
                  """ % (dataTopoStr, dataTopoStr, dataProductCluster(storageCluster))
  if coherentStokesData | incoherentStokesData:
    if coherentStokesData & ~incoherentStokesData:
      dataTopoStr = dataTopo + '.cs'
    elif incoherentStokesData & ~coherentStokesData:
      dataTopoStr = dataTopo + '.is'
    else:
      dataTopoStr = dataTopo + '.csis'
    strVal += r"""                <item>
                    <lofar:bfDataProduct>
                    <name>%s</name>
                    <topology>%s</topology>
                    <status>no_data</status>
                    %s
                    </lofar:bfDataProduct>
                  </item>
                  """ % (dataTopoStr, dataTopoStr, dataProductCluster(storageCluster))               
  strVal = strVal.rstrip() # strip off the last newline
  return strVal

def writeImagingPipelineInputDataproducts(ofile, topologyList):
  print >> ofile, r"""                <usedDataProducts>"""
  for topology in topologyList:
    print >> ofile, r"""                <item>
              <lofar:uvDataProduct topology="%s">
                <name>%s</name>
              </lofar:uvDataProduct>
            </item>""" % (topology, topology)
  print >> ofile, r"""               </usedDataProducts>"""

def writeSkyImageOutputDataproduct(ofile, topology):
        print >> ofile, r"""                <resultDataProducts>
                  <item>
                    <lofar:skyImageDataProduct>
                      <name>%s</name>
                      <topology>%s</topology>
                      <status>no_data</status>
                    </lofar:skyImageDataProduct>
                  </item>
                </resultDataProducts>
              </lofar:pipeline>
            </item>""" % (topology, topology)

def writeFolderStart(ofile, packageName, packageDescription, processing):
  print >>ofile, r"""   <item index="0">
        <lofar:folder topology_parent="true">
          <topology>0</topology>
          <name>%s</name>
          <description>%s (%s)</description>
          <children>""" % (packageName, packageDescription, processing)

def writeFolderEnd(ofile):
  print >> ofile, r"""</children>
  </lofar:folder>
  </item>"""

def writeBoolean(booleanValue):
  if booleanValue == '':
    return ''
  elif booleanValue:
    return 'true'
  else:
    return 'false'
  
def toBool(strVal):
  strVal = strVal.rstrip().lstrip()
  if strVal.rstrip() in TRUE:
    return True
  elif strVal in FALSE:
    return False
  #TODO I think we want something else here?
  elif strVal == '':
    return ''
  else:
    raise GenException(strVal + " could not be represented as a boolean value")

def writeProjectStart(ofile, version, projectName):
  print >>ofile, r"""<?xml version="1.0" encoding="UTF-8"?>
  <lofar:project xmlns:lofar="http://www.astron.nl/MoM2-Lofar" xmlns:mom2="http://www.astron.nl/MoM2" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.astron.nl/MoM2-Lofar http://lofar.astron.nl:8080/mom3/schemas/LofarMoM2.xsd http://www.astron.nl/MoM2 http://lofar.astron.nl:8080/mom3/schemas/MoM2.xsd ">
  <version>%s</version>
  <template version="%s" author="Alwin de Jong,Adriaan Renting" changedBy="Adriaan Renting">
  <description>XML Template generator version %s</description>
  </template>
  <name>%s</name>
  <children>""" % (version, version, version, projectName)

def writeProjectEnd(ofile):
  print >> ofile, r"""          </children>
  </lofar:project>"""

def writeMainFolderStart(ofile, mainFolderName, mainFolderDescription):
  print >>ofile, r"""   <item index="0">
    <lofar:folder topology_parent="false">
    <name>%s</name>
    <description>%s</description>
    <children>""" % (mainFolderName, mainFolderDescription)

def writeMainFolderEnd(ofile):
  print >> ofile, r"""</children>
  </lofar:folder>
  </item>"""

def writeImagingPipelineXML(ofile, input_list, bbsParameters):        
  print >> ofile, r"""<item index="0">
        <lofar:pipeline xsi:type="lofar:%(imaging_pipe_type)s">
          <topology>%(imaging_pipe_topology)s</topology>
          <predecessor_topology>%(imaging_pipe_predecessors_string)s</predecessor_topology>
          <name>%(imaging_pipe_name)s</name>
          <description>%(imaging_pipe_name)s (Imaging pipeline beam %(beamNr)s</description>
          <imagingPipelineAttributes>
            <defaultTemplate>%(imaging_pipe_default_template)s</defaultTemplate>
            <duration>%(imaging_pipe_duration)s</duration>
            <nrOfOutputSkyImage>%(nrImages)s</nrOfOutputSkyImage>
            <imagingParameters>
              <nrSlicesPerImage>%(nrRepeats)s</nrSlicesPerImage>
              <nrSubbandsPerImage>%(nrSubbandsPerImage)s</nrSubbandsPerImage>
              <maxBaseline>%(maxBaseline)s</maxBaseline>
              <fieldOfView>%(fieldOfView)s</fieldOfView>
              <weight>%(weightingScheme)s</weight>
              <robust>%(robustParameter)s</robust>
              <iterations>%(nrOfIterations)s</iterations>
              <threshold>%(cleaningThreshold)s</threshold>
              <uvMin>%(uvMin)s</uvMin>
              <uvMax>%(uvMax)s</uvMax>
              <stokes>%(stokesToImage)s</stokes>
            </imagingParameters>""" % (input_list)
  if bbsParameters:
    writeBBSParameters(ofile, bbsParameters)
  print >> ofile, r"""
          </imagingPipelineAttributes>"""

def parseOptions(argv):
  inputfile = ''
  outputfile = ''
  
  try:
    opts, args = getopt.getopt(argv,"hi:o:",["ifile=","ofile="])
  except getopt.GetoptError:
    print 'xmlgen.py -i <inputfile> [-o <outputfile>]'
    sys.exit(2)
    
  if len(opts) == 0:
    print 'usage: xmlgen.py -i <inputfile> [-o <outputfile>]'
    sys.exit(2)    

  for opt, arg in opts:
    if opt == '-h':
      print 'usage: xmlgen.py -i <inputfile> [-o <outputfile.xml>]'
      sys.exit()
    elif opt in ("-i", "--ifile"):
      inputfile = arg
    elif opt in ("-o", "--ofile"):
      outputfile = arg

  if (outputfile == inputfile):
    raise GenException("Output file'" + outputfile + "' has the same name as inputfile")
  if len(outputfile):
    print "Writing output xml file: " + outputfile
  else:
    outputfile = splitext(inputfile)[0] + '.xml'
    print "Output file not specified, writing output xml file:'" + outputfile + "'"
  return (inputfile, outputfile)

def processInput(inputfile):
  ifile = open(inputfile, 'r')
  lines = ifile.readlines()
  header = []
  blocks = []
  block  = []
  block_count = 0
  for l in lines:
    line = l.strip()
    if line: ##skipping empty lines
      if not line[0] == "#": #skipping comments
        if "BLOCK" in line:
          if block_count == 0:
            header = block
          else:
            if len(block) > 1: #We have at least BLOCK
              blocks.append(block)
            else:
              printWarning("BLOCK %i was found to be empty" % block_count)
          block = []
          block_count += 1
        stripped_line = line.split('#')[0]
        if stripped_line: #Not sure if this can happen?
          block.append(stripped_line)
  if len(block) > 1: #We have at least BLOCK
    blocks.append(block)
  else:
    printWarning("BLOCK %i was found to be empty" % block_count)
  ifile.close()
  return (header, blocks)
  
def wrongCombiError():
  #TODO check if this list matches the actual code, replace it with a print of the define?
  raise GenException("the combination of antennaMode, clock and instrumentFilter is not a valid combination, should be one of:\n \
  LBA - 160 MHz > '10-70 MHz', '30-70 MHz'\n \
  LBA - 200 MHz > '10-90 MHz', '30-90 MHz'\n \
  HBA - 160 MHz > '170-230 MHz'\n \
  HBA - 200 MHz > '110-190 MHz', '210-250 MHz'")

def readProcessing(value):
  if value:
    processing = value
    try:
      p = PROCESSING.index(processing)+1
    except ValueError:
      raise GenException("the specified processing '" + processing + "' is not recognized. It should be one of %s" % ", ".join(PROCESSING))
    print "processing = %s" % processing
  else:
    processing = ''
  return processing

def readKeyValuePair(line):
  if not '=' in line: #TODO print line/linenumber
    raise GenException("'=' not found in line that should have one!")
  split = line.split('=')
  key = split[0].strip()
  if not key: #TODO print line/linenumber
    raise GenException("Found a line starting with '='!")
  if len(split) < 2:
    value = ''
  else:
    value = split[1].strip()
  if len(split) > 2:
    raise GenException("Found a line with multiple '='s")
  return key, value  

def readBoolKey(keyname, value):
  if value:
    key = toBool(value)
    print "%s = %s" % (keyname, value)
  else:
    raise GenException("the %s has not been specified" % keyname)
  return key

def readStringKey(keyname, value):
  if value:
    key = value
    print "%s = %s" % (keyname, value)
  else:
    raise GenException("the %s has not been specified" % keyname)
  return key

def readIntKey(keyname, value):
  if value:
    key = int(value) #TODO try: ?
    print "%s = %s" % (keyname, key)
  else:
    raise GenException("the calibratorDuration_s has not been specified")
  return key

def readFloatKey(keyname, value):
  if value:
    key = float(value) #TODO try: ?
    print "%s = %s" % (keyname, key)
  else:
    raise GenException("the calibratorDuration_s has not been specified")
  return key

def readListKey(keyname, value):
  if keyname == "whichIS": keylist = WHICH_IS
  if keyname == "whichCS": keylist = WHICH_CS
  if keyname == "imagingPipeline": keylist = IMAGING_PIPELINE_TYPES
  if keyname == "clock": keylist = CLOCK_MODES
  if keyname == "instrumentFilter": keylist = INSTRUMENT_FILTERS
  if keyname == "antennaMode": keylist = ANTENNA_MODES
  if keyname == "weightingScheme": keylist = WEIGHTING_SCHEMES
  if keyname == "calibration": keylist = CALIBRATION_MODE
  if value:
    key = value
    if key not in keylist:
      raise GenException("the %s parameter '%s' not correct. Should be one of %s" % (keyname, value, ", ".join(keylist)))
    print "%s = %s" % (keyname, key)
  else: #TODO added this as it seemed to make sense?
    raise GenException("the %s has not been specified" % keyname)
  return key

def readIntListKey(keyname, value):
  if keyname == "numberOfBitsPerSample": keylist = NUMBER_OF_BITS_PER_SAMPLE
  if value:
    key = int(value) #TODO try?
    if key not in keylist:
      raise GenException("the %s parameter '%s' not correct. Should be one of %s" % (keyname, value, str(keylist)))
    print "%s = %s" % (keyname, key)
  else: #TODO added this as it seemed to make sense?
    raise GenException("the %s has not been specified" % keyname)
  return key

def processHeader(header):
  for line in header:
    key, value = readKeyValuePair(line)
    if key == "projectName":
      projectName = readStringKey("projectName", value)
    elif key == "mainFolderName":
      mainFolderName = readOptionalStringKey("mainFolderName", value)
    elif key == "mainFolderDescription":
      mainFolderDescription = readOptionalStringKey("mainFolderDescription", value)
  return projectName, mainFolderName, mainFolderDescription

def readOptionalStringKey(keyname, value):
  if value:
    key = value
    print "%s = %s" % (keyname, value)
  else:
    printWarning("The %s has not been specified" % keyname)
    key = "" #TODO put in some dummy description?
  return key

def readPackageTag(value):
  if value:
    packageTag = value
    if len(packageTag) > 8:
      raise GenException("the package tag:'" + packageTag + "' is too long. Max 8 characters.") 
    print "package tag = %s" % packageTag
  else:
    packageTag = ''
    print "no package tag will be used."
  return packageTag

def readStartTimeUTC(value):
  if value:
    startTimeUTC = value
    startTime = datetime.strptime(startTimeUTC, '%Y-%m-%d %H:%M:%S')
    print "start time (UTC) = %s" % startTime.strftime('%b %d %Y %H:%M:%S')
    set_starttime = True
  return startTime, set_starttime

def readTimeStep(number, value):
  if value:
    timeStep = int(value)
    print "time step%i = %s seconds" % (number, timeStep)
  else:
    timeStep = ''
  return timeStep

def readStationList(value):
  if value:
    stationList = ','.join(sorted(set(value.replace('core',CORE_STATIONS).replace('superterp',SUPERTERP_STATIONS).replace('remote',REMOTE_STATIONS).replace('international',INTERNATIONAL_STATIONS).replace('all',ALL_STATIONS).replace('NL',NL_STATIONS).replace('nl',NL_STATIONS).replace('dutch',NL_STATIONS).split(','))))
    print "stations = %s" % stationList
  else:
    raise GenException("the stationList has not been specified")
  return stationList

def readCreate_extra_ncp_beam(value):
  if value:
    create_extra_ncp_beam = toBool(value) #TODO toBool can return True, False or ''
    if create_extra_ncp_beam:
      print "extra ncp beam will be created"
    else:
      print "extra ncp beam will not be created"
  else:
    raise GenException("create_extra_ncp_beam has not been specified")
  return create_extra_ncp_beam

def readGlobalBBS(value):
  globalBBS = ['','','','true','','','','']
  if value:
    valList = value.split(';')
    for i in range(0,len(valList)):
      globalBBS[i] = XMLescape(valList[i])
    globalBBS[3] = toBool(globalBBS[3])
  return globalBBS

def readImagingBBS(value):
  imagingBBS = ['','','true','','','','']
  if value:
    valList = value.split(';')
    for i in range(0,len(valList)):
      imagingBBS[i] = XMLescape(valList[i])
    imagingBBS[2] = toBool(imagingBBS[2])
  return imagingBBS

def readGlobalDemix(value):
  globalDemix = ['','','','','','','']
  if value:
    valList = value.split(';')
    for i in range(0,len(valList)):
      globalDemix[i] = valList[i]
    if (globalDemix[0] != '') and (globalDemix[2] != ''):
      if int(globalDemix[2]) % int(globalDemix[0]) <> 0: #TODO try ?
        raise GenException("demixFreqStep (" + globalDemix[2] + ") should be integer multiple of averagingFreqStep (" + globalDemix[0] + ") for globalDemix")
      globalDemix[6] = toBool(globalDemix[6]) # convert ignoreTarget to bool
  return globalDemix

def readGlobalPulsar(value):
  globalPulsar = ['','','','','','','','','','','','','','','','','','','','','','']
  if value:
    valList = re.sub(r"\s+;", ';', value) # clear white-space just before ';'
    valList = re.sub(r";\s+", ';', valList).split(';') # clear white-space directly after ';' (this method intentionally does not clear white-space in the (string) parameters self!)
    for i in range(0,len(valList)):
      globalPulsar[i] = XMLescape(valList[i])
    globalPulsar[1] = toBool(globalPulsar[1]) # singlePulse
    globalPulsar[2] = toBool(globalPulsar[2]) # rawTo8bit
    globalPulsar[7] = toBool(globalPulsar[7]) # norfi
    globalPulsar[8] = toBool(globalPulsar[8]) # nofold
    globalPulsar[9] = toBool(globalPulsar[9]) # nopdmp
    globalPulsar[10] = toBool(globalPulsar[10]) # skipDsps
    globalPulsar[11] = toBool(globalPulsar[11]) # rrats
    globalPulsar[19] = toBool(globalPulsar[19]) # skipDynamicSpectrum
    globalPulsar[20] = toBool(globalPulsar[20]) # skipPrepfold
  return globalPulsar

def readGlobalSubbands(value):
  if value:
    globalSubbands = value.replace(' ','').split(';')
    if (len(globalSubbands) == 2) and (globalSubbands[1].rstrip() != ''):
      verifySubbandList("Global_Subbands", globalSubbands[0], globalSubbands[1])
    else:
      raise GenException("Global_Subbands error: not enough parameters specified. Should be: subband list;nr_subbands")
  else:
    raise GenException("Global_Subbands specified incorrectly")
  return globalSubbands

def readGlobalTABrings(value):
  if value:
    globalTABrings = value.split(';')
    if (len(globalTABrings) == 2) and (globalTABrings[1].rstrip() != ''):
      globalTABrings[0] = int(globalTABrings[0]) # nrTABrings
      if globalTABrings[1].endswith('deg') or globalTABrings[1].endswith('d'):
        globalTABrings[1] = deg2rad(globalTABrings[1].rstrip(' deg'))
      else:
        globalTABrings[1] = float(globalTABrings[1]) # TAB ring size
  else:
    globalTABrings = []
  return globalTABrings

def findBeamSpecs(startLine, lines):
  beams = []
  beamSpec = ''
  for lineNr in range(startLine, len(lines)):
    line = lines[lineNr]
    if line[:1].isdigit(): #startswith a digit, new beam
      if beamSpec: #save previous found one
        beam = {'beam': beamSpec, 'pipelines': pipelines, "TABs":TABs}
        beams.append(beam)
      beamSpec = line
      pipelines = []
      TABs = []
    if line.startswith(('BBS','Demix','Pulsar')): #Can contain '='
      pipelines.append(line)
    elif '=' in line: #key=value pair, so end of beam spec
      break;
    if line.startswith(('TAB','c;','i;')):
      TABs.append(line)
  if beamSpec: # save last found one
    beam = {'beam': beamSpec, 'pipelines': pipelines, "TABs":TABs}
    beams.append(beam)
  return beams

def readCalibratorBeam(startLine, lines, globalSubbands, globalTABrings, globalBBS, globalDemix, globalTAB, coherentStokesData, flysEye):
  printInfo("found a calibrator beam")
  beamspecs = findBeamSpecs(startLine, lines)
  if len(beamspecs) < 1:
    raise GenException("the calibration beam is not specified")
  #TODO currently only one Calibrator Beam?
  beam      = beamspecs[0]["beam"]
  pipelines = beamspecs[0]["pipelines"]
  TABs      = beamspecs[0]["TABs"]

  nr_parms = beam.count(';') + 1
  if nr_parms > 9:
    raise GenException("too many parameters for calibrator beam: " + beam)
  elif nr_parms < 8:
    raise GenException("too few parameters for calibrator beam: " + beam)
  else:
    calibratorBeam = beam.replace(' ;',';').replace('; ',';').split(';')
    if nr_parms == 9:
      try:
        calibratorBeam[8] = int(calibratorBeam[8]) # the (optionally specified) duration of the pipeline
      except:
        raise GenException("the specified pipeline duration: " + calibratorBeam[8] + " needs to be an integer value in seconds")
    else:
      calibratorBeam.append(0)
    
    # convert coordinated HMS to degrees
    # Right Ascension
    calibratorBeam[0] = convertAngle1(calibratorBeam[0], "calibratorBeam")
    # declination
    calibratorBeam[1] = convertAngle2(calibratorBeam[1], "calibratorBeam")
    
    if not calibratorBeam[3]:
      if globalSubbands:
        calibratorBeam[3] = globalSubbands[0]
        calibratorBeam[4] = globalSubbands[1]
      else:
        raise GenException("No subbands specified for the calibrator beam")
    else:
      verifySubbandList("calibratorBeam", calibratorBeam[3], calibratorBeam[4])
    
    if not calibratorBeam[5]: # TABrings specified?
      if globalTABrings:
        calibratorBeam[5] = globalTABrings[0]
        calibratorBeam[6] = globalTABrings[1]
    else:
      calibratorBeam[5] = int(calibratorBeam[5]) # nrTABrings
      if calibratorBeam[6].endswith('deg') or calibratorBeam[6].endswith('d'):
        calibratorBeam[6] = deg2rad(calibratorBeam[6].rstrip(' deg'))
      else:
        calibratorBeam[6] = float(calibratorBeam[6]) # TAB ring size
        
    calibratorBeam[7] = toBool(calibratorBeam[7]) # create pipeline?
    create_calibrator_pipeline = calibratorBeam[7]
    print ("right ascenscion:" + str(calibratorBeam[0]) + " declination:" + str(calibratorBeam[1]) + " target:" + calibratorBeam[2] + " subbands:" + calibratorBeam[3] + " nrSubbands:" + calibratorBeam[4] + " create pipeline:" + str(calibratorBeam[7]))
    
    if create_calibrator_pipeline:
      BBSDefault      = ['','','','true','','','','']
      DemixDefault    = ['','','','','','','']
      calibratorBBS   = [] #Can now be a list of pipelines per beam
      calibratorDemix = []
      print "!!!!! " + str(len(pipelines))
      for pipeline in pipelines:
        if pipeline.startswith("BBS"):
          calibratorBBS.append(BBSDefault)
          calBBS = readExtraParms("BBS", [pipeline])
          if len(calBBS) > 0:
            for i in range(0,len(calBBS)):
              calibratorBBS[-1][i] = calBBS[i]
            calibratorBBS[-1][3] = toBool(calibratorBBS[-1][3])
          elif globalBBS != []:
            printInfo('Using global BBS settings for Calibrator beam pipeline')
            for i in range(0,len(globalBBS)):
              calibratorBBS[-1][i] = globalBBS[i]
      
        if pipeline.startswith("Demix"):
          calibratorDemix.append(DemixDefault)
          calDemix = readExtraParms("Demix", [pipeline])
          if len(calDemix) > 0:
            for i in range(0,len(calDemix)):
              calibratorDemix[-1][i] = calDemix[i]
            if (calibratorDemix[-1][0] != '') and (calibratorDemix[-1][2] != ''):
              if int(calibratorDemix[-1][2]) % int(calibratorDemix[-1][0]) <> 0:
                raise GenException("demixFreqStep (" + calibratorDemix[-1][2] + ") should be integer multiple of averagingFreqStep (" + calibratorDemix[-1][0] + ") for calibrator beam pipeline")
            calibratorDemix[-1][6] = toBool(calibratorDemix[-1][6])
          elif globalDemix != []:
              printInfo('Using global demix settings for Calibrator beam pipeline')
              for i in range(0,len(globalDemix)):
                calibratorDemix[-1][i] = globalDemix[i]
    
    calibratorTAB = readTiedArrayBeams(TABs)
    if not calibratorTAB:
      if globalTAB:
        printInfo('Using global TABs for calibrator beam')
        calibratorTAB = globalTAB #TODO check no possibility for globalTABrings?
    if coherentStokesData and not (hasCoherentTab(calibratorTAB) or flysEye):
      raise GenException("CalibratorBeam: no coherent TAB specified while coherent Stokes data requested")

    if not calibratorBBS:
      calibratorBBS.append(BBSDefault)
      if globalBBS:
        printInfo('Using global BBS settings for pipeline(s) coupled to Calibrator beam')
        calibratorBBS.append(BBSDefault)
        for i in range(0,len(globalBBS)):
          calibratorBBS[-1][i] = globalBBS[i]

    if not calibratorDemix:
      calibratorDemix.append(DemixDefault)
      if globalDemix:
        printInfo('Using global demix settings for pipeline(s) coupled to Calibrator beam')
        calibratorDemix.append(DemixDefault)
        for i in range(0,len(globalDemix)):
          calibratorDemix[-1][i] = globalDemix[i]

  return calibratorBeam, calibratorBBS, calibratorDemix, calibratorTAB, create_calibrator_pipeline

def readTargetBeams(startLine, lines, globalSubbands, globalBBS, globalDemix, globalPulsar, globalTAB, globalTABrings, coherentStokesData, flysEye, numberOfBitsPerSample):
  printInfo('found the target beams')
  beamspecs = findBeamSpecs(startLine, lines)
  if len(beamspecs) < 1:
    raise GenException("the target beams are not specified")
  targetBeams  = []
  targetTAB    = []
  targetBBS    = []
  targetDemix  = []
  targetPulsar = []
  nr_beams     = 0

  for beamspec in beamspecs:
    beam      = beamspec["beam"]
    pipelines = beamspec["pipelines"]
    TABs      = beamspec["TABs"]
    nr_parms = beam.count(';') + 1
    if nr_parms > 9:
      raise GenException("too many parameters for target beam: " + beam)
    elif nr_parms < 8:
      raise GenException("too few parameters for target beam: " + beam)
    else:
      targetBeams.append(beam.replace(' ;',';').replace('; ',';').split(';'))
      
      if nr_parms == 9:
        try:
          targetBeams[nr_beams][8] = int(targetBeams[nr_beams][8]) # the (optionally specified) duration of the pipeline
        except:
          raise GenException("the specified pipeline duration: " + targetBeams[nr_beams][8] + " needs to be an integer value in seconds")
      else:
        targetBeams[nr_beams].append(0)

      # convert coordinated HMS to degrees
      # right ascension
      targetBeams[nr_beams][0] = convertAngle1(targetBeams[nr_beams][0], "targetBeam:" + str(nr_beams))
      # declination
      targetBeams[nr_beams][1] = convertAngle2(targetBeams[nr_beams][1], "targetBeam:" + str(nr_beams))
          
      if not targetBeams[nr_beams][3]:
        if globalSubbands:
          targetBeams[nr_beams][3] = globalSubbands[0]
          targetBeams[nr_beams][4] = globalSubbands[1]
          printInfo('Using Global_Subband settings for target beam: %i' % nr_beams)
        else:
          raise GenException("No subbands specified for the calibrator beam")
      else:
        verifySubbandList("targetBeam %i" % (nr_beams+1), targetBeams[nr_beams][3], targetBeams[nr_beams][4])

      if not targetBeams[nr_beams][5]: # TABrings specified?
        if globalTABrings:
          targetBeams[nr_beams][5] = globalTABrings[0]
          targetBeams[nr_beams][6] = globalTABrings[1]
          printInfo('Using Global_TABrings settings for target beam: %i' % nr_beams)
        else:
          targetBeams[nr_beams][5] = 0
      else:
        targetBeams[nr_beams][5] = int(targetBeams[nr_beams][5])
        if targetBeams[nr_beams][5] > 0:
          if targetBeams[nr_beams][6].endswith('deg') or targetBeams[nr_beams][6].endswith('d'):
            targetBeams[nr_beams][6] = deg2rad(targetBeams[nr_beams][6].rstrip(' deg'))
          else: #TODO try?
            targetBeams[nr_beams][6] = float(targetBeams[nr_beams][6]) # TAB ring size
            
      targetBeams[nr_beams][7] = toBool(targetBeams[nr_beams][7]) # create pipeline coupled to target beam?
      print ("right ascenscion:" + str(targetBeams[nr_beams][0]) + " declination:" + str(targetBeams[nr_beams][1]) + " target:" + targetBeams[nr_beams][2] + " subbands:" + targetBeams[nr_beams][3] + " nrSubbands:" + targetBeams[nr_beams][4] 
      + " create pipeline:" + str(targetBeams[nr_beams][7]))
      
      BBSDefault    = ['','','','true','','','','']
      DemixDefault  = ['','','','','','','']
      PulsarDefault = ['','','','','','','','','','','','','','','','','','','','','','']
      targetBBS.append([]) #Can now be a list of pipelines per beam
      targetDemix.append([])
      targetPulsar.append([])
      if targetBeams[nr_beams][7]: # pipeline created?
        for pipeline in pipelines:
          if pipeline.startswith("BBS"):
            targetBBS[nr_beams].append(BBSDefault)
            tarBBS = readExtraParms("BBS", [pipeline])
            for i in range(0, len(tarBBS)):
              targetBBS[nr_beams][-1][i] = tarBBS[i]
            targetBBS[nr_beams][-1][3] = toBool(targetBBS[nr_beams][-1][3])
          
          if pipeline.startswith("Demix"):
            targetDemix[nr_beams].append(DemixDefault)
            tarDemix = readExtraParms("Demix", [pipeline])
            if len(tarDemix) >= 4:
              for i in range(0,len(tarDemix)):
                targetDemix[nr_beams][-1][i] = tarDemix[i]
              if int(targetDemix[nr_beams][-1][2]) % int(targetDemix[nr_beams][-1][0]) <> 0:
                raise GenException("demixFreqStep (" + targetDemix[nr_beams][-1][2] + ") should be integer multiple of averagingFreqStep (" + targetDemix[nr_beams][-1][0] + "), target beam pipeline:" + str(nr_beams))
              targetDemix[nr_beams][-1][6] = toBool(targetDemix[nr_beams][-1][6]) # convert ignoreTarget to bool
            elif len(tarDemix) > 0:
              raise GenException("Demixing parameters should at least have the first four averaging/demixing steps (block %s, targetBeam %s)" % (blockNr, nr_beams))
      
          if pipeline.startswith("Pulsar"):
            targetPulsar[nr_beams].append(PulsarDefault)
            tarPulsar = readExtraParms("Pulsar", [pipeline])
            if len(tarPulsar) > 0:
              for i in range(0,len(tarPulsar)):
                targetPulsar[nr_beams][-1][i] = tarPulsar[i]
              targetPulsar[nr_beams][-1][1] = toBool(targetPulsar[nr_beams][-1][1]) # singlePulse
              targetPulsar[nr_beams][-1][2] = toBool(targetPulsar[nr_beams][-1][2]) # rawTo8bit
              targetPulsar[nr_beams][-1][7] = toBool(targetPulsar[nr_beams][-1][7]) # norfi
              targetPulsar[nr_beams][-1][8] = toBool(targetPulsar[nr_beams][-1][8]) # nofold
              targetPulsar[nr_beams][-1][9] = toBool(targetPulsar[nr_beams][-1][9]) # nopdmp
              targetPulsar[nr_beams][-1][10] = toBool(targetPulsar[nr_beams][-1][10]) # skipDsps
              targetPulsar[nr_beams][-1][11] = toBool(targetPulsar[nr_beams][-1][11]) # rrats
              targetPulsar[nr_beams][-1][19] = toBool(targetPulsar[nr_beams][-1][19]) # skipDynamicSpectrum
              targetPulsar[nr_beams][-1][20] = toBool(targetPulsar[nr_beams][-1][20]) # skipPrepfold

        if not targetBBS[nr_beams]:
          targetBBS[nr_beams].append(BBSDefault)
          if globalBBS:
            printInfo('Using global BBS settings for pipeline(s) coupled to target beam:' + str(nr_beams))
            targetBBS[nr_beams].append(BBSDefault)
            for i in range(0,len(globalBBS)):
              targetBBS[nr_beams][-1][i] = globalBBS[i]

        if not targetDemix[nr_beams]:
          targetDemix[nr_beams].append(DemixDefault)
          if globalDemix:
            printInfo('Using global demix settings for pipeline(s) coupled to target beam:' + str(nr_beams))
            targetDemix[nr_beams].append(DemixDefault)
            for i in range(0,len(globalDemix)):
              targetDemix[nr_beams][-1][i] = globalDemix[i]

        if not targetPulsar[nr_beams]:
          targetPulsar[nr_beams].append(PulsarDefault)
          if globalPulsar:
            printInfo('Using global Pulsar settings for pulsar pipeline(s) coupled to target beam:' + str(nr_beams))
            targetPulsar[nr_beams].append(PulsarDefault)
            for i in range(0,len(globalPulsar)):
              targetPulsar[nr_beams][-1][i] = globalPulsar[i]
      
      tarTAB = readTiedArrayBeams(TABs)
      if tarTAB:
        targetTAB.append(tarTAB)
      elif globalTAB:
        targetTAB.append(globalTAB)
      else:
        targetTAB.append([])
      if coherentStokesData and not (hasCoherentTab(targetTAB[-1]) or (targetBeams[nr_beams][5] > 0) or flysEye):
        raise GenException("Target Beam %i: no coherent TAB specified while coherent Stokes data requested" % nr_beams)
      nr_beams += 1
  totSubbands = sum([int(targetBeams[i][4]) for i in range(len(targetBeams))])
  maxSubbands = MAX_NR_SUBBANDS[NUMBER_OF_BITS_PER_SAMPLE.index(numberOfBitsPerSample)]
  print "total subbands for all target beams = %s" % totSubbands
  if totSubbands > maxSubbands: #TODO this doesn't count the calibrator beam!
    raise GenException("the total number of subbands (%s) for all target beams exceeds the maximum number of subbands (%s) for %s bit mode" % (totSubbands, maxSubbands, numberOfBitsPerSample))
  return targetBeams, targetBBS, targetDemix, targetPulsar, targetTAB, nr_beams

def checkAntennaModeInstrumentFilterAndClockCombination(antennaMode, instrumentFilter, clock):
#TODO hardcoded values, should check against INSTRUMENT_FILTERS
  if clock == '160 MHz':
    if antennaMode.startswith('HBA'): # 160 MHz, HBA
      if instrumentFilter != '170-230 MHz':
        wrongCombiError()
    else: # 160 MHz, LBA
      if instrumentFilter not in ['10-70 MHz', '30-70 MHz']:
        wrongCombiError()
  else:
    if antennaMode.startswith('HBA'): # 200 MHz, HBA
      if instrumentFilter not in ['110-190 MHz', '210-250 MHz']:
        wrongCombiError()
    else: # 200 MHz, LBA
      if instrumentFilter not in ['10-90 MHz', '30-90 MHz']:
        wrongCombiError()

def determineNrImages(targetBeams, nrSubbandsPerImage, variableName):
  nrImages = []
  for beam in targetBeams:
    if int(beam[4]) % nrSubbandsPerImage <> 0:
      raise GenException("nrSubbands (%s) should be integer dividable by the %s (%s) for target beam %i" % (beam[4], variableName, nrSubbandsPerImage, targetBeams.index(beam)+1))
    nrImages.append(int(beam[4]) / nrSubbandsPerImage)
  return nrImages

def readBlock(lines, projectName, blockNr):
  s = { ##settings
  "set_starttime": False,
  "nrRepeats": 1,
  "globalSubbands": [],
  "globalBBS": [],
  "globalDemix": [],
  "globalPulsar": [],
  "globalTAB": [],
  "globalTABrings": [],
  "coherentStokesData": False,
  "flysEye": False,
  "numberOfBitsPerSample": 0}
  
  for lineNr, cline in enumerate(lines):
    if "=" in cline and not cline.startswith(('BBS','Demix','Pulsar')): #we skip beam and pipelines lines
      key, value = readKeyValuePair(cline)
      if key == "processing":
        s["processing"] = readProcessing(value)
      elif key == "split_targets":
        s["split_targets"] = readBoolKey("split_targets", value)
      elif key == "packageName":
        s["packageName"] = readStringKey("packageName", value)
      elif key == "packageDescription":
        s["packageDescription"] = readOptionalStringKey("packageDescription", value)
      elif key == "packageTag":
        s["packageTag"] = readPackageTag(value)
      elif key == "startTimeUTC":
        s["startTime"], s["set_starttime"] = readStartTimeUTC(value)
      elif key == "timeStep1":
        s["timeStep1"] = readTimeStep(1, value)
      elif key == "timeStep2":
        s["timeStep2"] = readTimeStep(2, value)
      elif key == "stationList":
        s["stationList"] = readStationList(value)
      elif key == "create_calibrator_observations":
        s["create_calibrator_observations"] = readBoolKey("create_calibrator_observations", value)
      elif key == "create_target_cal_beam":
        s["create_target_cal_beam"] = readBoolKey("create_target_cal_beam", value)
      elif key == "calibration":
        s["calibration_mode"] = readListKey("calibration", value)
      elif key == "create_extra_ncp_beam":
        s["create_extra_ncp_beam"] = readCreate_extra_ncp_beam(value)
      elif key == "antennaMode":
        s["antennaMode"] = readListKey("antennaMode", value)
      elif key == "clock":
        s["clock"] = readListKey("clock", value)
      elif key == "instrumentFilter":
        s["instrumentFilter"] = readListKey("instrumentFilter", value)
      elif key == "integrationTime":
        #TODO should check if it's a valid float?
        s["integrationTime"] = readStringKey("integrationTime", value)
      elif key == "correlatedData":
        s["correlatedData"] = readBoolKey("correlatedData", value)
      elif key == "coherentStokesData":
        s["coherentStokesData"] = readBoolKey("coherentStokesData", value)
      elif key == "incoherentStokesData":
        s["incoherentStokesData"] = readBoolKey("incoherentStokesData", value)
      elif key == "coherentDedisperseChannels":
        s["coherentDedisperseChannels"] = readBoolKey("coherentDedisperseChannels", value)
      elif key == "flysEye":
        s["flysEye"] = readBoolKey("flysEye", value)
      elif key == "calibratorDuration_s":
        s["calibratorDuration_s"] = readIntKey("calibratorDuration_s", value)
      elif key == "targetDuration_s":
        s["targetDuration_s"] = readIntKey("targetDuration_s", value)
      elif key == "numberOfBitsPerSample":
        s["numberOfBitsPerSample"] = readIntListKey("numberOfBitsPerSample", value)
      elif key == "channelsPerSubband":
        #TODO should this be Int?
        s["channelsPerSubband"] = readStringKey("channelsPerSubband", value)
      elif key == "subbandsPerFileCS":
        s["subbandsPerFileCS"] = readIntKey("subbandsPerFileCS", value)
      elif key == "numberCollapsedChannelsCS":
        s["numberCollapsedChannelsCS"] = readIntKey("numberCollapsedChannelsCS", value)
      elif key == "stokesDownsamplingStepsCS":
        s["stokesDownsamplingStepsCS"] = readIntKey("stokesDownsamplingStepsCS", value)
      elif key == "whichCS":
        s["whichCS"] = readListKey("whichCS", value)
      elif key == "subbandsPerFileIS":
        s["subbandsPerFileIS"] = readIntKey("subbandsPerFileIS", value)
      elif key == "numberCollapsedChannelsIS":
        s["numberCollapsedChannelsIS"] = readIntKey("numberCollapsedChannelsIS", value)
      elif key == "stokesDownsamplingStepsIS":
        s["stokesDownsamplingStepsIS"] = readIntKey("stokesDownsamplingStepsIS", value)
      elif key == "whichIS":
        s["whichIS"] = readListKey("whichIS", value)
      elif key == "nrSubbandsPerImage":
        s["nrSubbandsPerImage"] = readIntKey("nrSubbandsPerImage", value)
      elif key == "imagingPipeline":
        s["imagingPipeline"] = readListKey("imagingPipeline", value)
      elif key == "imagingDuration_s":
        s["imaging_pipe_duration"] = readIntKey("imaging_pipe_duration", value)
      elif key == "maxBaseline_m":
        s["maxBaseline"] = readIntKey("maxBaseline", value)
      elif key == "fieldOfView_deg":
        s["fieldOfView"] = readFloatKey("fieldOfView", value)
      elif key == "weightingScheme":
        s["weightingScheme"] = readListKey("weightingScheme", value)
      elif key == "robustParameter":
        s["robustParameter"] = readFloatKey("robustParameter", value)
      elif key == "nrOfIterations":
        s["nrOfIterations"] = readIntKey("nrOfIterations", value)
      elif key == "cleaningThreshold":
        s["cleaningThreshold"] = readFloatKey("cleaningThreshold", value)
      elif key == "uvMin_klambda":
        s["uvMin"] = readFloatKey("uvMin", value)
      elif key == "uvMax_klambda":
        s["uvMax"] = readFloatKey("uvMax", value)
      elif key == "stokesToImage":
        s["stokesToImage"] = readStringKey("stokesToImage", value)
      elif key == "skyModel":
        s["skyModel"] = readStringKey("skyModel", value)
      elif key == "tbbPiggybackAllowed":
        s["tbbPiggybackAllowed"] = readBoolKey("tbbPiggybackAllowed", value)
      elif key == "aartfaacPiggybackAllowed":
        s["aartfaacPiggybackAllowed"] = readBoolKey("aartfaacPiggybackAllowed", value)
      elif key == "flaggingStrategy":
        s["flaggingStrategy"] = readStringKey("flaggingStrategy", value)
      elif key == "subbandsPerSubbandGroup":
        s["subbandsPerSubbandGroup"] = readIntKey("subbandsPerSubbandGroup", value)
      elif key == "subbandGroupsPerMS":
        s["subbandGroupsPerMS"] = readIntKey("subbandGroupsPerMS", value)
      elif key == "Global_BBS":
        s["globalBBS"] = readGlobalBBS(value)
      elif key == "Imaging_BBS":
        s["imagingBBS"] = readImagingBBS(value)
      elif key == "Global_Demix":
        s["globalDemix"] = readGlobalDemix(value)
      elif key == "Global_Pulsar":
        s["globalPulsar"] = readGlobalPulsar(value)
      elif key == "Global_Subbands":
        s["globalSubbands"] = readGlobalSubbands(value)
      elif key == "Global_TAB":
        s["globalTAB"] = readTiedArrayBeams(lines, lineNr, nr_lines)
      elif key == "Global_TABrings":
        s["globalTABrings"] = readGlobalTABrings(value)
      elif key == "calibratorBeam":
        s["calibratorBeam"], s["calibratorBBS"], s["calibratorDemix"], s["calibratorTAB"], s["create_calibrator_pipeline"] = \
          readCalibratorBeam(lineNr+1, lines, s["globalSubbands"], s["globalTABrings"], s["globalBBS"], s["globalDemix"], s["globalTAB"],
                             s["coherentStokesData"], s["flysEye"])
      elif key == 'targetBeams':
        s["targetBeams"], s["targetBBS"], s["targetDemix"], s["targetPulsar"], s["targetTAB"], s["nr_beams"] = \
          readTargetBeams(lineNr+1, lines, s["globalSubbands"], s["globalBBS"], s["globalDemix"], s["globalPulsar"], s["globalTAB"],
                          s["globalTABrings"], s["coherentStokesData"], s["flysEye"], s["numberOfBitsPerSample"])
      elif key == "repeat":
        try:
          s["nrRepeats"] = int(value)
          print "number of repeats = %s" % s["nrRepeats"]
        except:
          raise GenException("the repeat parameter is not valid for BLOCK: %i" % blockNr)
      elif key == "cluster":
        s["cluster"] = readStringKey("cluster", value)
      else:
        raise GenException("unknown key:'%s' in BLOCK: %i" % (key, blockNr))
  return s ##settings

def checkSettings(settings, blockNr):
  if "calibration_mode" not in settings:
    raise GenException("the calibration parameter is not specified for BLOCK: %i" % blockNr)
  elif settings["calibration_mode"] == "internal":
    settings["create_target_cal_beam"] = True
    if not "create_calibrator_observations" in settings:
      settings["create_calibrator_observations"] = False
  elif settings["calibration_mode"] == "external":
    settings["create_calibrator_observations"] = True
  elif settings["calibration_mode"] == "none":
    settings["create_calibrator_observations"] = False
  if "split_targets" not in settings:
    raise GenException("the split_targets parameter is not specified for BLOCK: %i" % blockNr)
  if "processing" not in settings:
    raise GenException("the processing parameter has not been specified. It should be one of %s" % ", ".join(PROCESSING));
  elif settings["processing"] == 'Pulsar':
    if not (("coherentStokesData" in settings and settings["coherentStokesData"]) 
            or ("incoherentStokesData" in settings and settings["incoherentStokesData"])):
      raise GenException("Pulsar processing requires one or both of coherentStokesData / incoherentStokesData to be set for BLOCK: %i" % blockNr)
  elif settings["processing"] == 'Imaging' and settings["calibration_mode"] == "none":
    raise GenException("processing=imaging requires calibration. While calibration is set to 'none' for BLOCK: %i" % blockNr)
  if settings["nr_beams"] == 0:
    raise GenException("no target beams have been specified for BLOCK: %i" % blockNr)
  elif settings["calibration_mode"] == "none":
    settings["create_target_cal_beam"] = False
  if "packageName" not in settings:
    raise GenException("the packageName is not specified for BLOCK: %i" % blockNr)
  if "stationList" not in settings:
    raise GenException("the stationList is not specified for BLOCK: %i" % blockNr)
  if "antennaMode" not in settings:
    raise GenException("the antennaMode is not specified for BLOCK: %i" % blockNr)
  if "instrumentFilter" not in settings:
    raise GenException("the instrumentFilter is not specified for BLOCK: %i" % blockNr)
  if "integrationTime" not in settings and ("correlatedData" in settings and settings["correlatedData"]): #TODO can it be false?
    raise GenException("the integrationTime is not specified for BLOCK: %i" % blockNr)
  if settings["create_calibrator_observations"] or settings["calibration_mode"] == "external":
    if settings["calibratorDuration_s"] == 0:
      raise GenException("the calibratorDuration_s is not specified for BLOCK: %i" % blockNr)
  if (settings["calibration_mode"] != "none") and not settings["calibratorBeam"]:
      raise GenException("the calibratorBeam is not specified while calibration parameter is not set to 'none' for BLOCK: %i" % blockNr)
  if (not "calibratorBeam" in settings and settings["calibration_mode"] != "none"): # calibration_mode is no calibrator beam
      raise GenException("the calibratorBeam is not specified for BLOCK: %i" % blockNr)
  if ("targetDuration_s" not in settings):
    raise GenException("the targetDuration_s is not specified for BLOCK: %i" % blockNr)
  if "numberOfBitsPerSample" not in settings:
    raise GenException("the numberOfBitsPerSample is not specified for BLOCK: %i" % blockNr)
  if "channelsPerSubband" not in settings:
    raise GenException("the channelsPerSubband is not specified for BLOCK: %i" % blockNr)
  if "flysEye" in settings and settings["flysEye"] and not "coherentStokesData" in settings:
    raise GenException("FlysEye cannot be switched on when coherentStokesData is switched off, specified in BLOCK: %i" % blockNr)        
    
  if settings["processing"] == 'Imaging':
    if "imagingPipeline" in settings:
      if settings["imagingPipeline"] == 'none':
        settings["do_imaging"] = False
      else:
        if not "nrSubbandsPerImage" in settings:
          raise GenException("the nrSubbandsPerImage is not specified for BLOCK: %i" % blockNr)

        settings["do_imaging"] = True
        if settings["imagingPipeline"] == 'standard':
          settings["imaging_pipe_type"] = 'ImagingPipelineType'
          if settings["antennaMode"].startswith("HBA"):
            settings["imaging_pipe_default_template"] = "Imaging Pipeline HBA"
          else:
            settings["imaging_pipe_default_template"] = "Imaging Pipeline LBA"
        elif settings["imagingPipeline"] == 'MSSS':
          settings["imaging_pipe_type"] = 'ImagingPipelineMSSSType'
          settings["imaging_pipe_default_template"] = "MSSS Imaging Pipeline"
        # determine nrImages
        settings["nrImages"] = determineNrImages(settings["targetBeams"], settings["nrSubbandsPerImage"], "nrSubbandsPerImage")
    else:
      raise GenException("the 'imagingPipeline' type parameter has not been specified while processing is set to Imaging. imagingPipeline should be one of: MSSS, standard or none");
  else:
    settings["do_imaging"] = False

  if settings["processing"] == "LongBaseline": #TODO issue 8357, needs better function name
    determineNrImages(settings["targetBeams"], settings["subbandsPerSubbandGroup"], "subbandsPerSubbandGroup")
    determineNrImages(settings["targetBeams"], settings["subbandGroupsPerMS"], "subbandGroupsPerMS")

  if not "flaggingStrategy" in settings:
    if settings["antennaMode"].startswith("LBA"):
      settings["flaggingStrategy"] = "LBAdefault"
    else:
      settings["flaggingStrategy"] = "HBAdefault"
    
  checkAntennaModeInstrumentFilterAndClockCombination(settings["antennaMode"], settings["instrumentFilter"], settings["clock"])

  settings["writePackageTag"] = "packageTag" in settings and settings["packageTag"]
  return settings

def  writeImagingPipeline(ofile, nr_beams, targetBeams, blockTopo, nrRepeats,
       imaging_pipe_inputs, imaging_pipe_predecessors,
       writePackageTag, packageTag, nrImages, imagingPipelineSettings, imagingBBS):
  for key,val in imagingPipelineSettings.items(): #TODO somewhat dirty hack, to be solved better later.
    exec(key + '=val')
  for beamNr in range (0, nr_beams):
    create_pipeline = targetBeams[beamNr][7]
    if create_pipeline:
      imaging_pipe_topology = blockTopo + 'PI' + str(beamNr)        # 1.PI
      imaging_pipe_output_topology = imaging_pipe_topology + '.dps' # 1.PI.dps
      imaging_pipe_predecessors_string = ','.join(imaging_pipe_predecessors[beamNr]) #creates nrRepeats long comma separated list

      #for repeatNr in range (1, nrRepeats+1): 
        # ****** ADD AN IMAGING PIPELINE FOR EVERY TARGET BEAM ******

      if writePackageTag:
        imaging_pipe_name = packageTag + "/" + targetBeams[beamNr][2] + "/IM"
      else:
        imaging_pipe_name = targetBeams[beamNr][2] + "/IM"

      temp = {"imaging_pipe_topology":imaging_pipe_topology,
        "imaging_pipe_predecessors_string":imaging_pipe_predecessors_string,
        "imaging_pipe_name":imaging_pipe_name,
        "beamNr":beamNr, "nrImages":nrImages[beamNr], "nrRepeats":nrRepeats}
        
      writeImagingPipelineXML(ofile, merge_dicts(temp, imagingPipelineSettings), imagingBBS)          
      writeImagingPipelineInputDataproducts(ofile, imaging_pipe_inputs[beamNr])
      writeSkyImageOutputDataproduct(ofile, imaging_pipe_output_topology)

def determineBfDataExtension(coherentStokesData, incoherentStokesData):
  bfDataExtension = ''
  if coherentStokesData | incoherentStokesData:
    if coherentStokesData & ~incoherentStokesData:
      bfDataExtension = '.cs'
    elif incoherentStokesData & ~coherentStokesData:
      bfDataExtension = '.is'
    else:
      bfDataExtension = '.csis'
  return bfDataExtension

def writeRepeat(ofile, projectName, blockTopo, repeatNr, settings, imaging_pipe_inputs, imaging_pipe_predecessors):
  for key,val in settings.items(): #TODO somewhat dirty hack, to be solved better later.
    exec(key + '=val')
  repeatTopo = blockTopo + str(repeatNr)
  
  tar_obs_beam_topologies = []
  tar_obs_uv_data_topologies = []
  tar_obs_bf_data_topologies = []
  tar_pipe_topologies = []
  LB_preproc_pipe_predecessor = []
  LB_preproc_pipe_topologies = []
  tar_pipe_output_INST_topologies = []  
  tar_pipe_output_MS_topologies = []
  pulsar_pipe_output_topologies = []
  LB_preproc_pipe_output_MS_topologies = []

  #nv 13okt2014: #6716 - Implement Long Baseline Pipeline
  LB_pipeline_topologies =[]
  LB_pipeline_predecessor =[] 
  LB_pipeline_input_uv_topologies =[]
  LB_pipeline_output_uv_topologies =[]

  cal_obs_topology = repeatTopo + '.C'                   # 1.C
  cal_obs_beam0_topology = cal_obs_topology + '.SAP000'     # 1.C.SAP000
  tar_obs_topology = repeatTopo + '.T'                   # 1.T
  cal_pipe_calibrator_topology = repeatTopo + '.CPC'       # 1.CPC
  cal_pipe_target_topology = repeatTopo + '.CPT'       # 1.CPT
  
  if processing == 'Imaging':
    if calibration_mode == "internal":
      cal_obs_pipe_default_template = "Calibrator Pipeline (export)"
      cal_tar_pipe_default_template = "Calibrator Pipeline (no export)"
      cal_pipe_calibrator_description = "Cal Pipe Calibrator"
      cal_pipe_target_description = "Cal Pipe Target"
      tar_pipe_predecessor = tar_obs_topology + ',' + cal_pipe_target_topology # 1.T,1.CPT
      tar_pipe_input_INST_topo = cal_pipe_target_topology + '.inst.dps'               # 1.P1.dps
    elif calibration_mode == "external":
      cal_obs_pipe_default_template = "Calibrator Pipeline (export)"
      cal_tar_pipe_default_template = "Calibrator Pipeline (no export)"
      cal_pipe_calibrator_description = "Cal Pipe Calibrator"
      cal_pipe_target_description = "Cal Pipe Target"
      tar_pipe_predecessor = tar_obs_topology + ',' + cal_pipe_calibrator_topology # 1.T,1.CPC
      tar_pipe_input_INST_topo = cal_pipe_calibrator_topology + '.inst.dps'         # 1.CPC.inst.dps
  elif processing == 'Preprocessing':
    tar_pipe_predecessor = tar_obs_topology # 1.T
    tar_pipe_input_INST_topo = ''   # no input instrument models for these modes
    cal_obs_pipe_default_template = "Preprocessing Pipeline"
    cal_tar_pipe_default_template = "Preprocessing Pipeline"
    cal_pipe_calibrator_description = "Preprocessing"
    cal_pipe_target_description = "Preprocessing"
  elif processing == 'Calibration':
    tar_pipe_predecessor = tar_obs_topology # 1.T
    tar_pipe_input_INST_topo = ''   # no input instrument models for these modes
    cal_obs_pipe_default_template = "Calibration Pipeline"
    cal_tar_pipe_default_template = "Calibration Pipeline"
    cal_pipe_calibrator_description = "Calibration"
    cal_pipe_target_description = "Calibration"
  elif processing == 'Pulsar':
    #pulsar_pipe_predecessor = tar_obs_topology
    pulsar_pipe_default_template = "Pulsar Pipeline"
  elif processing == 'LongBaseline':
    if calibration_mode == "internal": # internal calibration (previously Calbeam)
      cal_obs_pipe_default_template = "Calibrator Pipeline (export)"
      cal_tar_pipe_default_template = "Calibrator Pipeline (no export)"
      cal_pipe_calibrator_description = "Cal Pipe Calibrator"
      cal_pipe_target_description = "Cal Pipe Target"
      tar_pipe_predecessor = tar_obs_topology + ',' + cal_pipe_target_topology # 1.T,1.CPT
      tar_pipe_input_INST_topo = cal_pipe_target_topology + '.inst.dps'               # 1.P1.dps
    elif calibration_mode == "external": # external calibration (previously calObs)
      cal_obs_pipe_default_template = "Calibrator Pipeline (export)"
      cal_tar_pipe_default_template = "Calibrator Pipeline (no export)"
      cal_pipe_calibrator_description = "Cal Pipe Calibrator"
      cal_pipe_target_description = "Cal Pipe Target"
      tar_pipe_predecessor = tar_obs_topology + ',' + cal_pipe_calibrator_topology # 1.T,1.CPC
      tar_pipe_input_INST_topo = cal_pipe_calibrator_topology + '.inst.dps'         # 1.CPC.inst.dps

  bfDataExtension = determineBfDataExtension(coherentStokesData, incoherentStokesData)

  for beamNr in range (0, nr_beams):
    beam_nr_str = str(beamNr)
    if create_calibrator_observations:
      if writePackageTag:
        cal_obs_name = packageTag + "/" + calibratorBeam[2] + "/" + str(repeatNr) + "/CO"
      else:
        cal_obs_name = calibratorBeam[2] + "/" + str(repeatNr) + "/CO"
    
    # TODO: for multiObs this is not ok. The SAP numbers should start from scratch again with every new target observation
    # and there should be a .beamnr added before the .SAP in the topology
    # this work has to be done when multiObs with multiple SAPs per target observation is implemented.
    tar_obs_beam_topologies.append(tar_obs_topology + ".SAP" + beam_nr_str.rjust(3,'0'))

    tar_obs_bf_data_topologies.append(tar_obs_beam_topologies[beamNr] + bfDataExtension)
    tar_obs_uv_data_topologies.append(tar_obs_beam_topologies[beamNr] + ".uv.dps")
        
    tar_pipe_topologies.append(repeatTopo + ".PT" + beam_nr_str)
    tar_pipe_output_INST_topologies.append(tar_pipe_topologies[beamNr] + ".inst.dps")
    tar_pipe_output_MS_topologies.append(tar_pipe_topologies[beamNr] + ".uv.dps")
    pulsar_pipe_output_topologies.append(tar_pipe_topologies[beamNr] + ".pu.dps")

    if processing == 'LongBaseline':
      LB_preproc_pipe_topologies.append(repeatTopo + ".PTLB" + beam_nr_str)
      LB_preproc_pipe_output_MS_topologies.append(LB_preproc_pipe_topologies[beamNr] + ".uv.dps")
      LB_preproc_pipe_predecessor.append(tar_pipe_topologies[beamNr])

      #nv 13okt2014: #6716 - Implement Long Baseline Pipeline
      LB_pipeline_topologies.append(repeatTopo + ".LBP" + beam_nr_str)
      LB_pipeline_predecessor.append(LB_preproc_pipe_topologies[beamNr])
      LB_pipeline_input_uv_topologies.append(LB_preproc_pipe_output_MS_topologies[beamNr])
      LB_pipeline_output_uv_topologies.append(LB_pipeline_topologies[beamNr] + ".uv.dps")

    if do_imaging:
      imaging_pipe_inputs[beamNr].append(tar_pipe_output_MS_topologies[beamNr])
      imaging_pipe_predecessors[beamNr].append(tar_pipe_topologies[beamNr])

  if "create_extra_ncp_beam" in settings and settings["create_extra_ncp_beam"]:
    tarObsCalBeamDataTopoStr = tar_obs_topology + ".SAP%03i" % (nr_beams+1,)
  else:
    tarObsCalBeamDataTopoStr = tar_obs_topology + ".SAP%03i" % (nr_beams,)
  tar_obs_beam_topologies.append(tarObsCalBeamDataTopoStr)
  tar_obs_bf_data_topologies.append(tarObsCalBeamDataTopoStr + bfDataExtension)
  tar_obs_uv_data_topologies.append(tarObsCalBeamDataTopoStr + ".uv.dps")  
      
  tar_obs_predecessor = ''
  if create_calibrator_observations:
    tar_obs_predecessor = cal_obs_topology      # 1.C
    if set_starttime:
      startTimeStr = startTimeObs.strftime('%Y-%m-%dT%H:%M:%S')
      endTimeStr = (startTimeObs + timedelta(seconds=calibratorDuration_s)).strftime('%Y-%m-%dT%H:%M:%S')
    else:
      startTimeStr = ''
      endTimeStr = ''
      
    writeXMLObs(ofile, cal_obs_name, cal_obs_name + ' (Calibration Observation)', cal_obs_topology, '', cal_obs_name,
      projectName, tbbPiggybackAllowed, aartfaacPiggybackAllowed, correlatedData, coherentStokesData, incoherentStokesData,
      antennaMode, clock, instrumentFilter, integrationTime, channelsPerSubband, coherentDedisperseChannels, flysEye,
      subbandsPerFileCS, numberCollapsedChannelsCS,
      stokesDownsamplingStepsCS, whichCS, subbandsPerFileIS, numberCollapsedChannelsIS, stokesDownsamplingStepsIS, whichIS,
      stationList, startTimeStr, endTimeStr, calibratorDuration_s, numberOfBitsPerSample)
    writeXMLBeam(ofile, calibratorBeam[2], calibratorBeam[2], cal_obs_beam0_topology, 'Calibration', calibratorBeam[2],
      calibratorBeam[0], calibratorBeam[1], calibratorBeam[3], flysEye, str(calibratorBeam[5]), str(calibratorBeam[6]),
      writeTABXML(calibratorTAB), writeDataProducts(cal_obs_beam0_topology, correlatedData, coherentStokesData,
      incoherentStokesData, cluster) )
    writeXMLObsEnd(ofile)
      
  # target start and end time:
  if set_starttime:
    if create_calibrator_observations:
      startTimeObs = startTimeObs + timedelta(seconds=timeStep1+calibratorDuration_s)
    startTimeStr = startTimeObs.strftime('%Y-%m-%dT%H:%M:%S')
    endTimeStr = (startTimeObs + timedelta(seconds=targetDuration_s)).strftime('%Y-%m-%dT%H:%M:%S')
  else:
    startTimeStr = ''
    endTimeStr = ''

  if create_calibrator_observations and create_calibrator_pipeline:
    
    if writePackageTag:
      cal_pipe_name = packageTag + "/" + calibratorBeam[2] + "/" + str(repeatNr) + "/CPC"
    else:
      cal_pipe_name = calibratorBeam[2] + "/" + str(repeatNr) + "/CPC"
      
    if processing == 'Imaging' or processing == 'LongBaseline':
      if not calibratorBBS:
        raise GenException("BBS SkyModel is not specified for pipeline coupled to calibrator beam")

      writeXMLCalPipe(ofile, cal_pipe_calibrator_topology, cal_obs_topology, cal_pipe_name,
        cal_pipe_calibrator_description, cal_obs_pipe_default_template,
        flaggingStrategy, calibratorBeam[8], calibratorBBS[0][0], calibratorDemix[0], calibratorBBS[0][1:],
        cal_obs_beam0_topology + '.uv.dps', cal_pipe_calibrator_topology + '.inst.dps',
        cal_pipe_calibrator_topology + '.inst.dps', cal_pipe_calibrator_topology + '.uv.dps', cluster)

    elif processing == 'Preprocessing':
      for i in range(0,len(calibratorDemix)):
        if len(calibratorDemix) > 1: #TODO a cludge right now, but want to refactor how to call the writeXML soon
          cal_pipe_calibrator_topology = cal_pipe_calibrator_topology + ".%i" % i
          cal_pipe_name = cal_pipe_name + ".%i" % i
        writeXMLAvgPipeline(ofile, cal_pipe_calibrator_topology, cal_obs_topology, cal_pipe_name, cal_pipe_calibrator_description, 
          cal_obs_pipe_default_template, flaggingStrategy, calibratorBeam[8], calibratorDemix[i],
          cal_obs_beam0_topology + '.uv.dps', cal_pipe_calibrator_topology + '.uv.dps', cluster) ##FIXME cal_pipe_calibrator_topology + '.uv.dps'

    elif processing == 'Calibration':
      
      if not calibratorBBS:
        raise GenException("BBS SkyModel is not specified for pipeline coupled to calibrator beam")
      
      #TODO ['', '', '', '', '', '', ''] is really ugly, this will break the regression test
      writeXMLCalPipe(ofile, cal_pipe_calibrator_topology, cal_obs_topology, cal_pipe_name, cal_pipe_calibrator_description, 
        cal_obs_pipe_default_template, flaggingStrategy, calibratorBeam[8], calibratorBBS[0][0], calibratorDemix[0], 
        ['', '', '', '', '', '', ''], cal_obs_beam0_topology + '.uv.dps', cal_pipe_calibrator_topology + '.inst.dps', 
        cal_pipe_calibrator_topology + '.inst.dps', cal_pipe_calibrator_topology + '.uv.dps', cluster)
   
  if not split_targets:  
    if writePackageTag:
      tar_obs_name = packageTag + "/" + targetBeams[0][2] + "/" + str(repeatNr) + "/TO"
    else:
      tar_obs_name = targetBeams[0][2] + "/" + str(repeatNr) + "/TO"

    writeXMLObs(ofile, tar_obs_name, tar_obs_name + ' (Target Observation)', tar_obs_topology, tar_obs_predecessor, 
          tar_obs_name, projectName, tbbPiggybackAllowed,
          aartfaacPiggybackAllowed, correlatedData, coherentStokesData, incoherentStokesData, antennaMode,
          clock, instrumentFilter, integrationTime, channelsPerSubband, coherentDedisperseChannels, flysEye,
          subbandsPerFileCS, numberCollapsedChannelsCS, stokesDownsamplingStepsCS, whichCS, subbandsPerFileIS,
          numberCollapsedChannelsIS, stokesDownsamplingStepsIS, whichIS, stationList, startTimeStr, endTimeStr,
          targetDuration_s, numberOfBitsPerSample)
      
    if set_starttime:
      if create_calibrator_observations:
        startTimeObs = startTimeObs + timedelta(seconds=timeStep2+targetDuration_s)
      else:
        startTimeObs = startTimeObs + timedelta(seconds=timeStep1+targetDuration_s)

    for beamNr in range(0, nr_beams):
      writeXMLBeam(ofile, targetBeams[beamNr][2], targetBeams[beamNr][2], tar_obs_beam_topologies[beamNr], 'Target', targetBeams[beamNr][2],
        targetBeams[beamNr][0], targetBeams[beamNr][1], targetBeams[beamNr][3], flysEye, targetBeams[beamNr][5],
        targetBeams[beamNr][6], writeTABXML(targetTAB[beamNr]), 
        writeDataProducts(tar_obs_beam_topologies[beamNr], correlatedData, coherentStokesData, incoherentStokesData, cluster) )

    # create the extra polarization beam?
    if "create_extra_ncp_beam" in settings and settings["create_extra_ncp_beam"]:
      polBeamTopo = tar_obs_topology + ".SAP" + str(beamNr+1).rjust(3,'0')
      writeXMLBeam(ofile, targetBeams[0][2], targetBeams[0][2], targetBeams[0][2], 'Target', targetBeams[0][0], flysEye,
      targetBeams[0][5], targetBeams[0][6], writeTABXML(targetTAB[0]),
      writeDataProducts(polBeamTopo, correlatedData, coherentStokesData, incoherentStokesData, cluster) )

    # create a calibrator beam in the target observation?
    if create_target_cal_beam:
      if "create_extra_ncp_beam" in settings and settings["create_extra_ncp_beam"]:
        calBeamTopo = tar_obs_topology + ".SAP" + str(beamNr+2).rjust(3,'0')
      else:
        calBeamTopo = tar_obs_topology + ".SAP" + str(beamNr+1).rjust(3,'0')
      
      writeXMLBeam(ofile, calibratorBeam[2], calibratorBeam[2], calBeamTopo, 'Calibration', calibratorBeam[2],
              calibratorBeam[0], calibratorBeam[1], calibratorBeam[3], flysEye, calibratorBeam[5],
              calibratorBeam[6], writeTABXML(calibratorTAB),
              writeDataProducts(tar_obs_beam_topologies[nr_beams], correlatedData, coherentStokesData, incoherentStokesData, cluster) )
      
      writeXMLObsEnd(ofile)
              
      if writePackageTag:
        cal_pipe_target_name = packageTag + "/" + calibratorBeam[2] + "/" + str(repeatNr) + "/CPT"
      else:
        cal_pipe_target_name = calibratorBeam[2] + "/" + str(repeatNr) + "/CPT"
      
      create_pipeline = calibratorBeam[7]
      if create_pipeline:
        if processing == 'Imaging' or processing == 'LongBaseline':
          if not calibratorBBS:
            raise GenException("BBS SkyModel is not specified for pipeline coupled to calibration beam")

          writeXMLCalPipe(ofile, cal_pipe_target_topology, tar_obs_topology, cal_pipe_target_name, cal_pipe_target_description,
          cal_tar_pipe_default_template, flaggingStrategy, calibratorBeam[8], calibratorBBS[0][0], calibratorDemix[0],
          calibratorBBS[0][1:], tar_obs_uv_data_topologies[nr_beams], cal_pipe_target_topology + '.inst.dps',
          cal_pipe_target_topology + '.inst.dps', cal_pipe_target_topology + '.uv.dps', cluster)
        
        elif processing == 'Preprocessing':
          for i in range(0, len(calibratorDemix)):
            if len(calibratorDemix) > 1: #TODO a cludge right now, but want to refactor how to call the writeXML soon
              cal_pipe_target_topology = cal_pipe_target_topology + ".%i" % i
              cal_pipe_target_name     = cal_pipe_target_name + ".%i" % i
            writeXMLAvgPipeline(ofile, cal_pipe_target_topology, tar_obs_topology, cal_pipe_target_name, cal_pipe_target_description,
              cal_tar_pipe_default_template, flaggingStrategy, calibratorBeam[8], calibratorDemix[i],
              tar_obs_uv_data_topologies[nr_beams], cal_pipe_target_topology + '.uv.dps', cluster) ##FIXME cal_pipe_target_topology + '.uv.dps'
        
        elif processing == 'Calibration':
          
          if not calibratorBBS:
            raise GenException("BBS SkyModel is not specified for pipeline coupled to calibration beam")

          writeXMLCalPipe(ofile, cal_pipe_target_topology, tar_obs_topology, cal_pipe_target_name, cal_pipe_target_description,
            cal_tar_pipe_default_template, flaggingStrategy, calibratorBeam[8], calibratorBBS[0][0], calibratorDemix[0],
            calibratorBBS[0][1:], tar_obs_uv_data_topologies[nr_beams], cal_pipe_target_topology + '.inst.dps',
            cal_pipe_target_topology + '.inst.dps', cal_pipe_target_topology + '.uv.dps', cluster)
    else:    
      writeXMLObsEnd(ofile)
      
  else: # split target sources into separate observations
    for beamNr in range(0, nr_beams):
      if writePackageTag:
        tar_obs_name = packageTag + "/" + targetBeams[beamNr][2] + "/" + str(repeatNr) + "/TO"
      else:
        tar_obs_name = targetBeams[beamNr][2] + "/" + str(repeatNr) + "/TO"

      tar_obs_topology_MultiObs = tar_obs_topology + '.' + str(beamNr)
      writeXMLObs(ofile, tar_obs_name, tar_obs_name + ' (Target Observation)', tar_obs_topology_MultiObs, '', tar_obs_name, projectName, tbbPiggybackAllowed,
          aartfaacPiggybackAllowed, correlatedData, coherentStokesData, incoherentStokesData, antennaMode,
          clock, instrumentFilter, integrationTime, channelsPerSubband, coherentDedisperseChannels, flysEye,
          subbandsPerFileCS, numberCollapsedChannelsCS, stokesDownsamplingStepsCS, whichCS, subbandsPerFileIS,
          numberCollapsedChannelsIS, stokesDownsamplingStepsIS, whichIS, stationList, startTimeStr, endTimeStr,
          targetDuration_s, numberOfBitsPerSample)
          
      writeXMLBeam(ofile, targetBeams[beamNr][2], targetBeams[beamNr][2], tar_obs_beam_topologies[beamNr], 'Target', targetBeams[beamNr][2],
        targetBeams[beamNr][0], targetBeams[beamNr][1], targetBeams[beamNr][3], flysEye, targetBeams[beamNr][5],
        targetBeams[beamNr][6], writeTABXML(targetTAB[beamNr]), 
        writeDataProducts(tar_obs_beam_topologies[beamNr], correlatedData, coherentStokesData, incoherentStokesData, cluster) )
          
      writeXMLObsEnd(ofile)
      
      if set_starttime:
        startTimeObs = startTimeObs + timedelta(seconds=timeStep1+targetDuration_s)


  # Target PIPELINES generation from here on
       
  for beamNr in range(0, nr_beams):
    create_pipeline = targetBeams[beamNr][7]
    if create_pipeline:
      tar_pipe_ID = "/TP"
      if processing == 'Imaging': # imaging modes
        tar_pipe_default_template = "Calibration Pipeline Target"
        tar_pipe_description = "Target Pipeline"
      elif processing == 'Preprocessing':
        tar_pipe_default_template = "Preprocessing Pipeline"
        tar_pipe_description = "Preprocessing"
      elif processing == 'Calibration':
        tar_pipe_default_template = "Calibration Pipeline"
        tar_pipe_description = "Calibration"
      elif processing == 'Pulsar':
        tar_pipe_default_template = "Pulsar Pipeline"
        tar_pipe_description = "Pulsar Pipeline"
        tar_pipe_ID = "/PP"
      elif processing == 'LongBaseline':
        tar_pipe_default_template = "Calibration Pipeline Target"
        tar_pipe_description = "Target Pipeline"

      if writePackageTag:
        tar_pipe_name = packageTag + "/" + targetBeams[beamNr][2] + "/" + str(repeatNr) + "." + str(beamNr) + tar_pipe_ID
      else:
        tar_pipe_name = targetBeams[beamNr][2] + "/" + str(repeatNr) + "." + str(beamNr) + tar_pipe_ID
        
      if processing == 'Imaging' or processing == 'LongBaseline':
        writeXMLTargetPipeline(ofile, tar_pipe_topologies[beamNr], tar_pipe_predecessor, tar_pipe_name, 
          tar_pipe_description, tar_pipe_default_template,
          flaggingStrategy, targetBeams[beamNr][8], targetDemix[beamNr][0],
          targetBBS[beamNr][0][1:], tar_obs_uv_data_topologies[beamNr], 
          tar_obs_uv_data_topologies[beamNr], tar_pipe_input_INST_topo, tar_pipe_input_INST_topo, 
          tar_pipe_output_MS_topologies[beamNr], tar_pipe_output_MS_topologies[beamNr], cluster)
            
      elif processing == 'Preprocessing':
        for i in range(0,len(targetDemix[beamNr])):
          if len(targetDemix[beamNr]) > 1: #TODO a cludge right now, but want to refactor how to call the writeXML soon
            tar_pipe_topology = tar_pipe_topologies[beamNr] + ".%i" % i
            tar_pipe_name     = tar_pipe_name + ".%i" % i
          else:
            tar_pipe_topology = tar_pipe_topologies[beamNr]
          writeXMLAvgPipeline(ofile, tar_pipe_topology, tar_pipe_predecessor, tar_pipe_name, 
            tar_pipe_description, tar_pipe_default_template,
            flaggingStrategy, targetBeams[beamNr][8], targetDemix[beamNr][i],
            tar_obs_uv_data_topologies[beamNr], tar_pipe_output_MS_topologies[beamNr], cluster) ##FIXME tar_pipe_output_MS_topologies[beamNr]

      elif processing == 'Calibration': #TODO currently doesn't work according to Alwin's wiki, why?
        if targetBBS[beamNr][0][0] == '':
          raise GenException("BBS SkyModel is not specified for pipeline coupled to target beam " + str(beamNr))
        
        writeXMLCalPipe(ofile, tar_pipe_topologies[beamNr], tar_pipe_predecessor, tar_pipe_name, 
          tar_pipe_description, tar_pipe_default_template, 
          flaggingStrategy, targetBeams[beamNr][8], targetBBS[beamNr][0][0], targetDemix[beamNr][0],
          targetBBS[beamNr][0][1:], tar_obs_uv_data_topologies[beamNr], 
          tar_pipe_output_INST_topologies[beamNr], tar_pipe_output_INST_topologies[beamNr], tar_pipe_output_MS_topologies[beamNr], cluster)
      elif processing == 'Pulsar':
        #tar_obs_topology_MultiObs = tar_obs_topology + '.' + str(beamNr)
        tar_pipe_predecessor = tar_obs_topology
        
        writeXMLPulsarPipe(ofile, tar_pipe_topologies[beamNr], tar_obs_topology, tar_pipe_name, 
        tar_pipe_description, tar_pipe_default_template, targetBeams[beamNr][8],
        tar_obs_bf_data_topologies[beamNr], pulsar_pipe_output_topologies[beamNr], cluster,
            pulsar                  = targetPulsar[beamNr][0][0],
            singlePulse             = targetPulsar[beamNr][0][1], 
            rawTo8bit               = targetPulsar[beamNr][0][2], 
            dspsrExtraOpts          = targetPulsar[beamNr][0][3],
            prepdataExtraOpts       = targetPulsar[beamNr][0][4],
            _8bitConversionSigma    = targetPulsar[beamNr][0][5],
            tsubint                 = targetPulsar[beamNr][0][6],
            norfi                   = targetPulsar[beamNr][0][7],
            nofold                  = targetPulsar[beamNr][0][8],
            nopdmp                  = targetPulsar[beamNr][0][9],
            skipDsps                = targetPulsar[beamNr][0][10],
            rrats                   = targetPulsar[beamNr][0][11],
            _2bf2fitsExtraOpts      = targetPulsar[beamNr][0][12],
            decodeSigma             = targetPulsar[beamNr][0][13],
            decodeNblocks           = targetPulsar[beamNr][0][14],
            rfifindExtraOpts        = targetPulsar[beamNr][0][15],
            prepfoldExtraOpts       = targetPulsar[beamNr][0][16],
            prepsubbandExtraOpts    = targetPulsar[beamNr][0][17],
            dynamicSpectrumTimeAverage = targetPulsar[beamNr][0][18],
            skipDynamicSpectrum     = targetPulsar[beamNr][0][19],
            skipPrepfold            = targetPulsar[beamNr][0][20],
            digifilExtraOpts        = targetPulsar[beamNr][0][21])
        
  # for long baseline processsing an additional (special purpose adapted) preprocessing pipeline is necessary
  if processing == 'LongBaseline':
    LB_preproc_pipe_template = 'Preprocessing LB'
    LB_preproc_pipe_description = 'Phaseshift + adding CS stations'

    LB_pipeline_default_template = "Long-Baseline Pipeline"
    LB_pipeline_description = "Long-Baseline Concat"
    
    for beamNr in range(0, nr_beams):
      if writePackageTag:
        LB_preproc_pipe_name = packageTag + "/" + targetBeams[beamNr][2] + "/" + str(repeatNr) + "." + str(beamNr) + "/PP"
        LB_pipeline_name = packageTag + "/" + targetBeams[beamNr][2] + "/" + str(repeatNr) + "." + str(beamNr) + "/LBP"
      else:
        LB_preproc_pipe_name = targetBeams[beamNr][2] + "/" + str(repeatNr) + "." + str(beamNr) + "/PP"
        LB_pipeline_name = targetBeams[beamNr][2] + "/" + str(repeatNr) + "." + str(beamNr) + "/LBP"

      writeXMLAvgPipeline(ofile, LB_preproc_pipe_topologies[beamNr], LB_preproc_pipe_predecessor[beamNr], LB_preproc_pipe_name,
      LB_preproc_pipe_description, LB_preproc_pipe_template, flaggingStrategy, targetBeams[beamNr][8], targetDemix[beamNr][0],
      tar_pipe_output_MS_topologies[beamNr], LB_preproc_pipe_output_MS_topologies[beamNr], cluster)

      #nv 13okt2014: #6716 - Implement Long Baseline Pipeline     
      writeXMLLongBaselinePipe(ofile, LB_pipeline_topologies[beamNr], LB_pipeline_predecessor[beamNr], LB_pipeline_name,
      LB_pipeline_description, LB_pipeline_default_template, targetBeams[beamNr][8], subbandsPerSubbandGroup, subbandGroupsPerMS,
      LB_pipeline_input_uv_topologies[beamNr], LB_pipeline_output_uv_topologies[beamNr], cluster)
  return imaging_pipe_inputs, imaging_pipe_predecessors

def writeBlock(ofile, settings, projectName, blockNr):
  defaults = {
  "subbandsPerFileCS": '',
  "numberCollapsedChannelsCS": '',
  "stokesDownsamplingStepsCS": '',
  "whichCS": '',
  "subbandsPerFileIS": '',
  "numberCollapsedChannelsIS": '',
  "stokesDownsamplingStepsIS": '',
  "whichIS": '',
  "tbbPiggybackAllowed":True,
  "aartfaacPiggybackAllowed":True,
  "imagingBBS": '',
  "cluster":'CEP2'}
  defaults.update(settings) #FIXME somewhat dirty hack, to be solved better later.
  settings = defaults
  
  #There's a lot of stuff in settings that's only relevant to the imaging pipelines
  #otherSettings = { key: settings[key] for key not in imagingPipelineKeys }
       
  writeFolderStart(ofile, settings["packageName"], settings["packageDescription"], settings["processing"])
          
  if settings["set_starttime"]:
    settings["startTimeObs"] = settings["startTime"]
  
  imaging_pipe_inputs = [[] for i in range(settings["nr_beams"])]
  imaging_pipe_predecessors = [[] for i in range(settings["nr_beams"])]
      
  blockTopo = "B%i." % (blockNr-1,)
  for repeatNr in range (1, settings["nrRepeats"]+1):
     imaging_pipe_inputs, imaging_pipe_predecessors = writeRepeat(ofile, projectName, blockTopo, repeatNr, settings, imaging_pipe_inputs, imaging_pipe_predecessors)

  if settings["do_imaging"]:
    imagingPipelineKeys = ["imaging_pipe_type", "imaging_pipe_default_template", "imaging_pipe_duration",
                           "nrSubbandsPerImage", "maxBaseline", "fieldOfView", "weightingScheme",
                           "robustParameter", "nrOfIterations", "cleaningThreshold",
                           "uvMin", "uvMax", "stokesToImage"]
    for key in imagingPipelineKeys: #Can this be done with list comprehension as well?
      if key not in settings.keys():
        settings[key] = ''
    imagingPipelineSettings = { key: settings[key] for key in imagingPipelineKeys }
    writeImagingPipeline(ofile, settings["nr_beams"], settings["targetBeams"], blockTopo,
      settings["nrRepeats"], imaging_pipe_inputs, imaging_pipe_predecessors,
      settings["writePackageTag"], settings["packageTag"], settings["nrImages"],
      imagingPipelineSettings, settings["imagingBBS"])

  writeFolderEnd(ofile)
                
def main(argv):
  try:
    inputfile, outputfile = parseOptions(argv)
    ofile = open(outputfile, 'w')
  
    header, blocks = processInput(inputfile)
  
    projectName, mainFolderName, mainFolderDescription = processHeader(header)
    writeProjectStart(ofile, VERSION, projectName)
    if mainFolderName:
      writeMainFolderStart(ofile, mainFolderName, mainFolderDescription)
    for index, block in enumerate(blocks):
      printMessage("\nProcessing BLOCK %i" % (index+1))
      settings = readBlock(block, projectName, index+1)
      settings = checkSettings(settings, index+1)
      writeBlock(ofile, settings, projectName, index+1)
    if mainFolderName:
      writeMainFolderEnd(ofile)
    writeProjectEnd(ofile)
    #TODO make things not write to the ofile directly
    # for b in block:
    #   output += generateBlock()
    #  ofile.write(output)
    ofile.close()
  except:
    import traceback
    traceback.print_exc(file=sys.stdout)
    print "something went wrong here, now aborting"
    ofile.close()
    exit(1)
  

    
if __name__ == "__main__":
  main(sys.argv[1:])
    
