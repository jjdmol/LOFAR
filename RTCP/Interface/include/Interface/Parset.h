//#  Parset.h: class/struct that holds the Parset information
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_INTERFACE_PARSET_H
#define LOFAR_INTERFACE_PARSET_H

// \file
// class/struct that holds the Parset information

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/ParameterSet.h>
#include <Common/StreamUtil.h>
#include <Common/StringUtil.h>
#include <Common/lofar_datetime.h>
#include <Common/LofarLogger.h> 
#include <Interface/BeamCoordinates.h>
#include <Interface/Config.h>
#include <Interface/OutputTypes.h>
#include <Stream/Stream.h>

#include <algorithm>
#include <sstream>
#include <vector>
#include <string>

#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/format.hpp>

namespace LOFAR {
namespace RTCP {

// The Parset class is a public struct that can be used as base-class
// for holding Parset related information.
// It can be instantiated with a parset containing Parset information.
class Parset: public ParameterSet
{
  public:
    Parset();
    Parset(const char *name);
    Parset(Stream *);
     
    std::string			name() const;
    void			check() const;

    void			write(Stream *) const;

    unsigned			observationID() const;
    double			startTime() const;
    double			stopTime() const;
    unsigned			nrStations() const;
    unsigned			nrTabStations() const;
    unsigned			nrMergedStations() const;
    unsigned			nrBaselines() const;
    unsigned			nrCrossPolarisations() const;
    unsigned			clockSpeed() const; // Hz
    double			sampleRate() const;
    double			sampleDuration() const;
    unsigned			nrBitsPerSample() const;
    std::vector<double>		positions() const;
    std::string			positionType() const;
    std::vector<double>		getRefPhaseCentre() const;
    std::vector<double>		getPhaseCentreOf(const std::string &name) const;
    double			dispersionMeasure() const;
    unsigned			dedispersionFFTsize() const;
    unsigned			CNintegrationSteps() const;
    unsigned			IONintegrationSteps() const;
    unsigned			coherentStokesTimeIntegrationFactor() const;
    unsigned			incoherentStokesTimeIntegrationFactor() const;
    unsigned			coherentStokesChannelsPerSubband() const;
    unsigned			incoherentStokesChannelsPerSubband() const;
    double			CNintegrationTime() const;
    double			IONintegrationTime() const;
    unsigned			nrSubbandSamples() const;
    unsigned			nrSubbandsPerPset() const; 
    unsigned			nrSubbandsPerPart() const; 
    unsigned			nrPartsPerStokes() const; 
    unsigned			nrBeamsPerPset() const; 
    unsigned			nrHistorySamples() const;
    unsigned			nrSamplesToCNProc() const;
    unsigned			inputBufferSize() const; // in samples
    unsigned			maxNetworkDelay() const;
    unsigned			nrPPFTaps() const;
    unsigned			nrChannelsPerSubband() const;
    unsigned			nrCoresPerPset() const;
    std::vector<unsigned>	usedCoresInPset() const;
    std::vector<unsigned>	phaseOneTwoCores() const;
    std::vector<unsigned>	phaseThreeCores() const;
    double			channelWidth() const;
    bool			delayCompensation() const;
    unsigned			nrCalcDelays() const;
    bool			correctClocks() const;
    double			clockCorrectionTime(const std::string &station) const;
    bool			correctBandPass() const;
    bool			hasStorage() const;
    std::string			stationName(int index) const;
    std::vector<std::string>	allStationNames() const;
    unsigned			nrPsetsPerStorage() const;
    unsigned			getLofarStManVersion() const;
    std::vector<unsigned>	phaseOnePsets() const;
    std::vector<unsigned>	phaseTwoPsets() const;
    std::vector<unsigned>	phaseThreePsets() const;
    std::vector<unsigned>	usedPsets() const; // union of phasePsets
    bool			phaseThreeDisjunct() const; // if phase 3 does not overlap with phase 1 or 2 in psets or cores
    std::vector<unsigned>	tabList() const;
    bool			conflictingResources(const Parset &otherParset, std::stringstream &error) const;
    int				phaseOnePsetIndex(unsigned pset) const;
    int				phaseTwoPsetIndex(unsigned pset) const;
    int				phaseThreePsetIndex(unsigned pset) const;
    std::string			getTransportType(const std::string &prefix) const;

