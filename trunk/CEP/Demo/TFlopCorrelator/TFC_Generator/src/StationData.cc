//#  StationData.cc: classes to hold the data from a station
//#
//#  Copyright (C) 2002-2005
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <TFC_Generator/StationData.h>

namespace LOFAR
{
  // EpaHeader constructor and destructor
  EpaHeader::EpaHeader(char* bufferP)
    : itsBufferp(bufferP)
  {};

  EpaHeader::~EpaHeader()
  {};


  // EpaPacket constructor and destructor
  EpaPacket::EpaPacket(char* bufferSpace, int bufferSize, ParameterSet ps)
    : itsBufferp(bufferSpace),
      itsEpaHeader(bufferSpace),
      itsMatrix(0)
  {
    int NoSubbands = ps.getInt32("Input.SzEPApayload") / ( 2 * sizeof(RSPDataType));
    ASSERTSTR(NoSubbands * 2 * sizeof(RSPDataType) == ps.getInt32("Input.SzEPApayload"), "SzEPApayload is not a multiple of the size of 1 subband");
    int BufferSize = EpaPacket::getSize(ps);

    ASSERTSTR(bufferSize == BufferSize, "EpaPacket received the wrong amount of buffer data");
    
    // create the RectMatrix
    vector<DimDef> vdd;
    vdd.push_back(DimDef("subband", NoSubbands));
    vdd.push_back(DimDef("polarisation", 2));
    itsMatrix = new RectMatrix<RSPDataType>(vdd);
    itsMatrix->setBuffer((RSPDataType*)(itsBufferp + EpaHeader::getSize()), 2*NoSubbands);
  };

  EpaPacket::~EpaPacket()
  {
    delete itsMatrix;
  };

  // EthernetFrame constructor and destructor
  EthernetFrame::EthernetFrame(ParameterSet ps, char* bufferP, int bufferSize) 
  {
    if (((int)bufferP)*bufferSize == 0) {
      // if either bufferP or the size is 0, allocate the memory myself
      itsIsMemoryMine = true;
    } else {
      itsIsMemoryMine = false;
    }
    int noEpaP = ps.getInt32("Input.NPacketsInFrame");
    int epaSize = EpaPacket::getSize(ps);
    itsBufferSize = EthernetFrame::getSize(ps);
    if (itsIsMemoryMine) {
      itsBufferp = new char[itsBufferSize];
    } else {
      ASSERTSTR(itsBufferSize <= bufferSize, "EthernetFrame received too little memory");
      itsBufferp = bufferP;
    }
    for (int epap = 0; epap < noEpaP; epap++) {
      itsEpaPackets.push_back(new EpaPacket(&(itsBufferp[epap * epaSize]), epaSize, ps));
    }
  };

  EthernetFrame::~EthernetFrame()
  {
    vector<EpaPacket*>::iterator epap = itsEpaPackets.begin();
    for (; epap != itsEpaPackets.end(); epap++) {
      delete *epap;
    }
    if (itsIsMemoryMine) {
      delete [] itsBufferp;
    }
  };


  // Some accessors to the EpaHeader that could not be inlined because
  //   of the ifdef WORDS_BIGENDIAN
  int16 EpaHeader::getProtocol()
  { 
#ifdef WORDS_BIGENDIAN
    int16 ret;
    char* valuep = (char*)&ret;
    valuep[0] = itsBufferp[1];
    valuep[1] = itsBufferp[0];
    return ret;
#else 
    return (int16)itsBufferp[0];
#endif
  };
  void EpaHeader::setProtocol(int16 newProt)
  {
#ifdef WORDS_BIGENDIAN
    itsBufferp[0] = ((const char*)(&newProt))[1];
    itsBufferp[1] = ((const char*)(&newProt))[0];
#else 
    itsBufferp[0] = ((const char*)(&newProt))[0];
    itsBufferp[1] = ((const char*)(&newProt))[1];
#endif
  };

  int32 EpaHeader::getInt32(char* memLoc)
  {
#ifdef WORDS_BIGENDIAN
    int32 ret;
    char* valuep = &ret;
    valuep[0] = memloc[3];
    valuep[1] = memloc[2];
    valuep[2] = memloc[1];
    valuep[3] = memloc[0];
    return ret;
#else 
    return ((int32*)memLoc)[0];
#endif
  }
  void EpaHeader::setInt32(char* memLoc, const char* newValueP)
  {
#ifdef WORDS_BIGENDIAN
    memLoc[0] = (newValueP)[3];
    memLoc[1] = (newValueP)[2];
    memLoc[2] = (newValueP)[1];
    memLoc[3] = (newValueP)[0];
#else 
    memLoc[0] = (newValueP)[0];
    memLoc[1] = (newValueP)[1];
    memLoc[2] = (newValueP)[2];
    memLoc[3] = (newValueP)[3];
#endif
  }
  
} // namespace LOFAR

