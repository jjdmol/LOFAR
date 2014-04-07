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

#if ! defined HAVE_BGP_CN

//# Includes
#include <Common/ParameterSet.h>
#include <Common/StreamUtil.h>
#include <Common/StringUtil.h>
#include <Common/lofar_datetime.h>
#include <Common/LofarLogger.h> 
#include <Interface/Config.h>
#include <Interface/BeamCoordinates.h>
#include <Stream/Stream.h>
#include <algorithm>
#include <sstream>

#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/format.hpp>

namespace LOFAR {
namespace RTCP {

// \addtogroup APLCommon
// @{

//# Forward Declarations
//class forward;

// The Parset class is a public struct that can be used as base-class
// for holding Parset related information.
// It can be instantiated with a parset containing Parset information.
class Parset: public ParameterSet
{
public:
	Parset(const char *name);
	~Parset();
	 
	std::string    name() const;
	void           check() const;
	void	       maintainBackwardCompatibility();

	uint32         observationID() const;
	double         startTime() const;
	double         stopTime() const;
	uint32	       nrStations() const;
	uint32	       nrTabStations() const;
	uint32	       nrMergedStations() const;
	uint32	       nrBaselines() const;
	uint32         nrCrossPolarisations() const;
	uint32         clockSpeed() const; // Hz
	double         sampleRate() const;
	double         sampleDuration() const;
	uint32	       nrBitsPerSample() const;
	vector<double> positions() const;
	string         positionType() const;
	vector<double> getRefPhaseCentre() const;
	vector<double> getPhaseCentreOf(const string &name) const;	
	double         dispersionMeasure() const;
	uint32         dedispersionFFTsize() const;
	uint32	       CNintegrationSteps() const;
	uint32	       IONintegrationSteps() const;
	uint32	       stokesIntegrationSteps() const;
	uint32	       stokesNrChannelsPerSubband() const;
	double         CNintegrationTime() const;
	double         IONintegrationTime() const;
	uint32         nrSubbandSamples() const;
        uint32         nrSubbandsPerPset() const; 
        uint32         nrSubbandsPerPart() const; 
        uint32         nrPartsPerStokes() const; 
        uint32         nrBeamsPerPset() const; 
	uint32         nrHistorySamples() const;
	uint32         nrSamplesToCNProc() const;
	uint32         inputBufferSize() const; // in samples
	uint32	       maxNetworkDelay() const;
	uint32         nrPPFTaps() const;
	uint32         nrChannelsPerSubband() const;
	uint32         nrCoresPerPset() const;
	vector<unsigned> usedCoresInPset() const;
	vector<unsigned> phaseOneTwoCores() const;
	vector<unsigned> phaseThreeCores() const;
	double         channelWidth() const;
	bool	       delayCompensation() const;
	uint32	       nrCalcDelays() const;
	bool           correctClocks() const;
	double	       clockCorrectionTime(const std::string &station) const;
	bool	       correctBandPass() const;
	bool	       hasStorage() const;
	string         stationName(int index) const;
        vector<string> allStationNames() const;
	uint32	       nrPsetsPerStorage() const;
	unsigned       getLofarStManVersion() const;
	vector<uint32> phaseOnePsets() const;
	vector<uint32> phaseTwoPsets() const;
	vector<uint32> phaseThreePsets() const;
	vector<uint32> usedPsets() const; // union of phasePsets
        bool           phaseThreeDisjunct() const; // if phase 3 does not overlap with phase 1 or 2 in psets or cores
	vector<uint32> tabList() const;
	bool           conflictingResources(const Parset &otherParset, std::stringstream &error) const;
	int	       phaseOnePsetIndex(uint32 pset) const;
	int	       phaseTwoPsetIndex(uint32 pset) const;
	int	       phaseThreePsetIndex(uint32 pset) const;
	string         getTransportType(const string& prefix) const;

        bool           outputFilteredData() const;
        bool           outputCorrelatedData() const;
        bool           outputBeamFormedData() const;
        bool           outputCoherentStokes() const;
        bool           outputIncoherentStokes() const;
        bool           outputTrigger() const;
	unsigned       nrOutputsPerSubband() const;

