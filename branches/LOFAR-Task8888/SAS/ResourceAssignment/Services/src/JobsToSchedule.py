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

from lofar.messaging import FromBus, ToBus, RPC, LofarMessage
from lofar.parameterset import PyParameterValue
from lofar.sas.otdb.OTDBBusListener import OTDBBusListener
from lofar.common.util import waitForInterrupt

""" Prefix that is common to all parset keys, depending on the exact source. """
PARSET_PREFIX="ObsSW."

def predecessors( parset ):
  """ Extract the list of predecessor obs IDs from the given parset. """

  key = parset[PARSET_PREFIX + "Observation.Scheduler.predecessors"]
  strlist = PyParameterValue(parset_dict[key]).getStringVector()

  # Key contains "Lxxxxx" values, we want to have "xxxxx" only
  result = [int(filter(str.isdigit,x)) for x in strlist]

  return result

def resourceIndicatorsFromParset( parset ):
  """ Extract the parset keys that are required for resource assignment. """

  subset = {}

  def get(key, default=None):
    """ Return the value of parset key `key', or `default' if the key
        is not defined. """
    return parset.get(PARSET_PREFIX + key, default)

  def add(key, conversion=lambda x: x):
    """ Add the given key to our subset selection, using an optional
        conversion. """
    value = get(key)
    if value is not None:
      subset[key] = conversion(value)

  """ Some conversion functions for common parameter-value types."""
  def strvector(value):
    return PyParameterValue(value).getStringVector()

  def intvector(value):
    return PyParameterValue(value).getIntVector()

  def bool(value):
    return PyParameterValue(value).getBool()

  # =====================================
  # Parset meta info
  # =====================================
  add(parset["Version.number"])

  # =====================================
  # Observation settings
  # =====================================
  add("Observation.sampleClock")
  add("Observation.nrBitsPerSample")
  add("Observation.antennaSet")
  add("Observation.VirtualInstrument.stationList", strvector)
  add("Observation.startTime")
  add("Observation.stopTime")
  add("Observation.nrBeams")
  nrSAPs = get("Observation.nrBeams", 0)
  for sap in xrange(0, nrSAPs):
    add("Observation.Beam[%d].subbandList" % (sap,), intvector)

  # =====================================
  # Correlator settings
  # =====================================
  add("Observation.DataProducts.Output_Correlated.enabled", bool)
  add("Cobalt.Correlator.integrationTime")
  add("Cobalt.Correlator.nrChannelsPerSubband")
  # TODO: We need a service that computes these 3 values
  add("Cobalt.Correlator.nrBlocksPerIntegration")
  add("Cobalt.Correlator.nrIntegrationsPerBlock")
  add("Cobalt.blockSize")

  # =====================================
  # Beamformer settings
  # =====================================
  add("Observation.DataProducts.Output_IncoherentStokes.enabled", bool)
  add("Observation.DataProducts.Output_CoherentStokes.enabled", bool)
  add("Cobalt.BeamFormer.flysEye", bool)
  #add("Cobalt.BeamFormer.CoherentStokes.nrChannelsPerSubband") # only needed to determine Cobalt.blockSize
  add("Cobalt.BeamFormer.CoherentStokes.subbandsPerFile")
  add("Cobalt.BeamFormer.CoherentStokes.timeIntegrationFactor")
  add("Cobalt.BeamFormer.CoherentStokes.which")
  #add("Cobalt.BeamFormer.IncoherentStokes.nrChannelsPerSubband") # only needed to determine Cobalt.blockSize
  add("Cobalt.BeamFormer.IncoherentStokes.subbandsPerFile")
  add("Cobalt.BeamFormer.IncoherentStokes.timeIntegrationFactor")
  add("Cobalt.BeamFormer.IncoherentStokes.which")
  for sap in xrange(0, nrSAPs):
    add("Observation.Beam[%d].nrTabRings" % (sap,))

    nrTABs = get("Observation.Beam[%d].nrTiedArrayBeams" % (sap,), 0)
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

class JobsToSchedule(OTDBBusListener):
  def __init__(self, parset_busname=None, sendbus_name=None, **kwargs):
    super(JobsToSchedule, self).__init__(**kwargs)

    self.parset_rpc = RPC(service="TaskSpecification", busname=parset_busname)
    self.send_bus   = ToBus(sendbus_name)

  def start_listening(self, **kwargs):
    self.parset_rpc.open()
    self.send_bus.open()

    super(JobsToSchedule, self).start_listening(**kwargs)

  def stop_listening(self, **kwargs):
    super(JobsToSchedule, self).stop_listening(**kwargs)

    self.send_bus.close()
    self.parset_rpc.close()

  def onObservationPrescheduled(self, treeId, modificationTime):
    # Request the parset
    main_obsID  = treeId
    main_parset = self.parset_rpc( { "OtdbID": main_obsID } )

    # Construct a dict of all the parsets we retrieved
    parsets = {}
    parsets[main_obsID] = main_parset

    # Collect the initial set of predecessors
    request_obsIDs = set(predecessors(main_parset))

    # Iterate recursively over all known predecessor obsIDs, and request their parsets
    while not request_obsIDs.empty():
        obsID = request_obsIDs.pop()

        if obsID in parsets:
            # Predecessor lists can overlap -- we already have this one
            continue

        # Request predecessor parset
        parsets[obsID] = self.parset_rpc( { "OtdbID": obsID } )

        # Add the list of predecessors
        request_obsIDs = request_obsIDs.union(predecessors(parsets[obsID]))

    # Convert parsets to resource indicators
    resourceIndicators = dict([(obsID, resourceIndicatorsFromParset(parset)) for (obsID,parset) in parsets.iteritems()])

    # Construct and send result message
    result = {
      "sasID": main_obsID,
      "state": main_state,
      "time_of_change": modificationTime,
      "resource_indicators": resourceIndicators,
    }

    # Put result on bus
    msg = LofarMessage(content=result)
    self.send_bus.send(msg)

if __name__ == "__main__":
    import sys
    from optparse import OptionParser

    # Check the invocation arguments
    parser = OptionParser("%prog -S busname -P busname -B busname [options]")
    parser.add_option("-S", "--statusbus", dest="statusbus", type="string", default="",
                      help="Busname or queue-name the status changes are published on")
    parser.add_option("-P", "--parsetbus", dest="parsetbus", type="string", default="",
                      help="Busname or queue-name a parset can be requested from")
    parser.add_option("-B", "--busname", dest="busname", type="string", default="",
                      help="Busname or queue-name the status changes are published on")
    (options, args) = parser.parse_args()

    if not options.statusbus or not options.parsetbus or not options.busname:
        parser.print_help()
        sys.exit(1)

    with JobsToSchedule(busname=options.statusbus, parset_busname=options.parsetbus, send_busname=options.busname) as jts:
        waitForInterrupt()

