//#  StationData.h: classes to hold the data from a station
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

#ifndef TFC_STATIONDATA_H
#define TFC_STATIONDATA_H

#include <APS/ParameterSet.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_complex.h>
#include <TFC_Interface/RectMatrix.h>

namespace LOFAR
{
  typedef TYPES::i16complex RSPDataType;
  using ACC::APS::ParameterSet;
  
  // EpaHeader is the class that represents the header data of the epa packet
  // it contains the following fields:
  //   protocol(int16)
  //   stationId(int32)
  //   sequenceId(int32)
  //   blockId(int32)
  // Because there is an int16 in here, we need to do some strange copying with pointers
  // These operations are defined below.
  class EpaHeader
  {
  public:
    ~EpaHeader();

    int16 getProtocol();
    void setProtocol(int16 newProt);
    int32 getStationId();
    void setStationId(int32 newSid);
    int32 getSeqId();
    void setSeqId(int32 newSid);
    int32 getBlockId();
    void setBlockId(int32 newBid);
    static int getSize();
  protected:
    EpaHeader(EpaHeader&);
    int32 getInt32(char* memLoc);
    void setInt32(char* memLoc, const char* newValueP);
    friend class EpaPacket;
    EpaHeader(char* bufferSpace);
    char* itsBufferp;
  };


  // EpaPacket represents 1 packet in an ethernet frame
  // it contains:
  //    1 EpaHeader 
  //    a matrix containing:
  //       NoSubbands * noPolarisations(=2) * complex<int16>
  class EpaPacket
  {
  public:
    ~EpaPacket();

    EpaHeader& getHeader();
    RectMatrix<RSPDataType>& getMatrix();
    static int getSize(ParameterSet& ps);
  protected:
    EpaPacket(EpaPacket&);
    friend class EthernetFrame;
    // a packet should be constructed only by a EthernetFrame
    EpaPacket(char* bufferSpace, int bufferSize, ParameterSet ps);

    char* itsBufferp;
    EpaHeader itsEpaHeader;
    RectMatrix<RSPDataType>* itsMatrix;
  };

  // EthernetFrame represents the contents of an Ethernet frame
  // it contains a NoPacketsInFrame EpaPackets
  // it contains a buffer that holds all the actual data (from the header and the packets)
  // this data can be retreived by using getPayloadp() and getPayloadSize()
  class EthernetFrame
  {
  public:
    EthernetFrame(ParameterSet ps, char* bufferP = 0, int bufferSize = 0);
    ~EthernetFrame();
    int getNoPacketsInFrame();
    EpaPacket& getEpaPacket(int index);
    char* getPayloadp();
    int getPayloadSize();
    void reset();
    static int getSize(ParameterSet& ps);
  protected:
    EthernetFrame(EthernetFrame&);
    vector<EpaPacket*> itsEpaPackets;
    char* itsBufferp;
    int itsBufferSize;
    bool itsIsMemoryMine;
  };

  inline int EpaHeader::getSize()
  { return 14; };
  inline int32 EpaHeader::getStationId()
  { return getInt32(&itsBufferp[2]); };
  inline void EpaHeader::setStationId(int32 sid)
  { setInt32(&itsBufferp[2], (const char*)&sid); };
  inline int32 EpaHeader::getSeqId()
  { return getInt32(&itsBufferp[6]); };
  inline void EpaHeader::setSeqId(int32 sid)
  { setInt32(&itsBufferp[6], (const char*)&sid); };
  inline int32 EpaHeader::getBlockId()
  { return getInt32(&itsBufferp[10]); };
  inline void EpaHeader::setBlockId(int32 bid)
  { setInt32(&itsBufferp[10], (const char*)&bid); };

  inline EpaHeader& EpaPacket::getHeader()
    { return itsEpaHeader; };
  inline RectMatrix<RSPDataType>& EpaPacket::getMatrix()
    { return *itsMatrix; };
  inline int EpaPacket::getSize(ParameterSet& ps)
    // This was:
    // { return EpaHeader::getSize() + ps.getInt32("Input.NSubbands") * \
    // ps.getInt32("Input.NPolarisations") * sizeof(RSPDataType); };
    // but now the number of subbands to generate is:
    // ps.getInt32("Input.SzEPApayload") / ( 2 * sizeof(RSPDataType))
    { return EpaHeader::getSize() + sizeof(RSPDataType) * ps.getInt32("Input.NPolarisations") * \
	(ps.getInt32("Input.SzEPApayload") / ( 2 * sizeof(RSPDataType))); }; //this is the number

  inline int EthernetFrame::getNoPacketsInFrame()
    { return itsEpaPackets.size(); };
  inline EpaPacket& EthernetFrame::getEpaPacket(int index)
    { return *(itsEpaPackets[index]); };
  inline char* EthernetFrame::getPayloadp()
    { return itsBufferp; };
  inline int EthernetFrame::getPayloadSize()
    { return itsBufferSize; };
  inline void EthernetFrame::reset()
    { memset(itsBufferp, 0, itsBufferSize); };
  inline int EthernetFrame::getSize(ParameterSet& ps)
    { return EpaPacket::getSize(ps) * ps.getInt32("Input.NPacketsInFrame"); };
  

} // namespace LOFAR

#endif