        bool           fakeInputData() const;

        unsigned       nrStokes() const;
        bool           flysEye() const;
  string     bandFilter() const;
  string     antennaSet() const;



	uint32	       nrPencilBeams() const;
	BeamCoordinates pencilBeams() const;

	vector<unsigned> subbandList() const;
	unsigned         nrSubbands() const;
	unsigned         nrBeams() const;
	unsigned	 nyquistZone() const;

	vector<unsigned> subbandToSAPmapping() const;
	vector<double>	 subbandToFrequencyMapping() const;
	vector<unsigned> subbandToRSPboardMapping(const string &stationName) const;
	vector<unsigned> subbandToRSPslotMapping(const string &stationName) const;

	unsigned       nrSlotsInFrame() const;
	string         partitionName() const;
	bool           realTime() const;
	
	bool	       dumpRawData() const;
	
	vector<double> getBeamDirection(unsigned beam) const;
	string         getBeamDirectionType(unsigned beam) const;
	
	struct StationRSPpair {
	  string   station;
	  unsigned rsp;
	};
	
	vector<StationRSPpair> getStationNamesAndRSPboardNumbers(unsigned psetNumber) const;

	string         getInputStreamName(const string &stationName, unsigned rspBoardNumber) const;

	string         observerName() const;
	string         projectName() const;
  string         contactName() const;

	vector<double> itsStPositions;

        vector<string> fileNames(const string &prefix) const;
        vector<string> fileLocations(const string &prefix) const;
        string         fileNameMask(const string &prefix) const;
        string         targetDirectory(const string &prefix, const string &filename) const;
        string         targetHost(const string &prefix, const string &filename) const;

        string         constructSubbandFilename( const string &mask, unsigned subband ) const;
        string         constructBeamFormedFilename( const string &mask, unsigned beam, unsigned stokes, unsigned file ) const;

        bool            PLC_controlled() const;
        string          PLC_ProcID() const;
        string          PLC_Host() const;
        uint32          PLC_Port() const;

private:
	const std::string itsName;

	void           checkSubbandCount(const char *key) const;
	void	       checkInputConsistency() const;

	uint32         nrManualPencilBeams() const;
	vector<double> getManualPencilBeam(unsigned pencil) const;
	uint32	       nrPencilRings() const;
	double	       pencilRingSize() const;

	void           addPosition(string stName);
	double	       getTime(const char *name) const;
	static int     findIndex(uint32 pset, const vector<uint32> &psets);
	
	vector<double>   centroidPos(const string &stations) const;

