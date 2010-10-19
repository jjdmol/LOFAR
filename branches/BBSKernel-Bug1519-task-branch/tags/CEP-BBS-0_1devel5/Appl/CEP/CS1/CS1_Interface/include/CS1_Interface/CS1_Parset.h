//#  CS1_Parset.h: class/struct that holds the CS1_Parset information
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

#ifndef LOFAR_CS1_INTERFACE_CS1_PARSET_H
#define LOFAR_CS1_INTERFACE_CS1_PARSET_H

#if defined HAVE_APS

// \file
// class/struct that holds the CS1_Parset information

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <APS/ParameterSet.h>
#include <Common/StreamUtil.h>
#include <Common/lofar_datetime.h>
#include <Common/LofarLogger.h> 
#include <CS1_Interface/CS1_Config.h>
#include <ApplCommon/Observation.h>
#include <Stream/Stream.h>

#include <boost/date_time/c_local_time_adjustor.hpp>

namespace LOFAR {
namespace CS1 {

// \addtogroup APLCommon
// @{

//# Forward Declarations
//class forward;

// The CS1_Parset class is a public struct that can be used as base-class
// for holding CS1_Parset related information.
// It can be instantiated with a parset containing CS1_Parset information.
class CS1_Parset: public ACC::APS::ParameterSet
{
public:
	CS1_Parset();
	~CS1_Parset();
	explicit CS1_Parset (ACC::APS::ParameterSet* aParSet)
	: ACC::APS::ParameterSet(*aParSet),
	itsObservation(aParSet)
         {}
	 
	void           check() const;

	double         startTime() const;
	double         stopTime() const;
	uint32	       nrStations() const;
	uint32	       nrBaselines() const;
	double         sampleRate() const;
	double         sampleDuration() const;
	vector<double> positions() const;
	vector<double> getRefPhaseCentres() const;
	vector<double> getPhaseCentresOf(const string& name) const;	
	uint32	       BGLintegrationSteps() const;
	uint32	       IONintegrationSteps() const;
	uint32	       storageIntegrationSteps() const;
	double         BGLintegrationTime() const;
	double         IONintegrationTime() const;
	double         storageIntegrationTime() const;
	uint32         nrSubbandSamples() const;
        uint32         nrSubbandsPerPset() const; 
	uint32         nrHistorySamples() const;
	uint32         nrSamplesToBGLProc() const;
	uint32         inputBufferSize() const; // in samples
	uint32	       maxNetworkDelay() const;
	uint32         nrPPFTaps() const;
	uint32         nrChannelsPerSubband() const;
	uint32         nrPsets() const;
	uint32         nrCoresPerPset() const;
	double         channelWidth() const;
	bool	       delayCompensation() const;
	bool	       correctBandPass() const;
	vector<string> getPortsOf(const string& aKey) const;
	string         stationName(const int index) const;
	static string  expandedArrayString(const string& orgStr);
	uint32	       nrPsetsPerStorage() const;
	vector<uint32> inputPsets() const;
	vector<uint32> outputPsets() const;
	vector<uint32> psetDimensions() const;
	int	       inputPsetIndex(uint32 pset) const;
	int	       outputPsetIndex(uint32 pset) const;
	string	       getMSname(unsigned sb) const;
	string         getTransportType(const string& prefix) const;

	unsigned         nrSubbands() const;
	unsigned         nrBeams() const;
	unsigned	 nyquistZone() const;

	vector<unsigned> subbandToBeamMapping() const;
	vector<double>	 subbandToFrequencyMapping() const;
	vector<unsigned> subbandToRSPboardMapping() const;
	vector<unsigned> subbandToRSPslotMapping() const;

	int32          nrSubbandsPerFrame() const;
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

	string	       name;
	vector<double> itsStPositions;
	
private:
	void           addPosition(string stName);
	double	       getTime(const char *name) const;
	static int     findIndex(uint32 pset, const vector<uint32> &psets);
	void           checkSubbandCount(const char *key) const;
	
