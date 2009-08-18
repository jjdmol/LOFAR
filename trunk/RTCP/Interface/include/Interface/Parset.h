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
#include <Common/lofar_datetime.h>
#include <Common/LofarLogger.h> 
#include <Interface/Config.h>
#include <Interface/PencilCoordinates.h>
#include <ApplCommon/Observation.h>
#include <Stream/Stream.h>

#include <boost/date_time/c_local_time_adjustor.hpp>

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
	Parset();
	Parset(ParameterSet *aParSet);
	~Parset();
	 
	void           check() const;

	double         startTime() const;
	double         stopTime() const;
	uint32	       nrStations() const;
	uint32	       nrTabStations() const;
	uint32	       nrBaselines() const;
	uint32         nrCrossPolarisations() const;
	double         sampleRate() const;
	double         sampleDuration() const;
	uint32	       nrBitsPerSample() const;
	vector<double> positions() const;
	string         positionType() const;
	vector<double> getRefPhaseCentres() const;
	vector<double> getPhaseCentresOf(const string& name) const;	
	uint32	       CNintegrationSteps() const;
	uint32	       IONintegrationSteps() const;
	uint32	       stokesIntegrationSteps() const;
	double         CNintegrationTime() const;
	double         IONintegrationTime() const;
	uint32         nrSubbandSamples() const;
        uint32         nrSubbandsPerPset() const; 
	uint32         nrHistorySamples() const;
	uint32         nrSamplesToCNProc() const;
	uint32         inputBufferSize() const; // in samples
	uint32	       maxNetworkDelay() const;
	uint32         nrPPFTaps() const;
	uint32         nrChannelsPerSubband() const;
	uint32         nrPsets() const;
	uint32         nrCoresPerPset() const;
	double         channelWidth() const;
	bool	       delayCompensation() const;
	uint32	       nrCalcDelays() const;
	bool	       correctBandPass() const;
	vector<string> getPortsOf(const string& aKey) const;
	string         stationName(const int index) const;
	string         storageHostName(const string& aKey, const int index) const;
	uint32	       nrPsetsPerStorage() const;
	vector<uint32> inputPsets() const;
	vector<uint32> outputPsets() const;
	vector<uint32> tabList() const;
	int	       inputPsetIndex(uint32 pset) const;
	int	       outputPsetIndex(uint32 pset) const;
	string	       getMSname(unsigned sb) const;
	string         getMSBaseDir() const;
	string         getTransportType(const string& prefix) const;
	string         getModeName() const;
	bool           outputIncoherentStokesI() const;
	bool           stokesIntegrateChannels() const;

	uint32	       nrPencilBeams() const;
	PencilCoordinates pencilBeams() const;

	unsigned         nrSubbands() const;
	unsigned         nrBeams() const;
	unsigned	 nyquistZone() const;

	vector<unsigned> subbandToBeamMapping() const;
	vector<double>	 subbandToFrequencyMapping() const;
	vector<unsigned> subbandToRSPboardMapping() const;
	vector<unsigned> subbandToRSPslotMapping() const;

	int32          nrSlotsInFrame() const;
	string         partitionName() const;
	bool           realTime() const;
	
	vector<double> getBeamDirection(unsigned beam) const;
	string         getBeamDirectionType(unsigned beam) const;
	
	struct StationRSPpair {
	  string   station;
	  unsigned rsp;
	};
	
	vector<StationRSPpair> getStationNamesAndRSPboardNumbers(unsigned psetNumber) const;

	string         getInputStreamName(const string &stationName, unsigned rspBoardNumber) const;
	static Stream  *createStream(const string &description, bool asReader);

	string         observerName() const;
	string         projectName() const;

	string	       name;
	vector<double> itsStPositions;
	
private:
	uint32         nrManualPencilBeams() const;
	vector<double> getManualPencilBeam( const unsigned pencil ) const;
	uint32	       nrPencilRings() const;
	double	       pencilRingSize() const;

	void           addPosition(string stName);
	double	       getTime(const char *name) const;
	static int     findIndex(uint32 pset, const vector<uint32> &psets);
	void           checkSubbandCount(const char *key) const;
	void           checkSubbandCountFromObservation(const char *key, const vector<uint32> &list) const;
	
	vector<double>   centroidPos(const string &stations) const;
	
	Observation    itsObservation;
};

// @}

inline double Parset::getTime(const char *name) const
{
  return to_time_t(boost::date_time::c_local_adjustor<ptime>::utc_to_local(time_from_string(getString(name))));
}

inline double Parset::startTime() const
{
  return getTime("Observation.startTime");
}

inline double Parset::stopTime() const
{
  return getTime("Observation.stopTime");
}

inline string Parset::stationName(const int index) const
{
  return getStringVector("OLAP.storageStationNames")[index];
}

inline string Parset::storageHostName(const string& aKey, const int index) const
{
  return getStringVector(aKey)[getUint32Vector("OLAP.storageNodeList")[index]];
}