	bool	       compatibleInputSection(const Parset &otherParset, std::stringstream &error) const;
	bool	       disjointCores(const Parset &, std::stringstream &error) const;
};

// @}

inline std::string Parset::name() const
{
  return itsName;
}


inline uint32 Parset::observationID() const
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

inline uint32 Parset::nrStations() const
{
  return getStringVector("OLAP.storageStationNames",true).size();
} 

inline uint32 Parset::nrTabStations() const
{
  return getStringVector("OLAP.tiedArrayStationNames",true).size();
}

inline uint32 Parset::nrMergedStations() const
{
  if (tabList().empty()) {
    return nrStations();
  }

  return *std::max_element( tabList().begin(), tabList().end() ) + 1;
}   

inline uint32 Parset::nrBaselines() const
{
  unsigned stations;
  
  if (nrTabStations() > 0)
    stations = nrTabStations();
  else
    stations = nrStations();

  return stations * (stations + 1) / 2;
} 

inline uint32 Parset::nrCrossPolarisations() const
{
  return (getUint32("Observation.nrPolarisations") * getUint32("Observation.nrPolarisations"));
}

inline uint32 Parset::clockSpeed() const
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

inline uint32 Parset::dedispersionFFTsize() const
{
  return isDefined("OLAP.CNProc.dedispersionFFTsize") ? getUint32("OLAP.CNProc.dedispersionFFTsize") : CNintegrationSteps();
}

inline uint32 Parset::nrBitsPerSample() const
{
  return getUint32("OLAP.nrBitsPerSample");
}

inline uint32 Parset::CNintegrationSteps() const
{
  return getUint32("OLAP.CNProc.integrationSteps");
}

inline uint32 Parset::IONintegrationSteps() const
{
  return getUint32("OLAP.IONProc.integrationSteps");
}

inline uint32 Parset::stokesIntegrationSteps() const
{
  return getUint32("OLAP.Stokes.integrationSteps");
}

inline uint32 Parset::stokesNrChannelsPerSubband() const
{
  return getUint32("OLAP.Stokes.channelsPerSubband");
}

inline bool Parset::outputFilteredData() const
{
  return getBool("Observation.Dataproducts.Output_FilteredData.enabled",false);
}

inline bool Parset::outputCorrelatedData() const
{
  return getBool("Observation.Dataproducts.Output_Correlated.enabled",false);
}

inline bool Parset::outputBeamFormedData() const
{
  return getBool("Observation.Dataproducts.Output_Beamformed.enabled",false);
}

inline bool Parset::outputCoherentStokes() const
{
  return getBool("Observation.Dataproducts.Output_CoherentStokes.enabled",false);
}

inline bool Parset::outputIncoherentStokes() const
{
  return getBool("Observation.Dataproducts.Output_IncoherentStokes.enabled",false);
}

inline bool Parset::outputTrigger() const
{
  return getBool("Observation.Dataproducts.Output_Trigger.enabled",false);
}

inline unsigned Parset::nrOutputsPerSubband() const
{
  return (outputFilteredData()	   ? 1 : 0) +
	 (outputCorrelatedData()   ? 1 : 0) +
	 (outputBeamFormedData()   ? 1 : 0) +
	 (outputCoherentStokes()   ? 1 : 0) +
	 (outputIncoherentStokes() ? 1 : 0) +
	 (outputTrigger()          ? 1 : 0);
}

inline bool Parset::fakeInputData() const
{
  return getBool("OLAP.CNProc.fakeInputData",false);
}

inline unsigned Parset::nrStokes() const
{
  if( getString("OLAP.Stokes.which","") == "I" ) {
    return 1;
  } else if( getString("OLAP.Stokes.which","") == "IQUV" ) {
    return 4;
  } else {  
    // backward compatibility
    return 1;
  }
}

inline double Parset::CNintegrationTime() const
{
  return nrSubbandSamples() / sampleRate();
}

inline double Parset::IONintegrationTime() const
{
  return CNintegrationTime() * IONintegrationSteps();
}

inline uint32 Parset::nrSubbandSamples() const
{
  return CNintegrationSteps() * nrChannelsPerSubband();
}

inline uint32 Parset::nrHistorySamples() const
{
  return (nrPPFTaps() - 1) * nrChannelsPerSubband();
}

inline uint32 Parset::nrSamplesToCNProc() const
{
  return nrSubbandSamples() + nrHistorySamples() + 32 / (NR_POLARIZATIONS * 2 * nrBitsPerSample() / 8);
}

inline uint32 Parset::inputBufferSize() const
{
  return (uint32) (getDouble("OLAP.nrSecondsOfBuffer") * sampleRate());
}

inline uint32 Parset::maxNetworkDelay() const
{
  return (uint32) (getDouble("OLAP.maxNetworkDelay") * sampleRate());
}

inline uint32 Parset::nrSubbandsPerPset() const
{
  return getUint32("OLAP.subbandsPerPset");
}

inline uint32 Parset::nrSubbandsPerPart() const
{
  return getUint32("OLAP.Storage.subbandsPerPart");
}

inline uint32 Parset::nrPartsPerStokes() const
{
  return getUint32("OLAP.Storage.partsPerStokes");
}

inline uint32 Parset::nrBeamsPerPset() const
{
  return getUint32("OLAP.PencilInfo.beamsPerPset");
}

inline uint32 Parset::nrPPFTaps() const
{
  return getUint32("OLAP.CNProc.nrPPFTaps");
}

inline uint32 Parset::nrChannelsPerSubband() const
{
  return getUint32("Observation.channelsPerSubband");
}

inline vector<unsigned> Parset::usedCoresInPset() const
{
  return getUint32Vector("OLAP.CNProc.usedCoresInPset",true);
}

inline vector<unsigned> Parset::phaseOneTwoCores() const
{
  return getUint32Vector("OLAP.CNProc.phaseOneTwoCores",true);
}

inline vector<unsigned> Parset::phaseThreeCores() const
{
  return getUint32Vector("OLAP.CNProc.phaseThreeCores",true);
}  
 
inline uint32 Parset::nrCoresPerPset() const
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

inline uint32 Parset::nrCalcDelays() const
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

inline uint32 Parset::nrPsetsPerStorage() const
{
  return getUint32("OLAP.psetsPerStorage");
}

inline unsigned Parset::getLofarStManVersion() const
{
  return getUint32("OLAP.LofarStManVersion", 1); 
}

inline vector<uint32> Parset::phaseOnePsets() const
{
  return getUint32Vector("OLAP.CNProc.phaseOnePsets",true);
}

inline vector<uint32> Parset::phaseTwoPsets() const
{
  return getUint32Vector("OLAP.CNProc.phaseTwoPsets",true);
}

inline vector<uint32> Parset::phaseThreePsets() const
{
  return getUint32Vector("OLAP.CNProc.phaseThreePsets",true);
}

inline bool Parset::phaseThreeDisjunct() const
{
  return getBool("OLAP.CNProc.phaseThreeDisjunct");
}

inline vector<uint32> Parset::tabList() const
{
  return getUint32Vector("OLAP.CNProc.tabList",true);
}

inline int Parset::phaseOnePsetIndex(uint32 pset) const
{
  return findIndex(pset, phaseOnePsets());
}

inline int Parset::phaseTwoPsetIndex(uint32 pset) const
{
  return findIndex(pset, phaseTwoPsets());
}

inline int Parset::phaseThreePsetIndex(uint32 pset) const
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

inline uint32 Parset::nrPencilRings() const
{
  return getUint32("OLAP.PencilInfo.nrRings");
}

inline uint32 Parset::nrManualPencilBeams() const
{
  return getUint32("OLAP.nrPencils");
}

inline uint32 Parset::nrPencilBeams() const
{
  return 3 * nrPencilRings() * (nrPencilRings() + 1) + nrManualPencilBeams();
}

inline BeamCoordinates Parset::pencilBeams() const
{
  // include both the pencil rings and the manually defined pencil beam coordinates
  BeamRings coordinates(nrPencilRings(), pencilRingSize());

  for (unsigned i = 0; i < nrManualPencilBeams(); i ++) {
    const std::vector<double> coords = getManualPencilBeam(i);

    // assume ra,dec
    coordinates += BeamCoord3D(coords[0],coords[1]);
  }

  return coordinates;
}

inline double Parset::pencilRingSize() const
{
  return getDouble("OLAP.PencilInfo.ringSize");
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

inline vector<string> Parset::fileNames(const string &prefix) const
{
  return getStringVector(prefix + ".filenames",true);
}

inline vector<string> Parset::fileLocations(const string &prefix) const
{
  return getStringVector(prefix + ".locations",true);
}

inline string Parset::fileNameMask(const string &prefix) const
{
  return getString(prefix + ".namemask");
}

inline string Parset::targetDirectory(const string &prefix, const string &filename) const
{
  vector<string> locations = fileLocations( prefix );
  vector<string> filenames = fileNames( prefix );

  // TODO: cache and use a map or hashtable

  for (unsigned i = 0; i < filenames.size(); i++ )
    if (filenames[i] == filename) {
      const string &location = locations[i];
      vector<string> parts;

      parts = StringUtil::split(location, ':');

      return parts[1];
    }  

  return "";  
}

inline string Parset::targetHost(const string &prefix, const string &filename) const
{
  vector<string> locations = fileLocations( prefix );
  vector<string> filenames = fileNames( prefix );

  // TODO: cache and use a map or hashtable

  for (unsigned i = 0; i < filenames.size(); i++ )
    if (filenames[i] == filename) {
      const string &location = locations[i];
      vector<string> parts;

      parts = StringUtil::split(location, ':');

      return parts[0];
    }  

  return "";  
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

} // namespace RTCP
} // namespace LOFAR

#endif // ! defined HAVE_BGP_CN
#endif
