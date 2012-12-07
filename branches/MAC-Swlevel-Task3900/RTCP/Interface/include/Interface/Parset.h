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
#include <Common/LofarBitModeInfo.h>
#include <Common/StreamUtil.h>
#include <Common/StringUtil.h>
#include <Common/lofar_datetime.h>
#include <Common/LofarLogger.h> 
#include <Interface/BeamCoordinates.h>
#include <Interface/Config.h>
#include <Interface/OutputTypes.h>
#include <Interface/SmartPtr.h>
#include <Stream/Stream.h>
#include <Interface/PrintVector.h>

#include <algorithm>
#include <numeric>
#include <sstream>
#include <vector>
#include <string>

#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/format.hpp>

namespace LOFAR {
namespace RTCP {

class Transpose2;
class CN_Transpose2;

enum StokesType { STOKES_I = 0, STOKES_IQUV, STOKES_XXYY, INVALID_STOKES = -1 };

static StokesType stokesType( const std::string &name ) {
  if (name == "I")
    return STOKES_I;

  if (name == "IQUV")
    return STOKES_IQUV;

  if (name == "XXYY")
    return STOKES_XXYY;

  return INVALID_STOKES;
};
    

// The Parset class is a public struct that can be used as base-class
// for holding Parset related information.
// It can be instantiated with a parset containing Parset information.
class Parset: public ParameterSet
{
  public:
    Parset();
    Parset(const std::string &name);
    Parset(Stream *);

     
    std::string			name() const;
    void			check() const;

    void			write(Stream *) const;

    unsigned			observationID() const;
    double			startTime() const;
    double			stopTime() const;

    unsigned    nrCorrelatedBlocks() const;
    unsigned    nrBeamFormedBlocks() const;

    unsigned			nrStations() const;
    unsigned			nrTabStations() const;
    unsigned			nrMergedStations() const;
    std::vector<std::string>	mergedStationNames() const;
    unsigned			nrBaselines() const;
    unsigned			nrCrossPolarisations() const;
    unsigned			clockSpeed() const; // Hz
    double			subbandBandwidth() const;
    double			sampleDuration() const;
    unsigned			nrBitsPerSample() const;
    size_t			nrBytesPerComplexSample() const;
    std::vector<double>		positions() const;
    std::string			positionType() const;
    std::vector<double>		getRefPhaseCentre() const;
    std::vector<double>		getPhaseCentreOf(const std::string &name) const;
    unsigned			dedispersionFFTsize() const;
    unsigned			CNintegrationSteps() const;
    unsigned			IONintegrationSteps() const;
    unsigned			integrationSteps() const;
    unsigned			coherentStokesTimeIntegrationFactor() const;
    unsigned			coherentStokesNrSubbandsPerFile() const; 
    unsigned			incoherentStokesTimeIntegrationFactor() const;
    unsigned			coherentStokesChannelsPerSubband() const;
    unsigned			incoherentStokesChannelsPerSubband() const;
    unsigned			incoherentStokesNrSubbandsPerFile() const; 
    double			CNintegrationTime() const;
    double			IONintegrationTime() const;
    unsigned			nrSamplesPerChannel() const;
    unsigned			nrSamplesPerSubband() const;
    unsigned			nrSubbandsPerPset() const; 
    unsigned			nrPhase3StreamsPerPset() const; 
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
    int			        stationIndex(const std::string &name) const;
    std::vector<std::string>	allStationNames() const;
    unsigned			nrPsetsPerStorage() const;
    unsigned			getLofarStManVersion() const;
    std::vector<unsigned>	phaseOnePsets() const;
    std::vector<unsigned>	phaseTwoPsets() const;
    std::vector<unsigned>	phaseThreePsets() const;
    std::vector<unsigned>	usedPsets() const; // union of phasePsets
    unsigned	                totalNrPsets() const; // nr psets in the partition
    bool			phaseThreeDisjunct() const; // if phase 3 does not overlap with phase 1 or 2 in psets or cores
    std::vector<unsigned>	tabList() const;
    bool			conflictingResources(const Parset &otherParset, std::stringstream &error) const;

