#!/usr/bin/env python
#coding: iso-8859-15
#
# Copyright (C) 2015
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
"""
Daemon that listens to OTDB status changes to PRESCHEDULED and SCHEDULED, requests
the parset of such jobs (+ their predecessors), and posts them on the bus.
"""

from lofar.messaging import FromBus, ToBus, RPC, EventMessage
from lofar.parameterset import PyParameterValue
from lofar.sas.otdb.OTDBBusListener import OTDBBusListener
from lofar.common.util import waitForInterrupt

import logging
logger = logging.getLogger(__name__)

""" Prefix that is common to all parset keys, depending on the exact source. """
PARSET_PREFIX="ObsSW."

def predecessors( parset ):
  """ Extract the list of predecessor obs IDs from the given parset. """

  key = PARSET_PREFIX + "Observation.Scheduler.predecessors"
  strlist = PyParameterValue(str(parset[key]), True).getStringVector()

  # Key contains "Lxxxxx" values, we want to have "xxxxx" only
  result = [int(filter(str.isdigit,x)) for x in strlist]

  return result

def convertSchedulerProcessSubtype(processSubType):
    '''convert scheduler processSubType as defined in SAS/Scheduler/src/OTDBTree.h to RA (type, subtype) tuple'''
    if processSubType == "Averaging Pipeline":
        return "Pipeline", "Averaging Pipeline"
    elif processSubType == "Calibration Pipeline":
        return "Pipeline", "Calibration Pipeline"
    elif processSubType == "Imaging Pipeline":
        return "Pipeline", "Imaging Pipeline"
    elif processSubType == "Imaging Pipeline MSSS":
        return "Pipeline", "Imaging Pipeline MSSS"
    elif processSubType == "Long Baseline Pipeline":
        return "Pipeline", "Long Baseline Pipeline"
    elif processSubType == "Pulsar Pipeline":
        return "Pipeline", "Pulsar Pipeline"
    elif processSubType == "Beam Observation":
        return "Observation", "BFMeasurement"
    elif processSubType == "Interferometer":
        return "Observation", "Interferometer"
    elif processSubType == "TBB (piggyback)":
        return "Observation", "TBBMeasurement"
    elif processSubType == "TBB (standalone)":
        return "Observation", "TBBMeasurement"

    return "unknown", "unknown"

