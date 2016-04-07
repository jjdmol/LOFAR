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

from lofar.messaging import FromBus, ToBus, EventMessage # RPC,
from lofar.parameterset import PyParameterValue
from lofar.sas.otdb.OTDBBusListener import OTDBBusListener
from lofar.common.util import waitForInterrupt
from lofar.sas.resourceassignment.ratootdbtaskspecificationpropagator.otdbrpc import OTDBRPC
from lofar.sas.otdb.config import DEFAULT_OTDB_NOTIFICATION_BUSNAME, DEFAULT_OTDB_NOTIFICATION_SUBJECT
from lofar.sas.resourceassignment.rataskspecified.config import DEFAULT_RA_TASK_SPECIFIED_NOTIFICATION_BUSNAME
from lofar.sas.resourceassignment.rataskspecified.config import DEFAULT_RA_TASK_SPECIFIED_NOTIFICATION_SUBJECT
from lofar.sas.otdb.config import DEFAULT_OTDB_SERVICE_BUSNAME, DEFAULT_OTDB_SERVICENAME

import logging
logger = logging.getLogger(__name__)

""" Prefix that is common to all parset keys, depending on the exact source. """
PARSET_PREFIX="ObsSW."

def convertSchedulerProcessSubtype(processSubType):
    '''convert scheduler processSubType as defined in SAS/Scheduler/src/OTDBTree.h to RA (type, subtype) tuple'''
    if processSubType == "Averaging Pipeline":
        return "pipeline", "averaging pipeline"
    elif processSubType == "Calibration Pipeline":
        return "pipeline", "calibration pipeline"
    elif processSubType == "Imaging Pipeline":
        return "pipeline", "imaging pipeline"
    elif processSubType == "Imaging Pipeline MSSS":
        return "pipeline", "imaging pipeline msss"
    elif processSubType == "Long Baseline Pipeline":
        return "pipeline", "long baseline pipeline"
    elif processSubType == "Pulsar Pipeline":
        return "pipeline", "pulsar pipeline"
    elif processSubType == "Beam Observation":
        return "observation", "bfmeasurement"
    elif processSubType == "Interferometer":
        return "observation", "interferometer"
    elif processSubType == "TBB (piggyback)":
        return "observation", "tbbmeasurement"
    elif processSubType == "TBB (standalone)":
        return "observation", "tbbmeasurement"
    ##TODO Maintenance and Reservation
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
    def __init__(self,
                   otdb_notification_busname=DEFAULT_OTDB_NOTIFICATION_BUSNAME,
                   otdb_notification_subject=DEFAULT_OTDB_NOTIFICATION_SUBJECT,
                   otdb_service_busname=DEFAULT_OTDB_SERVICE_BUSNAME,
                   otdb_service_subject=DEFAULT_OTDB_SERVICENAME,
                   notification_busname=DEFAULT_RA_TASK_SPECIFIED_NOTIFICATION_BUSNAME,
                   notification_subject=DEFAULT_RA_TASK_SPECIFIED_NOTIFICATION_SUBJECT,
                   broker=None, **kwargs):
        super(RATaskSpecified, self).__init__(busname=otdb_notification_busname, subject=otdb_notification_subject, **kwargs)
        self.otdbrpc  = OTDBRPC(busname=otdb_service_busname, servicename=otdb_service_subject, broker=broker) ## , ForwardExceptions=True hardcoded in RPCWrapper right now
        self.send_bus = ToBus("%s/%s" % (notification_busname, notification_subject))

    def start_listening(self, **kwargs):
        self.otdbrpc.open()
        self.send_bus.open()
        super(RATaskSpecified, self).start_listening(**kwargs)

    def stop_listening(self, **kwargs):
        super(RATaskSpecified, self).stop_listening(**kwargs)
        self.send_bus.close()
        self.otdbrpc.close()

    def get_predecessors(self, parset):
        """ Extract the list of predecessor obs IDs from the given parset. """

        key = PARSET_PREFIX + "Observation.Scheduler.predecessors"
        stringlist = PyParameterValue(str(parset[key]), True).getStringVector()

        # Key contains values starting with 'S' = Scheduler, 'L'/'T' = OTDB, 'M' = MoM
        # 'S' we can probably ignore? Might be only internal in the Scheduler?
        result = set()
        for s in stringlist:
            try: # Made the source a string for readability, but it's not efficient
                if s.startswith('M'):
                    result.add({'source': 'mom', 'id': int(s[1:])})
                elif s.startswith('L') or s.startswith('T'):
                    result.add({'source': 'otdb', 'id': int(s[1:])})
                else: # 'S'
                    logger.info("found a predecessor ID I can't handle: %s" % s)
                    result.add({'source': 'other', 'id': int(s[1:])})
            except ValueError:
                logger.warning("found a predecessor ID that I can't parse %s" % s)
        return result

    def get_specification_with_predecessors(self, id, id_source, state, found_parsets):
        logger.info("Processing ID %s from %s" % (id, id_source))
        if id_source == "other":
            return None
        elif id_source == "mom":
            otdb_id = self.otdbrpc.taskGetIDs( mom_id=id )['otdb_id']
        elif id_source == "otdb":
            otdb_id = id
        else:
            logger.warning("Error in understanding id %s", id)
      
        logger.info("Processing OTDB ID %s", otdb_id)
        result = {"otdb_id": otdb_id, "predecessors": []} 
        if state:
            result["state"] = state # TODO should be status not state
        else:
            pass #otdbrpc.taskGetStatus not implemented and maybe not needed?
          
        if otdb_id in found_parsets:
            parset = found_parsets[otdb_id]
        else:
            parset = self.otdbrpc.taskGetSpecification( otdb_id=otdb_id )['specification']
            found_parsets[otdb_id] = parset
          
        logger.info("parset [%s]: %s" % (otdb_id, parset))
        result['specification'] = resourceIndicatorsFromParset(parset)
      
        key = PARSET_PREFIX + "Observation.processSubtype"
        result['task_type'], result['task_subtype'] = convertSchedulerProcessSubtype(parset.get(key, ""))

        logger.info("Processing predecessors")
        predecessor_ids = self.get_predecessors(parset)
        for id in predecessor_ids:
            predecessor_result = self.get_specification_with_predecessors(id['id'], id['source'], "", found_parsets)
            if predecessor_result:
                result["predecessors"].append(predecessor_result)
        return result

    def onObservationPrescheduled(self, main_id, modificationTime):
        # Construct root node of tree
        resultTree = self.get_specification_with_predecessors(main_id, "otdb", "prescheduled", {})
        logger.info("Sending result: %s" % resultTree)

        # Put result on bus
        msg = EventMessage(content=resultTree)
        self.send_bus.send(msg)
        logger.info("Result sent")

