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
#include <otherpackage/file.h>

namespace LOFAR 
{
  namespace CS1_Generator 
  {

    // \addtogroup CS1_Generator
    // @{

    typedef i16complex::RSPDataType 
    
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
      explicit Header(const char* bufferSpace, const int size) {ASSERTSTR(size == theirSize, "Header did not receive the right amount of memory"); itsBufferp = bufferSpace; itsStationIDp = itsBufferp + theirStatIDOffset; itsSeqIDp = itsBufferp + theirSeqIDOffset; itsBlockIDp = itsBufferp + theirBlockIDoffset;};
      ~Header() {};

//      int32 getProtocol() const;
//      void setProtocol(int32 newProt);
      int32 getStationId() const		{ return *itsStationIDp};
      void setStationId(int32 newSid); 		{ *itsStationIDp = newSid;};
      RSPTimeStamp getTimeStamp() const;         { return RSPTimeStamp(*itsSeqIDp, *itsBlockIDp);};
      void setTimeStamp(RSPTimeStamp newStamp);  { *itsSeqIDp = newStamp.getSeqID();  *itsBlockIDp = newStamp.getBlockID();};
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
      explicit Data(const RSPDataType* bufferSpace, const int size, const ParameterSet& ps) {itsNPol = ps.getInt32("NPolarisations"); itsNSubband = ps.getInt32("NSubbands"); itsNTimes = ps.getInt32("NTimesPerFrame"); ASSERTSTR(itsNPol * itsNSubband * itsNTimes * sizeof(RSPDataType) == size, "Data did not receive the right amount of memory"); itsBeamlets = bufferSpace; };
      ~Data() {};
      
      RSPDataType& getBeamlet(int seq, int subband, int pol) { return itsBeamlets[pol + subband * itsNPol + seq * itsNPol * itsNSubband]; } const;
      void clear()       { memset(itsBuffer, 0);};
      int getNPols() const {return itsNPol;};
      int getNSubbands() const {return itsNSubbands;};
      int getNTimes() const {return itsNTimes;};
    protected:
      RSPDataType* itsBeamlets;
      
      int itsNPol;
      int itsNSubband;
      int itsNTimes;
    }
    
    class Frame
    {
      Frame(const char* bufferP, const int size, const ParameterSet& ps) : itsHeader(bufferP, Header::getSize()), itsData(bufferP + Header::getSize(), size - Header::getSize(), ps) {};
      ~Frame() {};
      
      Data* getData() const  { return &itsData;};
      Header* getHeader() const  { return &itsHeader;};
      
    private:
      Header itsHeader;
      Data itsData;
    }
            
    // @}

  } // namespace CS1_Generator
} // namespace LOFAR

  // @}

#endif