inline string Parset::getTransportType(const string& prefix) const
{
  return getString(prefix + "_Transport");
}

inline uint32 Parset::nrStations() const
{
  return getStringVector("OLAP.storageStationNames").size();
} 

inline uint32 Parset::nrTabStations() const
{
  return getStringVector("OLAP.tiedArrayStationNames").size();
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
  
inline double Parset::sampleRate() const
{
  return getUint32("Observation.sampleClock") * 1000000.0 / 1024;
} 

inline double Parset::sampleDuration() const
{
  return 1.0 / sampleRate();
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
  return getUint32("Observation.stokesIntegrationSteps");
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

inline uint32 Parset::nrPPFTaps() const
{
  return getUint32("OLAP.CNProc.nrPPFTaps");
}

inline uint32 Parset::nrChannelsPerSubband() const
{
  return getUint32("Observation.channelsPerSubband");
}

inline uint32 Parset::nrPsets() const
{
  return getUint32("OLAP.nrPsets");
}

inline uint32 Parset::nrCoresPerPset() const
{
  return getUint32("OLAP.CNProc.coresPerPset");
}  
 
inline unsigned Parset::nrSubbands() const
{
  if (isDefined("Observation.subbandList"))
    return get("Observation.subbandList").expand().getUint32Vector().size();
  else
    return itsObservation.getSubbandList().size();   
} 

inline vector<unsigned> Parset::subbandToBeamMapping() const
{
  if (isDefined("Observation.subbandList"))
    return get("Observation.beamList").expand().getUint32Vector();
  else
    return itsObservation.getBeamList();   
}

inline vector<unsigned> Parset::subbandToRSPboardMapping() const
{
  if (isDefined("Observation.subbandList"))
    return get("Observation.rspBoardList").expand().getUint32Vector();
  else
    return itsObservation.getRspBoardList();
}


inline vector<unsigned> Parset::subbandToRSPslotMapping() const
{
  if (isDefined("Observation.subbandList"))
    return get("Observation.rspSlotList").expand().getUint32Vector();
  else
    return itsObservation.getRspSlotList();
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

inline bool Parset::correctBandPass() const
{
  return getBool("OLAP.correctBandPass");
}

inline uint32 Parset::nrPsetsPerStorage() const
{
  return getUint32("OLAP.psetsPerStorage");
}

inline vector<uint32> Parset::inputPsets() const
{
  return getUint32Vector("OLAP.CNProc.inputPsets");
}

inline vector<uint32> Parset::outputPsets() const
{
  return getUint32Vector("OLAP.CNProc.outputPsets");
}

inline vector<uint32> Parset::tabList() const
{
  return getUint32Vector("OLAP.CNProc.tabList");
}

inline int Parset::inputPsetIndex(uint32 pset) const
{
  return findIndex(pset, inputPsets());
}

inline int Parset::outputPsetIndex(uint32 pset) const
{
  return findIndex(pset, outputPsets());
}

inline int32 Parset::nrSlotsInFrame() const
{
  return getInt32("Observation.nrSlotsInFrame");
}

inline string Parset::partitionName() const
{
  return getString("OLAP.CNProc.partition");
}

inline bool Parset::realTime() const
{
  return getBool("OLAP.realTime");
}

inline string Parset::getModeName() const
{
  return getString("Observation.mode");
}

inline uint32 Parset::nrPencilRings() const
{
  return getUint32("Observation.nrPencilRings");
}

inline uint32 Parset::nrManualPencilBeams() const
{
  return getUint32("Observation.nrPencils");
}

inline uint32 Parset::nrPencilBeams() const
{
  return 3 * nrPencilRings() * (nrPencilRings() + 1) + 1 + nrManualPencilBeams();
}

inline PencilCoordinates Parset::pencilBeams() const
{
  // include both the pencil rings and the manually defined pencil beam coordinates
  PencilRings coordinates( nrPencilRings(), pencilRingSize() );
  for( unsigned i = 0; i < nrManualPencilBeams(); i++ ) {
    coordinates += PencilCoord3D( getManualPencilBeam( i ) );
  }

  return coordinates;
}

inline double Parset::pencilRingSize() const
{
  return getDouble("Observation.pencilRingSize");
}

inline bool Parset::outputIncoherentStokesI() const
{
  return getBool("Observation.outputIncoherentStokesI");
}

inline bool Parset::stokesIntegrateChannels() const
{
  return getBool("Observation.stokesIntegrateChannels");
}

inline string Parset::observerName() const
{
  // TODO - this should be included in the Parset from SAS
  return "LOFAR";
}

inline string Parset::projectName() const
{
  // TODO - this should be included in the Parset from SAS
  return "LOFAR";
}

} // namespace RTCP
} // namespace LOFAR

#endif // ! defined HAVE_BGP_CN
#endif