	Observation    itsObservation;
};

// @}

inline double CS1_Parset::getTime(const char *name) const
{
  return to_time_t(boost::date_time::c_local_adjustor<ptime>::utc_to_local(time_from_string(getString(name))));
}

inline double CS1_Parset::startTime() const
{
  return getTime("Observation.startTime");
}

inline double CS1_Parset::stopTime() const
{
  return getTime("Observation.stopTime");
}

inline string CS1_Parset::stationName(const int index) const
{
  return getStringVector("OLAP.storageStationNames")[index];
}

inline string CS1_Parset::getTransportType(const string& prefix) const
{
  return getString(prefix + "_Transport");
}

inline uint32 CS1_Parset::nrStations() const
{
  return getStringVector("OLAP.storageStationNames").size();
} 
  
inline uint32 CS1_Parset::nrBaselines() const
{
  unsigned stations = nrStations();

  return stations * (stations + 1) / 2;
} 
  
inline double CS1_Parset::sampleRate() const
{
  return getUint32("Observation.sampleClock") * 1000000.0 / 1024;
} 

inline double CS1_Parset::sampleDuration() const
{
  return 1.0 / sampleRate();
} 

inline uint32 CS1_Parset::BGLintegrationSteps() const
{
  return getUint32("OLAP.BGLProc.integrationSteps");
}

inline uint32 CS1_Parset::IONintegrationSteps() const
{
  return getUint32("OLAP.IONProc.integrationSteps");
}

inline uint32 CS1_Parset::storageIntegrationSteps() const
{
  return getUint32("OLAP.StorageProc.integrationSteps");
}

inline double CS1_Parset::BGLintegrationTime() const
{
  return nrSubbandSamples() / sampleRate();
}

inline double CS1_Parset::IONintegrationTime() const
{
  return BGLintegrationTime() * IONintegrationSteps();
}

inline double CS1_Parset::storageIntegrationTime() const
{
  return IONintegrationTime() * storageIntegrationSteps();
}

inline uint32 CS1_Parset::nrSubbandSamples() const
{
  return BGLintegrationSteps() * nrChannelsPerSubband();
}

inline uint32 CS1_Parset::nrHistorySamples() const
{
  return (nrPPFTaps() - 1) * nrChannelsPerSubband();
}

inline uint32 CS1_Parset::nrSamplesToBGLProc() const
{
  return nrSubbandSamples() + nrHistorySamples() + 32 / sizeof(INPUT_SAMPLE_TYPE[NR_POLARIZATIONS]);
}

inline uint32 CS1_Parset::inputBufferSize() const
{
  return (uint32) (getDouble("OLAP.nrSecondsOfBuffer") * sampleRate()) & ~(32 / sizeof(INPUT_SAMPLE_TYPE[NR_POLARIZATIONS]) - 1);
}

inline uint32 CS1_Parset::maxNetworkDelay() const
{
  return (uint32) (getDouble("OLAP.maxNetworkDelay") * sampleRate());
}

inline uint32 CS1_Parset::nrSubbandsPerPset() const
{
  return getUint32("OLAP.subbandsPerPset");
}

inline uint32 CS1_Parset::nrPPFTaps() const
{
  return getUint32("OLAP.BGLProc.nrPPFTaps");
}

inline uint32 CS1_Parset::nrChannelsPerSubband() const
{
  return getUint32("Observation.channelsPerSubband");
}

inline uint32 CS1_Parset::nrPsets() const
{
  return nrSubbands() / nrSubbandsPerPset();
}

inline uint32 CS1_Parset::nrCoresPerPset() const
{
  return getUint32("OLAP.BGLProc.coresPerPset");
}  
 
inline unsigned CS1_Parset::nrSubbands() const
{
  return getUint32Vector("Observation.subbandList").size();
} 

inline vector<unsigned> CS1_Parset::subbandToBeamMapping() const
{
  return getUint32Vector("Observation.beamList");
}

inline vector<unsigned> CS1_Parset::subbandToRSPboardMapping() const
{
  return getUint32Vector("Observation.rspBoardList");
}


inline vector<unsigned> CS1_Parset::subbandToRSPslotMapping() const
{
  return getUint32Vector("Observation.rspSlotList");
}


inline double CS1_Parset::channelWidth() const
{
  return sampleRate() / nrChannelsPerSubband();
}

inline bool CS1_Parset::delayCompensation() const
{
  return getBool("OLAP.delayCompensation");
}

inline bool CS1_Parset::correctBandPass() const
{
  return getBool("OLAP.correctBandPass");
}

inline uint32 CS1_Parset::nrPsetsPerStorage() const
{
  return getUint32("OLAP.psetsPerStorage");
}

inline vector<uint32> CS1_Parset::inputPsets() const
{
  return getUint32Vector("OLAP.BGLProc.inputPsets");
}

inline vector<uint32> CS1_Parset::outputPsets() const
{
  return getUint32Vector("OLAP.BGLProc.outputPsets");
}

inline vector<uint32> CS1_Parset::psetDimensions() const
{
  return getUint32Vector("OLAP.BGLProc.psetDimensions");
}

inline int CS1_Parset::inputPsetIndex(uint32 pset) const
{
  return findIndex(pset, inputPsets());
}

inline int CS1_Parset::outputPsetIndex(uint32 pset) const
{
  return findIndex(pset, outputPsets());
}

inline int32 CS1_Parset::nrSubbandsPerFrame() const
{
  return getInt32("OLAP.nrSubbandsPerFrame");
}

inline string CS1_Parset::partitionName() const
{
  return getString("OLAP.BGLProc.partition");
}

inline bool CS1_Parset::realTime() const
{
  return getBool("OLAP.realTime");
}


} // namespace CS1
} // namespace LOFAR

#endif // defined HAVE_APS
#endif