def resourceIndicatorsFromParset( parsetDict ):
  """ Extract the parset keys that are required for resource assignment. """

  subset = {}

  def get(key, default=None):
    """ Return the value of parset key `key', or `default' if the key
        is not defined. """
    return parsetDict.get(PARSET_PREFIX + key, default)

  def add(key, conversion=lambda x: x):
    """ Add the given key to our subset selection, using an optional
        conversion. """
    value = get(key)
    if value is not None:
      subset[key] = conversion(value)

  """ Some conversion functions for common parameter-value types."""
  def strvector(value):
    return PyParameterValue(str(value), True).getStringVector()

  def intvector(value):
    return PyParameterValue(str(value), True)._expand().getIntVector()

  def bool(value):
    return PyParameterValue(str(value), True).getBool()

  # =====================================
  # Parset meta info
  # =====================================
  subset["Version.number"] = parsetDict.get("Version.number")

  # =====================================
  # Observation settings
  # =====================================
  add("Observation.momID")
  add("Observation.sampleClock")
  add("Observation.nrBitsPerSample")
  add("Observation.antennaSet")
  add("Observation.VirtualInstrument.stationList", strvector)
  add("Observation.startTime")
  add("Observation.stopTime")
  add("Observation.nrBeams")

  nrSAPs = int(get("Observation.nrBeams", 0))
  for sap in xrange(0, nrSAPs):
    add("Observation.Beam[%d].subbandList" % (sap,), intvector)

  # =====================================
  # Correlator settings
  # =====================================
  add("Observation.DataProducts.Output_Correlated.enabled", bool)
  add("Observation.ObservationControl.OnlineControl.Cobalt.Correlator.integrationTime")
  add("Observation.ObservationControl.OnlineControl.Cobalt.Correlator.nrChannelsPerSubband")
  # TODO: We need a service that computes these 3 values
  add("Cobalt.Correlator.nrBlocksPerIntegration")
  add("Cobalt.Correlator.nrIntegrationsPerBlock")
  add("Cobalt.blockSize")


  # =====================================
  # Beamformer settings
  # =====================================
  add("Observation.DataProducts.Output_IncoherentStokes.enabled", bool)
  add("Observation.DataProducts.Output_CoherentStokes.enabled", bool)
  add("Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.flysEye", bool)
  #add("Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.CoherentStokes.nrChannelsPerSubband") # only needed to determine Cobalt.blockSize
  add("Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.CoherentStokes.subbandsPerFile")
  add("Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.CoherentStokes.timeIntegrationFactor")
  add("Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.CoherentStokes.which")
  #add("Observation.ObservationControl.OnlineControl.Cobalt.IncoherentStokes.nrChannelsPerSubband") # only needed to determine Cobalt.blockSize
  add("Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.IncoherentStokes.subbandsPerFile")
  add("Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.IncoherentStokes.timeIntegrationFactor")
  add("Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.IncoherentStokes.which")
  for sap in xrange(0, nrSAPs):
    add("Observation.Beam[%d].nrTabRings" % (sap,))

    add("Observation.Beam[%d].nrTiedArrayBeams" % (sap,))
    nrTABs = int(get("Observation.Beam[%d].nrTiedArrayBeams" % (sap,), 0))
    for tab in xrange(0, nrTABs):
      add("Observation.Beam[%d].TiedArrayBeam[%d].coherent" % (sap,tab), bool)

  # =====================================
  # Pipeline settings
  # =====================================
  # Calibrator / Averaging pipelines
  add("Observation.DataProducts.Output_Correlated.enabled", bool)
  add("Observation.DataProducts.Output_InstrumentModel.enabled", bool)
  add("Observation.DataProducts.Input_Correlated.enabled", bool)
  add("Observation.DataProducts.Input_Correlated.skip", intvector)
  add("Observation.ObservationControl.PythonControl.DPPP.demixer.demixfreqstep")
  add("Observation.ObservationControl.PythonControl.DPPP.demixer.demixtimestep")

  # Imaging pipeline
  add("Observation.DataProducts.Output_SkyImage.enabled", bool)
  add("Observation.ObservationControl.PythonControl.Imaging.slices_per_image")
  add("Observation.ObservationControl.PythonControl.Imaging.subbands_per_image")

  # Long-baseline pipeline
  add("Observation.ObservationControl.PythonControl.LongBaseline.subbandgroups_per_ms")
  add("Observation.ObservationControl.PythonControl.LongBaseline.subbands_per_subbandgroup")

  # Pulsar pipeline
  add("Observation.DataProducts.Output_Pulsar.enabled", bool)
  add("Observation.DataProducts.Input_CoherentStokes.enabled", bool)
  add("Observation.DataProducts.Input_CoherentStokes.skip", intvector)
  add("Observation.DataProducts.Input_IncoherentStokes.enabled", bool)
  add("Observation.DataProducts.Input_IncoherentStokes.skip", intvector)

  return subset

