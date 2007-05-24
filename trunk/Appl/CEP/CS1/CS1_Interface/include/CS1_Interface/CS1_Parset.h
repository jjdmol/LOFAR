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

// \file
// class/struct that holds the CS1_Parset information

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <bitset>
#include <set>
#include <APS/ParameterSet.h>
#include <Common/StreamUtil.h>
#include <Common/lofar_datetime.h>
#include <Common/LofarLogger.h> 

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
	explicit CS1_Parset (const ACC::APS::ParameterSet* aParSet)
	: ACC::APS::ParameterSet(*aParSet)
         {}
	 
	double         startTime() const;
	double         stopTime() const;
	uint32	       nrStations() const;
	double         sampleRate() const;
	vector<double> physicalPhaseCentras() const;
	vector<double> trackingPhaseCentras() const;
	uint32         nrSubbandSamples() const;
	double         integrationTime() const;
	uint32         nrSamplesToBGLProc() const;
        uint32         nrSubbandsPerCell() const; 
	uint32         nrHistorySamples() const;
	uint32         nrSamplesToBuffer() const;
	int            subbandsToReadFromFrame() const;
	uint32         nrPFFTaps() const;
	uint32         nrChannelsPerSubband() const;
	uint32         nrSubbands() const;
	uint32         nrCells() const;
	double         timeInterval() const;
	uint32         nrBGLNodesPerCell() const;
	vector<double> refFreqs() const;
	vector<string> msNames() const;
	double         chanWidth() const;
	vector<string> delay_Ports() const;
	vector<string> getPortsOf(const string& aKey) const;
	uint32         inputPortnr(const string& aKey) const;
	string         stationName(const int index) const;
	string         expandedArrayString(const string& orgStr) const;
	
	//# Datamembers
	string			name;
	vector<double> itsStPositions;
	
private:
        uint32         getStationIndex(const string& aKey) const;
	
	
	void           addPosition(string stName);
};

// @}

inline double CS1_Parset::startTime() const
{
  return  to_time_t(time_from_string(getString("Observation.startTime")));
}

inline double CS1_Parset::stopTime() const
{
  return  to_time_t(time_from_string(getString("Observation.stopTime")));
}

inline uint32 CS1_Parset::inputPortnr(const string& aKey) const
{
  return getUint32("OLAP.firstInputPortnr") + getStationIndex(aKey);
}

inline string CS1_Parset::stationName(const int index) const
{
  return getStringVector("OLAP.storageStationNames")[index];
}

inline uint32 CS1_Parset::nrStations() const
{
  return getStringVector("OLAP.storageStationNames").size();
} 
  
inline double CS1_Parset::sampleRate() const
{
  return getUint32("Observation.sampleClock")*1000000/1024;
} 

inline uint32 CS1_Parset::nrSubbandSamples() const
{
  return getUint32("OLAP.minorIntegrationSteps") * 
         nrChannelsPerSubband();
}

inline double CS1_Parset::integrationTime() const
{
  return nrSubbandSamples()/sampleRate();
}

inline uint32 CS1_Parset::nrSamplesToBGLProc() const
{
  return nrSubbandSamples() + ((nrPFFTaps() - 1) *
         nrChannelsPerSubband());
}

inline uint32 CS1_Parset::nrHistorySamples() const
{
  return (nrPFFTaps() - 1) * nrChannelsPerSubband();
}

inline uint32 CS1_Parset::nrSamplesToBuffer() const
{
  return getUint32("OLAP.nrSecondsOfBuffer") * nrSubbandSamples();
}

inline int CS1_Parset::subbandsToReadFromFrame() const
{
  return nrSubbands() * nrStations() / getInt32("OLAP.nrRSPboards");
}

inline uint32 CS1_Parset::nrSubbandsPerCell() const
{
  return getUint32("OLAP.subbandsPerPset") * 
         getUint32("OLAP.BGLProc.psetsPerCell");
}

inline uint32 CS1_Parset::nrPFFTaps() const
{
  return getUint32("OLAP.BGLProc.nrPPFTaps");
}

inline uint32 CS1_Parset::nrChannelsPerSubband() const
{
  return getUint32("Observation.channelsPerSubband");
}

inline uint32 CS1_Parset::nrSubbands() const
{
  return getUint32Vector("Observation.subbandList").size();
}
  
inline uint32 CS1_Parset::nrCells() const
{
  return nrSubbands() / nrSubbandsPerCell();
}

inline double CS1_Parset::timeInterval() const
{
  return nrSubbandSamples() / sampleRate() / nrSubbandsPerCell();
}

inline uint32 CS1_Parset::nrBGLNodesPerCell() const
{
  return getUint32("OLAP.BGLProc.nodesPerPset") * 
         getUint32("OLAP.BGLProc.psetsPerCell");
}  
 
inline double CS1_Parset::chanWidth() const
{
  return sampleRate() / nrChannelsPerSubband();
}

  } // namespace CS1
} // namespace LOFAR

#endif