    bool			outputFilteredData() const;
    bool			outputCorrelatedData() const;
    bool			outputBeamFormedData() const;
    bool			outputCoherentStokes() const;
    bool			outputIncoherentStokes() const;
    bool			outputTrigger() const;
    bool			outputThisType(OutputType) const;

    unsigned			nrStreams(OutputType, bool force=false) const;
    unsigned			maxNrStreamsPerPset(OutputType, bool force=false) const;
    static std::string		keyPrefix(OutputType);
    std::string			getHostName(OutputType, unsigned streamNr) const;
    std::string			getFileName(OutputType, unsigned streamNr) const;
    std::string			getDirectoryName(OutputType, unsigned streamNr) const;

    bool			fakeInputData() const;
    bool			checkFakeInputData() const;

    unsigned			nrCoherentStokes() const;
    unsigned			nrIncoherentStokes() const;
    bool			flysEye() const;
    std::string			bandFilter() const;
    std::string			antennaSet() const;

    unsigned			nrPencilBeams() const;
    BeamCoordinates		pencilBeams() const;

    std::vector<unsigned>	subbandList() const;
    unsigned			nrSubbands() const;
    unsigned			nrBeams() const;
    unsigned			nyquistZone() const;

    std::vector<unsigned>	subbandToSAPmapping() const;
    std::vector<double>		subbandToFrequencyMapping() const;
    std::vector<unsigned>	subbandToRSPboardMapping(const std::string &stationName) const;
    std::vector<unsigned>	subbandToRSPslotMapping(const std::string &stationName) const;

    unsigned			nrSlotsInFrame() const;
    std::string			partitionName() const;
    bool			realTime() const;
    
    bool			dumpRawData() const;
    
    std::vector<double>		getBeamDirection(unsigned beam) const;
    std::string			getBeamDirectionType(unsigned beam) const;
    
    struct StationRSPpair {
      std::string station;
      unsigned    rsp;
    };
    
    std::vector<StationRSPpair>	getStationNamesAndRSPboardNumbers(unsigned psetNumber) const;

    std::string			getInputStreamName(const string &stationName, unsigned rspBoardNumber) const;

    std::string			observerName() const;
    std::string			projectName() const;
    std::string			contactName() const;

    std::vector<double>		itsStPositions;

    bool			PLC_controlled() const;
    std::string			PLC_ProcID() const;
    std::string			PLC_Host() const;
    uint32			PLC_Port() const;
    std::string                 PVSS_TempObsName() const;

private:
    const std::string		itsName;

    void			checkVectorLength(const std::string &key, unsigned expectedSize) const;
    void			checkInputConsistency() const;

    std::vector<double>         getPencilBeam(unsigned beam, unsigned pencil) const;

    void			addPosition(string stName);
    double			getTime(const char *name) const;
    static int			findIndex(unsigned pset, const vector<unsigned> &psets);
    
    std::vector<double>		centroidPos(const string &stations) const;