def main():
    import logging
    import sys
    from optparse import OptionParser
    from lofar.common.util import waitForInterrupt

    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)

    # Check the invocation arguments
    parser = OptionParser("%prog [options]",
                          description="run the rataskspecified service")
    parser.add_option("-b", "--notification_bus", dest="notification_bus", type="string",
                      default=DEFAULT_RA_TASK_SPECIFIED_NOTIFICATION_BUSNAME,
                      help="Bus or queue we publish resource requests on. [default: %default]")
    parser.add_option("-s", "--notification_subject", dest="notification_subject", type="string",
                      default=DEFAULT_RA_TASK_SPECIFIED_NOTIFICATION_SUBJECT,
                      help="The subject of the event messages which this service publishes. [default: %default]")
    parser.add_option("--otdb_notification_bus", dest="otdb_notification_bus", type="string",
                      default=DEFAULT_OTDB_NOTIFICATION_BUSNAME,
                      help="Bus or queue where the OTDB notifications are published. [default: %default]")
    parser.add_option("--otdb_notification_subject", dest="otdb_notification_subject", type="string",
                      default=DEFAULT_OTDB_NOTIFICATION_SUBJECT,
                      help="Subject of OTDB notifications on otdb_notification_bus. [default: %default]")
    parser.add_option("--otdb_request_bus", dest="otdb_request_bus", type="string",
                      default=DEFAULT_OTDB_SERVICE_BUSNAME,
                      help="Bus or queue where the OTDB requests are handled. [default: %default]")
    parser.add_option('-q', '--broker', dest='broker', type='string', default=None, help='Address of the qpid broker, default: localhost')
    (options, args) = parser.parse_args()

    with RATaskSpecified(otdb_notification_busname=options.otdb_notification_bus,
                         otdb_notification_subject=options.otdb_notification_subject,
                         otdb_service_busname=options.otdb_request_bus,
                         otdb_service_subject=DEFAULT_OTDB_SERVICENAME, ##TODO parse this from command line
                         notification_busname=options.notification_bus,
                         notification_subject=options.notification_subject,
                         broker=options.broker):
        waitForInterrupt()


if __name__ == "__main__":
    main()