    int				phaseOnePsetIndex(unsigned pset) const;
    int				phaseTwoPsetIndex(unsigned pset) const;
    int				phaseThreePsetIndex(unsigned pset) const;
    int				phaseOneCoreIndex(unsigned core) const;
    int				phaseTwoCoreIndex(unsigned core) const;
    int				phaseThreeCoreIndex(unsigned core) const;

    std::string			getTransportType(const std::string &prefix) const;

    bool			outputFilteredData() const;
    bool			outputCorrelatedData() const;
    bool			outputBeamFormedData() const;
    bool			outputTrigger() const;
    bool			outputThisType(OutputType) const;

    bool                        onlineFlagging() const;
    bool                        onlinePreCorrelationFlagging() const;
    bool                        onlinePreCorrelationNoChannelsFlagging() const;
    bool                        onlinePostCorrelationFlagging() const;
    bool                        onlinePostCorrelationFlaggingDetectBrokenStations() const;
    unsigned                    onlinePreCorrelationFlaggingIntegration() const;
    std::string                 onlinePreCorrelationFlaggingType(std::string defaultVal) const;
    std::string                 onlinePreCorrelationFlaggingStatisticsType(std::string defaultVal) const;
    std::string                 onlinePostCorrelationFlaggingType(std::string defaultVal) const;
    std::string                 onlinePostCorrelationFlaggingStatisticsType(std::string defaultVal) const;

    unsigned			nrStreams(OutputType, bool force=false) const;
    unsigned			maxNrStreamsPerPset(OutputType, bool force=false) const;
    static std::string		keyPrefix(OutputType);
    std::string			getHostName(OutputType, unsigned streamNr) const;
    std::string			getFileName(OutputType, unsigned streamNr) const;
    std::string			getDirectoryName(OutputType, unsigned streamNr) const;

    bool			fakeInputData() const;
    bool			checkFakeInputData() const;

    std::string			coherentStokes() const;
    std::string			incoherentStokes() const;
    std::string			bandFilter() const;
    std::string			antennaSet() const;

    unsigned			nrBeams() const;
    std::string                 beamTarget(unsigned beam) const;
    double                      beamDuration(unsigned beam) const;

    unsigned			nrTABs(unsigned beam) const;
    std::vector<unsigned>	nrTABs() const;
    unsigned			totalNrTABs() const;
    unsigned			maxNrTABs() const;
    bool                        isCoherent(unsigned beam, unsigned pencil) const;
    BeamCoordinates		TABs(unsigned beam) const;
    double			dispersionMeasure(unsigned beam=0,unsigned pencil=0) const;
    std::vector<std::string>	TABStationList(unsigned beam=0,unsigned pencil=0) const;

    std::vector<unsigned>	subbandList() const;
    unsigned			nrSubbands() const;
    unsigned			nrSubbandsPerSAP(unsigned sap) const;
    unsigned			nyquistZone() const;

    std::vector<unsigned>	subbandToSAPmapping() const;
    std::vector<double>		subbandToFrequencyMapping() const;
    std::vector<unsigned>	subbandToRSPboardMapping(const std::string &stationName) const;
    std::vector<unsigned>	subbandToRSPslotMapping(const std::string &stationName) const;

    double channel0Frequency( size_t subband ) const;

    unsigned			nrSlotsInFrame() const;
    std::string			partitionName() const;
    bool			realTime() const;
    
    std::vector<double>		getBeamDirection(unsigned beam) const;
    std::string			getBeamDirectionType(unsigned beam) const;

    bool                        haveAnaBeam() const;
    std::vector<double>         getAnaBeamDirection() const;
    std::string                 getAnaBeamDirectionType() const;
          
    struct StationRSPpair {
      std::string station;
      unsigned    rsp;
    };
    
    std::vector<StationRSPpair>	getStationNamesAndRSPboardNumbers(unsigned psetNumber) const;

    std::string			getInputStreamName(const string &stationName, unsigned rspBoardNumber) const;

    std::vector<double>		itsStPositions;

    bool			PLC_controlled() const;
    std::string			PLC_ProcID() const;
    std::string			PLC_Host() const;
    uint32			PLC_Port() const;
    std::string                 PVSS_TempObsName() const;

    std::string                 AntennaSetsConf() const;
    std::string                 AntennaFieldsDir() const;
    std::string                 HBADeltasDir() const;

