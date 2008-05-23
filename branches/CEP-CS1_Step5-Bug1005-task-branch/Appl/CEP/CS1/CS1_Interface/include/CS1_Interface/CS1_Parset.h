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
	 
	double         startTime() const;
	double         stopTime() const;
	uint32	       nrStations(const int index) const;
	double         sampleRate() const;
	double         sampleDuration() const;
	vector<double> positions(const int index) const;
	vector<double> getRefPhaseCentres(const int index) const;
	vector<double> getPhaseCentresOf(const string& name) const;	
	uint32	       BGLintegrationSteps() const;
	uint32	       IONintegrationSteps() const;
	uint32	       storageIntegrationSteps() const;
	double         BGLintegrationTime() const;
	double         IONintegrationTime() const;
	double         storageIntegrationTime() const;
	uint32         nrSubbandSamples() const;
        uint32         nrSubbandsPerPset(const int index) const; 
	uint32         nrHistorySamples() const;
	uint32         nrSamplesToBGLProc() const;
	uint32         nrSamplesToBuffer() const;
	uint32	       maxNetworkDelay() const;
	uint32         nrRSPboards(const int index) const;
	uint32         nrRSPboardsPerStation(const int index) const;
	uint32         subbandsToReadFromFrame(const int index) const;
	uint32         nrPPFTaps() const;
	uint32         nrChannelsPerSubband() const;
	vector<string> stationNames(const int index) const;
	uint32         nrSubbands(const int index) const;
	uint32         nrPsets(const int index) const;
	uint32         nrCoresPerPset() const;
	vector<double> refFreqs(const int index) const;
	double         chanWidth() const;
	unsigned       portNr(const unsigned coreNr, const int index) const;
	string         inputPortnr(const string& aKey) const;
	string         stationName(const int stationNr, const int index) const;
	uint32         rspId(const string& stationName) const;
	static string  expandedArrayString(const string& orgStr);
	bool	       useScatter() const;
	bool	       useGather() const;
	uint32	       nrPsetsPerStorage(const int index) const;
	uint32	       nrOutputsPerInputNode() const;
	uint32	       nrInputsPerStorageNode(const int index) const;
	vector<uint32> inputPsets(const int index) const;
	vector<uint32> outputPsets(const int index) const;
	int	       inputPsetIndex(uint32 pset,const int index) const;
	int	       outputPsetIndex(uint32 pset, const int index) const;
	string	       getMSname(unsigned sb) const;
	uint32         nrBeams() const;
	vector<int32>  beamlet2beams(const int index) const;
	vector<int32>  beamlet2subbands(const int index) const;
	vector<uint32> subband2Index(const int index) const;
	int32          nrSubbandsPerFrame() const;
	
	vector<double> getBeamDirection(const unsigned currentBeam) const;
	string         getBeamDirectionType(const unsigned currentBeam) const;
	vector<string> partitionList() const;
	uint32         nrStorageNodes() const;
	int            partitionName2Index(const string& pName) const;

	//# Datamembers
	string	       name;
	vector<double> itsStPositions;
	
private:
	void           addPosition(string stName);
	double	       getTime(const char *name) const;
	static int     findIndex(uint32 pset, const vector<uint32> &psets);
	
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

inline string CS1_Parset::stationName(const int stationNr, const int index) const
{
  return stationNames(index)[stationNr];
}

inline uint32 CS1_Parset::nrStations(const int index) const
{
  return stationNames(index).size();
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

inline uint32 CS1_Parset::nrSamplesToBuffer() const
{
  return (uint32) (getDouble("OLAP.nrSecondsOfBuffer") * sampleRate()) & ~(32 / sizeof(INPUT_SAMPLE_TYPE[NR_POLARIZATIONS]) - 1);
}

inline uint32 CS1_Parset::maxNetworkDelay() const
{
  return (uint32) (getDouble("OLAP.maxNetworkDelay") * sampleRate());
}

inline uint32 CS1_Parset::nrRSPboards(const int index) const
{
  return stationNames(index).size();
}

inline uint32 CS1_Parset::nrRSPboardsPerStation(const int index) const
{
  return nrRSPboards(index) / nrStations(index);
}

inline uint32 CS1_Parset::subbandsToReadFromFrame(const int index) const
{
  return nrSubbands(index) * nrStations(index) / nrRSPboards(index);
}

inline uint32 CS1_Parset::nrPPFTaps() const
{
  return getUint32("OLAP.BGLProc.nrPPFTaps");
}

inline uint32 CS1_Parset::nrChannelsPerSubband() const
{
  return getUint32("Observation.channelsPerSubband");
}

inline uint32 CS1_Parset::nrPsets(const int index) const
{
  return nrSubbands(index) / nrSubbandsPerPset(index);
}

inline uint32 CS1_Parset::nrCoresPerPset() const
{
  return getUint32("OLAP.BGLProc.coresPerPset");
}  
 
inline double CS1_Parset::chanWidth() const
{
  return sampleRate() / nrChannelsPerSubband();
}

inline bool CS1_Parset::useScatter() const
{
  return getBool("OLAP.IONProc.useScatter");
}

inline bool CS1_Parset::useGather() const
{
  return getBool("OLAP.IONProc.useGather");
}

inline uint32 CS1_Parset::nrOutputsPerInputNode() const
{
  return useScatter() ? 1 : nrCoresPerPset();
}

inline uint32 CS1_Parset::nrInputsPerStorageNode(const int index) const
{
  return (useGather() ? 1 : nrCoresPerPset()) * nrPsetsPerStorage(index);
}

inline int CS1_Parset::inputPsetIndex(uint32 pset, const int index) const
{
  return findIndex(pset, inputPsets(index));
}

inline int CS1_Parset::outputPsetIndex(uint32 pset, const int index) const
{
  return findIndex(pset, outputPsets(index));
}

inline uint32 CS1_Parset::nrBeams() const
{
  return getUint32("Observation.nrBeams");
}

inline int32  CS1_Parset::nrSubbandsPerFrame() const
{
  return getInt32("OLAP.nrSubbandsPerFrame");
}

inline vector<string>  CS1_Parset::partitionList() const
{
  return getStringVector("Observation.VirtualInstrument.partitionList");
}

inline uint32 CS1_Parset::nrStorageNodes() const
{
  return getUint32("OLAP.nrStorageNodes");
}

} // namespace CS1
} // namespace LOFAR

#endif // defined HAVE_APS
#endif
