//#  StationData.h: classes to hold the data from a station
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

#ifndef LOFAR_CS1_GENERATOR_STATIONDATA_H
#define LOFAR_CS1_GENERATOR_STATIONDATA_H

// \file
// classes to hold the data from a station

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/lofar_complex.h>
#include <CS1_Interface/RSPTimeStamp.h>
#include <APS/ParameterSet.h>

namespace LOFAR 
{
  namespace CS1 
  {

    // \addtogroup CS1_Generator
    // @{

    typedef i16complex RSPDataType;
    
    // Header is the class that represents the header data of the epa packet
    // it contains the following fields:
    //   protocol(int32)
    //   stationId(int32)
    //   sequenceId(int32)
    //   blockId(int32)
    // This class can only be used on little endian machines because the RSPdata is little endian
    class Header
    {
    public:
      explicit Header(char* bufferSpace, const int size) {
	ASSERTSTR(size == theirSize, "Header did not receive the right amount of memory"); 
	itsBufferp = bufferSpace; 
	itsStationIDp = reinterpret_cast<int*>(itsBufferp + theirStatIDOffset); 
	itsSeqIDp = reinterpret_cast<int*>(itsBufferp + theirSeqIDOffset); 
	itsBlockIDp = reinterpret_cast<int*>(itsBufferp + theirBlockIDOffset);
      };
      ~Header() {};

//      int32 getProtocol() const;
//      void setProtocol(int32 newProt);
      int32 getStationId() const		{ return *itsStationIDp;};
      void setStationId(int32 newSid) 		{ *itsStationIDp = newSid;};
      TimeStamp getTimeStamp() const            { return TimeStamp(*itsSeqIDp, *itsBlockIDp);};
      void setTimeStamp(TimeStamp newStamp)     { *itsSeqIDp = newStamp.getSeqId();  *itsBlockIDp = newStamp.getBlockId();};
      static int getSize()		        { return theirSize;};
    protected:
      char* itsBufferp;
      int*  itsStationIDp;
      int*  itsSeqIDp;
      int*  itsBlockIDp;

      static const int theirSize = 16;
      static const int theirStatIDOffset = 4;
      static const int theirSeqIDOffset = 8;
      static const int theirBlockIDOffset = 12;
    };

    class Data
    {
    public:
      explicit Data(RSPDataType* bufferSpace, const unsigned int size, const ACC::APS::ParameterSet& ps) {
	itsNPol = ps.getInt32("NPolarisations"); 
	itsNSubbands = ps.getInt32("NSubbands"); 
	itsNTimes = ps.getInt32("NTimesPerFrame"); 
	ASSERTSTR(itsNPol * itsNSubbands * itsNTimes * sizeof(RSPDataType) == size, "Data did not receive the right amount of memory"); 
	itsBeamlets = bufferSpace; 
      };
      ~Data() {};
      
      RSPDataType& getBeamlet(int seq, int subband, int pol) const { return itsBeamlets[pol + subband * itsNPol + seq * itsNPol * itsNSubbands]; };
      void clear()       { memset(itsBeamlets, 0, itsNPol * itsNSubbands * itsNTimes * sizeof(RSPDataType));};
      int getNPols() const {return itsNPol;};
      int getNSubbands() const {return itsNSubbands;};
      int getNTimes() const {return itsNTimes;};

      static int getSize(const ACC::APS::ParameterSet& ps) { 
	return ps.getInt32("NPolarisations")
	  * ps.getInt32("NSubbands")
	  * ps.getInt32("NTimesPerFrame")
	  * sizeof(RSPDataType); };  

    protected:
      RSPDataType* itsBeamlets;
      
      int itsNPol;
      int itsNSubbands;
      int itsNTimes;
    };
    
    class Frame
    {
    public:
      Frame(char* bufferP, const int size, const ACC::APS::ParameterSet& ps) : itsHeader(bufferP, Header::getSize()), itsData(reinterpret_cast<RSPDataType*>(bufferP + Header::getSize()), size - Header::getSize(), ps) {};
      ~Frame() {};
      
      Data* getData() { return &itsData;};
      Header* getHeader() { return &itsHeader;};

      static int getSize(const ACC::APS::ParameterSet& ps) { return Header::getSize() + Data::getSize(ps);};
      
    private:
      Header itsHeader;
      Data itsData;
    };
            
    // @}

  } // namespace CS1
} // namespace LOFAR

  // @}

#endif