    const Transpose2            &transposeLogic() const;
    const CN_Transpose2         &CN_transposeLogic( unsigned pset, unsigned core ) const;

private:
    const std::string		itsName;

    mutable std::string		itsWriteCache;

    mutable SmartPtr<const Transpose2>     itsTransposeLogic;
    mutable SmartPtr<const CN_Transpose2>  itsCN_TransposeLogic;

    void			checkVectorLength(const std::string &key, unsigned expectedSize) const;
    void			checkInputConsistency() const;

    std::vector<double>         getTAB(unsigned beam, unsigned pencil) const;

    void			addPosition(string stName);
    double			getTime(const char *name) const;
    static int			findIndex(unsigned pset, const vector<unsigned> &psets);
    
    std::vector<double>		centroidPos(const string &stations) const;

    bool			compatibleInputSection(const Parset &otherParset, std::stringstream &error) const;
    bool			disjointCores(const Parset &, std::stringstream &error) const;
};

//
// All of the logic for the second transpose.
//

struct StreamInfo {
  unsigned stream;

  unsigned sap;
  unsigned beam;

  bool coherent;
  unsigned nrChannels;     // channels per subband
  unsigned timeIntFactor;  // time integration factor
  unsigned nrStokes;       // total # stokes for this beam
  StokesType stokesType;
  unsigned nrSamples;      // # samples/channel, after temporal integration

  unsigned stokes;
  unsigned part;

  std::vector<unsigned> subbands;

  void log() const {
    LOG_DEBUG_STR( "Stream " << stream << " is sap " << sap << " beam " << beam << " stokes " << stokes << " part " << part << " consisting of subbands " << subbands );
  }
};

class Transpose2 {
public:
  Transpose2( const Parset &parset )
  :
    phaseThreeDisjunct( parset.phaseThreeDisjunct() ),

    nrChannels( parset.nrChannelsPerSubband() ),
    nrCoherentChannels( parset.coherentStokesChannelsPerSubband() ),
    nrIncoherentChannels( parset.incoherentStokesChannelsPerSubband() ),
    nrSamples( parset.CNintegrationSteps() ),
    coherentTimeIntFactor( parset.coherentStokesTimeIntegrationFactor() ),
    incoherentTimeIntFactor( parset.incoherentStokesTimeIntegrationFactor() ),

    nrBeams( parset.totalNrTABs() ),
    coherentNrSubbandsPerFile( parset.coherentStokesNrSubbandsPerFile() ),
    incoherentNrSubbandsPerFile( parset.incoherentStokesNrSubbandsPerFile() ),

    nrPhaseTwoPsets( parset.phaseTwoPsets().size() ),
    nrPhaseTwoCores( parset.phaseOneTwoCores().size() ),
    nrPhaseThreePsets( parset.phaseThreePsets().size() ),
    nrPhaseThreeCores( parset.phaseThreeCores().size() ),

    nrSubbandsPerPset( parset.nrSubbandsPerPset() ),
    streamInfo( generateStreamInfo(parset) ),
    nrStreamsPerPset( divideRoundUp( nrStreams(), parset.phaseThreePsets().size() ) )
  {
  }

  unsigned nrStreams() const { return streamInfo.size(); }

  // compose and decompose a stream number
  unsigned stream( unsigned sap, unsigned beam, unsigned stokes, unsigned part, unsigned startAt = 0) const {
    for (unsigned i = startAt; i < streamInfo.size(); i++) {
      const struct StreamInfo &info = streamInfo[i];

      if (sap == info.sap && beam == info.beam && stokes == info.stokes && part == info.part)
        return i;
    }

    // shouldn't reach this point
    ASSERTSTR(false, "Requested non-existing sap " << sap << " beam " << beam << " stokes " << stokes << " part " << part);

    return 0;
  }

  void decompose( unsigned stream, unsigned &sap, unsigned &beam, unsigned &stokes, unsigned &part ) const {
    const struct StreamInfo &info = streamInfo[stream];

    sap    = info.sap;
    beam   = info.beam;
    stokes = info.stokes;
    part   = info.part;
  }