    bool			compatibleInputSection(const Parset &otherParset, std::stringstream &error) const;
    bool			disjointCores(const Parset &, std::stringstream &error) const;
};


inline std::string Parset::name() const
{
  return itsName;
}


inline unsigned Parset::observationID() const
{
  return getUint32("Observation.ObsID");
}

inline double Parset::getTime(const char *name) const
{
  return to_time_t(time_from_string(getString(name)));
}

inline double Parset::startTime() const
{
  return getTime("Observation.startTime");
}

inline double Parset::stopTime() const
{
  return getTime("Observation.stopTime");
}

inline string Parset::stationName(int index) const
{
  return getStringVector("OLAP.storageStationNames",true)[index];
}

inline vector<string> Parset::allStationNames() const
{
  return getStringVector("Observation.VirtualInstrument.stationList");
}

inline bool Parset::hasStorage() const
{
  return getString("OLAP.OLAP_Conn.IONProc_Storage_Transport") != "NULL";
}

inline string Parset::getTransportType(const string& prefix) const
{
  return getString(prefix + "_Transport");
}

inline unsigned Parset::nrStations() const
{
  return getStringVector("OLAP.storageStationNames",true).size();
} 

inline unsigned Parset::nrTabStations() const
{
  return getStringVector("OLAP.tiedArrayStationNames",true).size();
}

inline unsigned Parset::nrMergedStations() const
{
  if (tabList().empty()) {
    return nrStations();
  }

  return *std::max_element( tabList().begin(), tabList().end() ) + 1;
}   

inline unsigned Parset::nrBaselines() const
{
  unsigned stations;
  
  if (nrTabStations() > 0)
    stations = nrTabStations();
  else
    stations = nrStations();

  return stations * (stations + 1) / 2;
} 

inline unsigned Parset::nrCrossPolarisations() const
{
  return (getUint32("Observation.nrPolarisations") * getUint32("Observation.nrPolarisations"));
}

inline unsigned Parset::clockSpeed() const
{
  return getUint32("Observation.sampleClock") * 1000000;
} 

inline double Parset::sampleRate() const
{
  return 1.0 * clockSpeed() / 1024;
} 

inline double Parset::sampleDuration() const
{
  return 1.0 / sampleRate();
} 

inline double Parset::dispersionMeasure() const
{
  return getDouble("OLAP.dispersionMeasure");
}

inline unsigned Parset::dedispersionFFTsize() const
{
  return isDefined("OLAP.CNProc.dedispersionFFTsize") ? getUint32("OLAP.CNProc.dedispersionFFTsize") : CNintegrationSteps();
}

inline unsigned Parset::nrBitsPerSample() const
{
  return getUint32("OLAP.nrBitsPerSample");
}

inline unsigned Parset::CNintegrationSteps() const
{
  return getUint32("OLAP.CNProc.integrationSteps");
}

inline unsigned Parset::IONintegrationSteps() const
{
  return getUint32("OLAP.IONProc.integrationSteps");
}

inline unsigned Parset::coherentStokesTimeIntegrationFactor() const
{
  return getUint32("OLAP.CNProc_CoherentStokes.timeIntegrationFactor");
}

inline unsigned Parset::incoherentStokesTimeIntegrationFactor() const
{
  return getUint32("OLAP.CNProc_IncoherentStokes.timeIntegrationFactor");
}

inline unsigned Parset::coherentStokesChannelsPerSubband() const
{
  return getUint32("OLAP.CNProc_CoherentStokes.channelsPerSubband");
}

inline unsigned Parset::incoherentStokesChannelsPerSubband() const
{
  return getUint32("OLAP.CNProc_IncoherentStokes.channelsPerSubband");
}

inline bool Parset::outputFilteredData() const
{
  return getBool("Observation.DataProducts.Output_FilteredData.enabled", false);
}

inline bool Parset::outputCorrelatedData() const
{
  return getBool("Observation.DataProducts.Output_Correlated.enabled", false);
}

inline bool Parset::outputBeamFormedData() const
{
  return getBool("Observation.DataProducts.Output_Beamformed.enabled", false);
}

inline bool Parset::outputCoherentStokes() const
{
  return getBool("Observation.DataProducts.Output_CoherentStokes.enabled", false);
}

inline bool Parset::outputIncoherentStokes() const
{
  return getBool("Observation.DataProducts.Output_IncoherentStokes.enabled", false);
}

inline bool Parset::outputTrigger() const
{
  return getBool("Observation.DataProducts.Output_Trigger.enabled", false);
}

inline bool Parset::outputThisType(OutputType outputType) const
{
  return getBool(keyPrefix(outputType) + ".enabled", false);
}

inline bool Parset::fakeInputData() const
{
  return getBool("OLAP.CNProc.fakeInputData", false);
}

inline bool Parset::checkFakeInputData() const
{
  return getBool("OLAP.CNProc.checkFakeInputData", false);
}

inline double Parset::CNintegrationTime() const
{
  return nrSubbandSamples() / sampleRate();
}

inline double Parset::IONintegrationTime() const
{
  return CNintegrationTime() * IONintegrationSteps();
}

inline unsigned Parset::nrSubbandSamples() const
{
  return CNintegrationSteps() * nrChannelsPerSubband();
}

inline unsigned Parset::nrHistorySamples() const
{
  return (nrPPFTaps() - 1) * nrChannelsPerSubband();
}

inline unsigned Parset::nrSamplesToCNProc() const
{
  return nrSubbandSamples() + nrHistorySamples() + 32 / (NR_POLARIZATIONS * 2 * nrBitsPerSample() / 8);
}

inline unsigned Parset::inputBufferSize() const
{
  return (unsigned) (getDouble("OLAP.nrSecondsOfBuffer") * sampleRate());
}

inline unsigned Parset::maxNetworkDelay() const
{
  return (unsigned) (getDouble("OLAP.maxNetworkDelay") * sampleRate());
}

inline unsigned Parset::nrSubbandsPerPset() const
{
  unsigned psets    = phaseTwoPsets().size();
  unsigned subbands = nrSubbands();

  return (psets == 0 ? 0 : subbands + psets - 1) / psets;
}

inline unsigned Parset::nrSubbandsPerPart() const
{
  return getUint32("OLAP.Storage.subbandsPerPart");
}

inline unsigned Parset::nrPartsPerStokes() const
{
  return getUint32("OLAP.Storage.partsPerStokes");
}

inline unsigned Parset::nrBeamsPerPset() const
{
  return maxNrStreamsPerPset(BEAM_FORMED_DATA) + maxNrStreamsPerPset(COHERENT_STOKES) + maxNrStreamsPerPset(TRIGGER_DATA);
}

inline unsigned Parset::nrPPFTaps() const
{
  return getUint32("OLAP.CNProc.nrPPFTaps");
}

inline unsigned Parset::nrChannelsPerSubband() const
{
  return getUint32("Observation.channelsPerSubband");
}

inline std::vector<unsigned> Parset::phaseOneTwoCores() const
{
  return getUint32Vector("OLAP.CNProc.phaseOneTwoCores", true);
}

inline std::vector<unsigned> Parset::phaseThreeCores() const
{
  return getUint32Vector("OLAP.CNProc.phaseThreeCores",true);
}  
 
inline unsigned Parset::nrCoresPerPset() const
{
  return usedCoresInPset().size();
}  

inline vector<unsigned> Parset::subbandList() const
{
  return getUint32Vector("Observation.subbandList",true);
} 

inline unsigned Parset::nrSubbands() const
{
  return getUint32Vector("Observation.subbandList",true).size();
} 

inline vector<unsigned> Parset::subbandToSAPmapping() const
{
  return getUint32Vector("Observation.beamList",true);
}

inline double Parset::channelWidth() const
{
  return sampleRate() / nrChannelsPerSubband();
}

inline bool Parset::delayCompensation() const
{
  return getBool("OLAP.delayCompensation");
}

inline unsigned Parset::nrCalcDelays() const
{
  return getUint32("OLAP.DelayComp.nrCalcDelays");
}

inline string Parset::positionType() const
{
  return getString("OLAP.DelayComp.positionType");
}

inline double Parset::clockCorrectionTime(const std::string &station) const
{
  return getDouble(std::string("PIC.Core.") + station + ".clockCorrectionTime",0.0);
}

inline bool Parset::correctBandPass() const
{
  return getBool("OLAP.correctBandPass");
}

inline unsigned Parset::nrPsetsPerStorage() const
{
  return getUint32("OLAP.psetsPerStorage");
}

inline unsigned Parset::getLofarStManVersion() const
{
  return getUint32("OLAP.LofarStManVersion", 1); 
}

inline vector<unsigned> Parset::phaseOnePsets() const
{
  return getUint32Vector("OLAP.CNProc.phaseOnePsets",true);
}

inline vector<unsigned> Parset::phaseTwoPsets() const
{
  return getUint32Vector("OLAP.CNProc.phaseTwoPsets",true);
}

inline vector<unsigned> Parset::phaseThreePsets() const
{
  return getUint32Vector("OLAP.CNProc.phaseThreePsets",true);
}

inline vector<unsigned> Parset::tabList() const
{
  return getUint32Vector("OLAP.CNProc.tabList",true);
}

inline int Parset::phaseOnePsetIndex(unsigned pset) const
{
  return findIndex(pset, phaseOnePsets());
}

inline int Parset::phaseTwoPsetIndex(unsigned pset) const
{
  return findIndex(pset, phaseTwoPsets());
}

inline int Parset::phaseThreePsetIndex(unsigned pset) const
{
  return findIndex(pset, phaseThreePsets());
}

inline unsigned Parset::nrSlotsInFrame() const
{
  return getUint32("Observation.nrSlotsInFrame");
}

inline string Parset::partitionName() const
{
  return getString("OLAP.CNProc.partition");
}

inline bool Parset::dumpRawData() const
{
  return getBool("OLAP.OLAP_Conn.rawDataOutputOnly");
}

inline bool Parset::realTime() const
{
  return getBool("OLAP.realTime");
}

inline unsigned Parset::nrPencilBeams() const
{
  return getUint32("Observation.Beam[0].nrTiedArrayBeams");
}

inline BeamCoordinates Parset::pencilBeams() const
{
  BeamCoordinates coordinates;

  for (unsigned i = 0; i < nrPencilBeams(); i ++) {
    const std::vector<double> coords = getPencilBeam(0,i);

    // assume ra,dec
    coordinates += BeamCoord3D(coords[0],coords[1]);
  }

  return coordinates;
}

inline bool Parset::flysEye() const
{
  return getBool("OLAP.PencilInfo.flysEye", false);
}

inline string Parset::observerName() const
{
  // TODO - this should be included in the Parset from SAS
  return getString("Observation.ObserverName");
}

inline string Parset::projectName() const
{
  // TODO - this should be included in the Parset from SAS
  return getString("Observation.ProjectName");
}

inline string Parset::contactName() const
{
  // TODO - this should be included in the Parset from SAS
  return getString("Observation.ContactName");
}

inline string Parset::bandFilter() const
{
  return getString("Observation.bandFilter");
}

inline string Parset::antennaSet() const
{
  return getString("Observation.antennaSet");
}

inline bool Parset::PLC_controlled() const
{
  return getBool("OLAP.IONProc.PLC_controlled",false);
}

inline string Parset::PLC_ProcID() const
{
  return getString("_processName","CNProc");
}

inline string Parset::PLC_Host() const
{
  using boost::format;

  string prefix = getString("_parsetPrefix"); // includes a trailing dot!

  return getString(str(format("%s_ACnode") % prefix));
}

inline uint32 Parset::PLC_Port() const
{
  using boost::format;

  string prefix = getString("_parsetPrefix"); // includes a trailing dot!

  return getUint32(str(format("%s_ACport") % prefix));
}

inline string Parset::PVSS_TempObsName() const
{
  return getString("_DPname","");
}

} // namespace RTCP
} // namespace LOFAR

#endif