class RATaskSpecified(OTDBBusListener):
  def __init__(self, servicename, otdb_listen_busname=None, otdb_listen_subject=None, otdb_request_busname=None, my_busname=None, broker=None, **kwargs):
    super(RATaskSpecified, self).__init__(busname=otdb_listen_busname, subject=otdb_listen_subject, broker=broker, **kwargs)

    self.parset_rpc = RPC(service="OTDBService.TaskGetSpecification", busname=otdb_request_busname, broker='10.149.96.6')
    self.send_bus   = ToBus("%s/%s" % (my_busname, servicename), broker=broker)

  def start_listening(self, **kwargs):
    self.parset_rpc.open()
    self.send_bus.open()

    super(RATaskSpecified, self).start_listening(**kwargs)

  def stop_listening(self, **kwargs):
    super(RATaskSpecified, self).stop_listening(**kwargs)

    self.send_bus.close()
    self.parset_rpc.close()

  def onObservationPrescheduled(self, treeId, modificationTime):
    logger.info("Processing obs ID %s", treeId)

    # Request the parset
    main_obsID  = treeId
    main_parset,_ = self.parset_rpc( OtdbID=main_obsID )
    main_parset = main_parset['TaskSpecification']
    logger.info("main_parset [%s]: %s" % (main_obsID, main_parset))

    # Construct a dict of all the parsets we retrieved
    parsets = {}
    parsets[main_obsID] = main_parset

    logger.info("Processing predecessors")

    # Collect the initial set of predecessors
    request_obsIDs = set(predecessors(main_parset))

    logger.info("Processing %s", request_obsIDs)

    obsId2predId = {int(main_obsID):list(request_obsIDs)}

    # Iterate recursively over all known predecessor obsIDs, and request their parsets
    while request_obsIDs:
        obsID = request_obsIDs.pop()

        if obsID in parsets:
            # Predecessor lists can overlap -- we already have this one
            continue

        logger.info("Fetching predecessor %s", obsID)

        # Request predecessor parset
        parsets[obsID],_ = self.parset_rpc( OtdbID=obsID )
        parsets[obsID] = parsets[obsID]['TaskSpecification']
        #logger.info("predecessor parset [%s]: %s" % (obsID, parsets[obsID]))

        # Add the list of predecessors
        predecessor_ids = predecessors(parsets[obsID])
        request_obsIDs = request_obsIDs.union(predecessor_ids)
        obsId2predId[obsID] = predecessor_ids
        logger.info("obsID %s: preds: %s" % (obsID, predecessor_ids))
        logger.info("obsId2predId %s" % (obsId2predId))

    # Convert parsets to resource indicators
    logger.info("Extracting resource indicators")
    specifications = dict([(obsID, resourceIndicatorsFromParset(parset)) for (obsID,parset) in parsets.iteritems()])

    # recursive method to build the tree of obs and its predecessors
    def appendChildNodes(treeNode):
        node_otdb_id = treeNode['otdb_id']
        node_pred_otdb_ids = obsId2predId[node_otdb_id]
        node_pred_specifications = {pred_id:specifications[pred_id] for pred_id in node_pred_otdb_ids}

        for pred_id, pred_specification in node_pred_specifications.items():
            childNode = {
            "otdb_id": pred_id,
            "specification": pred_specification,
            "predecessors": []
            }
            childNode['task_type'], childNode['task_subtype'] = convertSchedulerProcessSubtype(parsets[pred_id].get(PARSET_PREFIX+"Observation.processSubtype", ""))

            appendChildNodes(childNode)
            treeNode["predecessors"].append(childNode)

    # Construct root node of tree
    resultTree = {
      "otdb_id": int(main_obsID),
      "state": "prescheduled",
      "specification": specifications[main_obsID],
      "predecessors": []
    }
    resultTree['task_type'], resultTree['task_subtype'] = convertSchedulerProcessSubtype(main_parset.get(PARSET_PREFIX+"Observation.processSubtype", ""))

    #recursively append predecessors as child nodes
    appendChildNodes(resultTree)

    logger.info("Sending result: %s" % resultTree)

    # Put result on bus
    msg = EventMessage(content=resultTree)
    self.send_bus.send(msg)

    logger.info("Result sent")


def main():
    import sys
    import logging
    from optparse import OptionParser
    from lofar.common.util import waitForInterrupt
    from lofar.sas.resourceassignment.rataskspecified.config import DEFAULT_NOTIFICATION_BUSNAME, RATASKSPECIFIED_NOTIFICATIONNAME
    DEFAULT_OTDB_NOTIFICATION_BUSNAME = 'lofar.otdb.status'
    DEFAULT_OTDB_NOTIFICATION_SUBJECT = 'otdb.treestatus'
    DEFAULT_OTDB_REQUEST_BUSNAME = 'lofar.otdb.specification'

    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)

    # Check the invocation arguments
    parser = OptionParser("%prog [options]",
                          description="run the rataskspecified service")
    parser.add_option("-b", "--notification_bus", dest="notification_bus", type="string", default=DEFAULT_NOTIFICATION_BUSNAME,
                      help="Bus or queue we publish resource requests on")
    parser.add_option("-s", "--notification_subject", dest="notification_subject", type="string", default=RATASKSPECIFIED_NOTIFICATIONNAME,
                      help="The subject of the event messages which this service publishes")
    parser.add_option("--otdb_notification_bus", dest="otdb_notification_bus", type="string", default=DEFAULT_OTDB_NOTIFICATION_BUSNAME,
                      help="Bus or queue where the OTDB notifications are published")
    parser.add_option("--otdb_notification_subject", dest="otdb_notification_subject", type="string", default=DEFAULT_OTDB_NOTIFICATION_SUBJECT,
                      help="Subject of OTDB notifications on otdb_notification_bus")
    parser.add_option("--otdb_request_bus", dest="otdb_request_bus", type="string", default=DEFAULT_OTDB_REQUEST_BUSNAME,
                      help="Bus or queue where the OTDB requests are handled")
    parser.add_option('-q', '--broker', dest='broker', type='string', default=None, help='Address of the qpid broker, default: localhost')
    (options, args) = parser.parse_args()

    with RATaskSpecified(RATASKSPECIFIED_NOTIFICATIONNAME,
                         otdb_listen_busname=options.otdb_notification_bus,
                         otdb_listen_subject=options.otdb_notification_subject,
                         otdb_request_busname=options.otdb_request_bus,
                         my_busname=options.notification_bus,
                         broker=options.broker):
        waitForInterrupt()


if __name__ == "__main__":
    main()