  std::vector<unsigned> subbands( unsigned stream ) const { ASSERT(stream < streamInfo.size()); return streamInfo[stream].subbands; }
  unsigned nrSubbands( unsigned stream ) const { return stream >= streamInfo.size() ? 0 : subbands(stream).size(); }
  unsigned maxNrSubbands() const { return streamInfo.size() == 0 ? 0 : std::max_element( streamInfo.begin(), streamInfo.end(), &streamInfoSubbandsComp )->subbands.size(); }
  unsigned maxNrChannels() const { return streamInfo.size() == 0 ? 0 : std::max_element( streamInfo.begin(), streamInfo.end(), &streamInfoChannelsComp )->nrChannels; }
  unsigned maxNrSamples() const { return streamInfo.size() == 0 ? 0 : std::max_element( streamInfo.begin(), streamInfo.end(), &streamInfoSamplesComp )->nrSamples; }

  size_t subbandSize( unsigned stream ) const { const StreamInfo &info = streamInfo[stream]; return (size_t)info.nrChannels * (info.nrSamples|2) * sizeof(float); }

  //unsigned maxNrSubbandsPerStream() const { return std::min(nrSubbands, nrSubbandsPerFile); }

  // the pset/core which processes a certain block of a certain subband
  // note: AsyncTransposeBeams applied the mapping of phaseThreePsets
  unsigned sourceCore( unsigned subband, unsigned block ) const { return (block * nrSubbandsPerPset + subband % nrSubbandsPerPset) % nrPhaseTwoCores; }
  unsigned sourcePset( unsigned subband, unsigned block ) const { (void)block; return subband / nrSubbandsPerPset; }

  // the pset/core which processes a certain block of a certain stream
  // note: AsyncTransposeBeams applied the mapping of phaseTwoPsets
  unsigned destCore( unsigned stream, unsigned block ) const { return (block * phaseThreeGroupSize() + stream % nrStreamsPerPset) % nrPhaseThreeCores; }
  unsigned destPset( unsigned stream, unsigned block ) const { (void)block; return stream / nrStreamsPerPset; }

  // if phase2 == phase3, each block in phase3 is processed by more cores (more cores idle to align phases 2 and 3)
  unsigned phaseThreeGroupSize() const {
    return phaseThreeDisjunct ? nrStreamsPerPset : nrSubbandsPerPset;
  }

  const bool phaseThreeDisjunct;

  const unsigned nrChannels;
  const unsigned nrCoherentChannels;
  const unsigned nrIncoherentChannels;
  const unsigned nrSamples;
  const unsigned coherentTimeIntFactor;
  const unsigned incoherentTimeIntFactor;

  const unsigned nrBeams;
  const unsigned coherentNrSubbandsPerFile;
  const unsigned incoherentNrSubbandsPerFile;

  const unsigned nrPhaseTwoPsets;
  const unsigned nrPhaseTwoCores;
  const unsigned nrPhaseThreePsets;
  const unsigned nrPhaseThreeCores;

  const unsigned nrSubbandsPerPset;

  const std::vector<struct StreamInfo> streamInfo;

  const unsigned nrStreamsPerPset;
private:

  static bool streamInfoSubbandsComp( const struct StreamInfo &a, const struct StreamInfo &b ) {
    return a.subbands.size() < b.subbands.size();
  }

  static bool streamInfoChannelsComp( const struct StreamInfo &a, const struct StreamInfo &b ) {
    return a.nrChannels < b.nrChannels;
  }

  static bool streamInfoSamplesComp( const struct StreamInfo &a, const struct StreamInfo &b ) {
    return a.nrSamples < b.nrSamples;
  }

  static unsigned divideRoundUp( unsigned a, unsigned b ) {
    return b == 0 ? 0 : (a + b - 1) / b;
  }

  std::vector<struct StreamInfo> generateStreamInfo( const Parset &parset ) const {
    // get all info from parset, since we will be called while constructing our members

    // ParameterSets are SLOW, so cache any info we need repeatedly

    std::vector<struct StreamInfo> infoset;
    const std::vector<unsigned> sapMapping = parset.subbandToSAPmapping();
    const unsigned nrSAPs             = parset.nrBeams();
    const unsigned nrSubbands         = parset.nrSubbands();
    const unsigned nrCoherentSubbandsPerFile    = parset.coherentStokesNrSubbandsPerFile();
    const unsigned nrIncoherentSubbandsPerFile  = parset.incoherentStokesNrSubbandsPerFile();

    const unsigned nrCoherentStokes          = parset.coherentStokes().size();
    const StokesType coherentType            = stokesType( parset.coherentStokes() );
    const unsigned nrCoherentChannels        = parset.coherentStokesChannelsPerSubband();
    const unsigned nrCoherentTimeIntFactor   = parset.coherentStokesTimeIntegrationFactor();

    const unsigned nrIncoherentStokes        = parset.incoherentStokes().size();
    const StokesType incoherentType          = stokesType( parset.incoherentStokes() );
    const unsigned nrIncoherentChannels      = parset.incoherentStokesChannelsPerSubband();
    const unsigned nrIncoherentTimeIntFactor = parset.incoherentStokesTimeIntegrationFactor();

    const unsigned nrSamples                 = parset.CNintegrationSteps();

    struct StreamInfo info;
    info.stream = 0;

    for (unsigned sap = 0; sap < nrSAPs; sap++) {
      const unsigned nrBeams = parset.nrTABs(sap);

      info.sap = sap;

      std::vector<unsigned> sapSubbands;

      for (unsigned sb = 0; sb < nrSubbands; sb++)
        if (sapMapping[sb] == sap)
          sapSubbands.push_back(sb);

      for (unsigned beam = 0; beam < nrBeams; beam++) {
        info.beam = beam;

        const bool coherent = parset.isCoherent(sap, beam);
        const unsigned nrStokes = coherent ? nrCoherentStokes : nrIncoherentStokes;

        info.coherent       = coherent;
        info.nrChannels     = coherent ? nrCoherentChannels : nrIncoherentChannels;
        info.timeIntFactor  = coherent ? nrCoherentTimeIntFactor : nrIncoherentTimeIntFactor;
        info.nrStokes       = nrStokes;
        info.stokesType     = coherent ? coherentType : incoherentType;
        info.nrSamples      = nrSamples / info.timeIntFactor;

        const unsigned nrSubbandsPerFile = coherent ? nrCoherentSubbandsPerFile : nrIncoherentSubbandsPerFile;

        for (unsigned stokes = 0; stokes < nrStokes; stokes++) {
          info.stokes = stokes;
          info.part   = 0;

          // split into parts of at most nrSubbandsPerFile
          for (unsigned sb = 0; sb < sapSubbands.size(); sb += nrSubbandsPerFile ) {
            for (unsigned i = 0; sb + i < sapSubbands.size() && i < nrSubbandsPerFile; i++)
              info.subbands.push_back(sapSubbands[sb + i]);

            infoset.push_back(info);

            info.subbands.clear();
            info.part++;
            info.stream++;
          }
        }  
      }
    }

    return infoset;
  }
};

class CN_Transpose2: public Transpose2 {
public:
  CN_Transpose2( const Parset &parset, unsigned myPset, unsigned myCore )
  :
    Transpose2( parset ),
    myPset( myPset ),
    myCore( myCore ),

    phaseTwoPsetIndex( parset.phaseTwoPsetIndex(myPset) ),
    phaseTwoCoreIndex( parset.phaseTwoCoreIndex(myCore) ),
    phaseThreePsetIndex( parset.phaseThreePsetIndex(myPset) ),
    phaseThreeCoreIndex( parset.phaseThreeCoreIndex(myCore) )
  {
  }

  // the stream to process on (myPset, myCore)
  int myStream( unsigned block ) const { 
    unsigned first = phaseThreePsetIndex * nrStreamsPerPset;
    unsigned blockShift = (phaseThreeGroupSize() * block) % nrPhaseThreeCores;
    unsigned relative = (nrPhaseThreeCores + phaseThreeCoreIndex - blockShift) % nrPhaseThreeCores;

    // such a stream does not exist
    if (first + relative >= nrStreams())
      return -1;

    // we could handle this stream, but it's handled by a subsequent pset
    if (relative >= nrStreamsPerPset)
      return -1;

    return first + relative;
  }

  // the part number of a subband with an absolute index
  unsigned myPart( unsigned subband, bool coherent ) const {
    for (unsigned i = 0; i < streamInfo.size(); i++) {
      const struct StreamInfo &info = streamInfo[i];

      if ( info.coherent == coherent
        && info.subbands[0] <= subband
        && info.subbands[info.subbands.size()-1] >= subband )
        return info.part;
    }

    // we reach this point if there are no beams of this coherency
    return 0;
  }

  const unsigned myPset;
  const unsigned myCore;

  const int phaseTwoPsetIndex;
  const int phaseTwoCoreIndex;
  const int phaseThreePsetIndex;
  const int phaseThreeCoreIndex;
};

inline std::string Parset::name() const
{
  return itsName;
}

inline const Transpose2 &Parset::transposeLogic() const
{
  if (!itsTransposeLogic)
    itsTransposeLogic = new Transpose2(*this);

  return *itsTransposeLogic;  
}

inline const CN_Transpose2 &Parset::CN_transposeLogic( unsigned pset, unsigned core ) const
{
  if (!itsCN_TransposeLogic)
    itsCN_TransposeLogic = new CN_Transpose2(*this, pset, core);

  return *itsCN_TransposeLogic;  
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

inline unsigned Parset::nrCorrelatedBlocks() const
{
  return static_cast<unsigned>(floor( (stopTime() - startTime()) / IONintegrationTime()));
}

inline unsigned Parset::nrBeamFormedBlocks() const
{
  return static_cast<unsigned>(floor( (stopTime() - startTime()) / CNintegrationTime()));
}

inline string Parset::stationName(int index) const
{
  return allStationNames()[index];
}

inline int Parset::stationIndex(const std::string &name) const
{
  std::vector<std::string> names = allStationNames();
  for (unsigned i = 0; i < names.size(); i++)
    if (names[i] == name)
      return i;

  return -1;    
}

inline std::vector<std::string> Parset::allStationNames() const
{
  return getStringVector("OLAP.storageStationNames",true);
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
  return allStationNames().size();
} 

inline unsigned Parset::nrTabStations() const
{
  return getStringVector("OLAP.tiedArrayStationNames",true).size();
}

inline std::vector<std::string> Parset::mergedStationNames() const
{
  std::vector<string> tabStations = getStringVector("OLAP.tiedArrayStationNames",true);

  if (tabStations.empty())
    return getStringVector("OLAP.storageStationNames",true);
  else
    return tabStations;
}

inline unsigned Parset::nrMergedStations() const
{
  const std::vector<unsigned> list = tabList();

  if (list.empty())
    return nrStations();

  return *std::max_element( list.begin(), list.end() ) + 1;
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

inline double Parset::subbandBandwidth() const
{
  return 1.0 * clockSpeed() / 1024;
} 

inline double Parset::sampleDuration() const
{
  return 1.0 / subbandBandwidth();
} 

inline unsigned Parset::dedispersionFFTsize() const
{
  return isDefined("OLAP.CNProc.dedispersionFFTsize") ? getUint32("OLAP.CNProc.dedispersionFFTsize") : CNintegrationSteps();
}

inline unsigned Parset::nrBitsPerSample() const
{
  const std::string key = "Observation.nrBitsPerSample";

  if (isDefined(key)) {
    return getUint32(key);
  } else {
#ifndef HAVE_BGP_CN
    LOG_WARN_STR( "Missing key " << key << ", using the depricated key OLAP.nrBitsPerSample");
#endif
    return getUint32("OLAP.nrBitsPerSample", 16);
  }  
}

inline size_t Parset::nrBytesPerComplexSample() const
{
  return 2 * nrBitsPerSample() / 8;
}

inline unsigned Parset::CNintegrationSteps() const
{
  return getUint32("OLAP.CNProc.integrationSteps");
}

inline unsigned Parset::IONintegrationSteps() const
{
  return getUint32("OLAP.IONProc.integrationSteps");
}

inline unsigned Parset::integrationSteps() const
{
  return CNintegrationSteps() * IONintegrationSteps();
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
 unsigned numch = getUint32("OLAP.CNProc_CoherentStokes.channelsPerSubband");

 if (numch == 0)
   return nrChannelsPerSubband();
 else
   return numch;
}

inline unsigned Parset::incoherentStokesChannelsPerSubband() const
{
 unsigned numch = getUint32("OLAP.CNProc_IncoherentStokes.channelsPerSubband");

 if (numch == 0)
   return nrChannelsPerSubband();
 else
   return numch;
}

inline std::string Parset::coherentStokes() const
{
  return getString("OLAP.CNProc_CoherentStokes.which");
}

inline std::string Parset::incoherentStokes() const
{
  return getString("OLAP.CNProc_IncoherentStokes.which");
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

inline bool Parset::outputTrigger() const
{
  return getBool("Observation.DataProducts.Output_Trigger.enabled", false);
}

inline bool Parset::outputThisType(OutputType outputType) const
{
  return getBool(keyPrefix(outputType) + ".enabled", false);
}

inline bool Parset::onlineFlagging() const
{
  return getBool("OLAP.CNProc.onlineFlagging", false);
}

inline bool Parset::onlinePreCorrelationFlagging() const
{
  return getBool("OLAP.CNProc.onlinePreCorrelationFlagging", false);
}

inline bool Parset::onlinePreCorrelationNoChannelsFlagging() const
{
  return getBool("OLAP.CNProc.onlinePreCorrelationNoChannelsFlagging", false);
}

inline bool Parset::onlinePostCorrelationFlagging() const
{
  return getBool("OLAP.CNProc.onlinePostCorrelationFlagging", false);
}

 inline unsigned Parset::onlinePreCorrelationFlaggingIntegration() const
{
  return getUint32("OLAP.CNProc.onlinePostCorrelationFlaggingIntegration", 0);
}


inline string Parset::onlinePreCorrelationFlaggingType(std::string defaultVal) const
{
  return getString("OLAP.CNProc.onlinePreCorrelationFlaggingType", defaultVal);
}

inline string Parset::onlinePreCorrelationFlaggingStatisticsType(std::string defaultVal) const
{
  return getString("OLAP.CNProc.onlinePreCorrelationFlaggingStatisticsType", defaultVal);
}

inline string Parset::onlinePostCorrelationFlaggingType(std::string defaultVal) const
{
  return getString("OLAP.CNProc.onlinePostCorrelationFlaggingType", defaultVal);
}

inline string Parset::onlinePostCorrelationFlaggingStatisticsType(std::string defaultVal) const
{
  return getString("OLAP.CNProc.onlinePostCorrelationFlaggingStatisticsType", defaultVal);
}

inline bool Parset::onlinePostCorrelationFlaggingDetectBrokenStations() const
{
  return getBool("OLAP.CNProc.onlinePostCorrelationFlaggingDetectBrokenStations", false);
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
  return nrSamplesPerSubband() / subbandBandwidth();
}

inline double Parset::IONintegrationTime() const
{
  return CNintegrationTime() * IONintegrationSteps();
}

inline unsigned Parset::nrSamplesPerChannel() const
{
  return CNintegrationSteps();
}

inline unsigned Parset::nrSamplesPerSubband() const
{
  return nrSamplesPerChannel() * nrChannelsPerSubband();
}

inline unsigned Parset::nrHistorySamples() const
{
  return nrChannelsPerSubband() > 1 ? (nrPPFTaps() - 1) * nrChannelsPerSubband() : 0;
}

inline unsigned Parset::nrSamplesToCNProc() const
{
  return nrSamplesPerSubband() + nrHistorySamples() + 32 / (NR_POLARIZATIONS * 2 * nrBitsPerSample() / 8);
}

inline unsigned Parset::inputBufferSize() const
{
  return (unsigned) (getDouble("OLAP.nrSecondsOfBuffer") * subbandBandwidth());
}

inline unsigned Parset::maxNetworkDelay() const
{
  return (unsigned) (getDouble("OLAP.maxNetworkDelay") * subbandBandwidth());
}

inline unsigned Parset::nrSubbandsPerPset() const
{
  unsigned psets    = phaseTwoPsets().size();
  unsigned subbands = nrSubbands();

  return (psets == 0 ? 0 : subbands + psets - 1) / psets;
}

inline unsigned Parset::coherentStokesNrSubbandsPerFile() const
{
  return std::min( getUint32("OLAP.CNProc_CoherentStokes.subbandsPerFile"), nrSubbands() );
}

inline unsigned Parset::incoherentStokesNrSubbandsPerFile() const
{
  return std::min( getUint32("OLAP.CNProc_IncoherentStokes.subbandsPerFile"), nrSubbands() );
}

inline unsigned Parset::nrPhase3StreamsPerPset() const
{
  return maxNrStreamsPerPset(BEAM_FORMED_DATA) + maxNrStreamsPerPset(TRIGGER_DATA);
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

inline unsigned Parset::nrSubbandsPerSAP(unsigned sap) const
{
  std::vector<unsigned> mapping = subbandToSAPmapping();

  return std::count(mapping.begin(), mapping.end(), sap);
} 

inline std::vector<unsigned> Parset::subbandToSAPmapping() const
{
  return getUint32Vector("Observation.beamList",true);
}

inline double Parset::channelWidth() const
{
  return subbandBandwidth() / nrChannelsPerSubband();
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
  return getUint32("OLAP.LofarStManVersion", 2); 
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

inline unsigned Parset::totalNrPsets() const
{
  const std::string key = "OLAP.IONProc.psetList";

  if (isDefined(key)) {
    return getStringVector(key,true).size();
  } else {
    LOG_WARN_STR( "Missing key " << key << ", using the used psets as a fallback");
    return usedPsets().size();
  }  
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

inline int Parset::phaseOneCoreIndex(unsigned core) const
{
  return findIndex(core, phaseOneTwoCores());
}

inline int Parset::phaseTwoCoreIndex(unsigned core) const
{
  return findIndex(core, phaseOneTwoCores());
}

inline int Parset::phaseThreeCoreIndex(unsigned core) const
{
  return findIndex(core, phaseThreeCores());
}

inline double Parset::channel0Frequency(size_t subband) const
{
  double sbFreq = subbandToFrequencyMapping()[subband];

  if (nrChannelsPerSubband() == 1)
    return sbFreq;

  // if the 2nd PPF is used, the subband is shifted half a channel
  // downwards, so subtracting half a subband results in the
  // center of channel 0 (instead of the bottom).
  return sbFreq - 0.5 * subbandBandwidth();
}

inline unsigned Parset::nrSlotsInFrame() const
{
  unsigned nrSlots = 0;

  nrSlots = getUint32("Observation.nrSlotsInFrame", 0);

  if (nrSlots == 0) {
    // return default
    return maxBeamletsPerRSP(nrBitsPerSample());
  }

  return nrSlots;
}

inline string Parset::partitionName() const
{
  return getString("OLAP.CNProc.partition");
}

inline bool Parset::realTime() const
{
  return getBool("OLAP.realTime");
}

inline unsigned Parset::nrTABs(unsigned beam) const
{
  using boost::format;

  return getUint32(str(format("Observation.Beam[%u].nrTiedArrayBeams") % beam));
}

inline std::vector<unsigned> Parset::nrTABs() const
{
  std::vector<unsigned> counts(nrBeams());

  for (unsigned beam = 0; beam < nrBeams(); beam++)
    counts[beam] = nrTABs(beam);

  return counts;
}

inline unsigned Parset::totalNrTABs() const
{
  std::vector<unsigned> beams = nrTABs();

  return std::accumulate(beams.begin(), beams.end(), 0);
}

inline unsigned Parset::maxNrTABs() const
{
  std::vector<unsigned> beams = nrTABs();

  return *std::max_element(beams.begin(), beams.end());
}

inline BeamCoordinates Parset::TABs(unsigned beam) const
{
  BeamCoordinates coordinates;

  for (unsigned pencil = 0; pencil < nrTABs(beam); pencil ++) {
    const std::vector<double> coords = getTAB(beam, pencil);

    // assume ra,dec
    coordinates += BeamCoord3D(coords[0],coords[1]);
  }

  return coordinates;
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

inline string Parset::AntennaSetsConf() const
{
  return getString("OLAP.Storage.AntennaSetsConf","");
}

inline string Parset::AntennaFieldsDir() const
{
  return getString("OLAP.Storage.AntennaFieldsDir","");
}

inline string Parset::HBADeltasDir() const
{
  return getString("OLAP.Storage.HBADeltasDir","");
}

} // namespace RTCP
} // namespace LOFAR

#endif
