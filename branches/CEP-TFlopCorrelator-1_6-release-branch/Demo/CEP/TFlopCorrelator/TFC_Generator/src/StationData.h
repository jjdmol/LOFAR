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

  // \addtogroup TFC_Generator
  // @{
  
  // Header is the class that represents the header data of the epa packet
  // it contains the following fields:
  //   protocol(int32)
  //   stationId(int32)
  //   sequenceId(int32)
  //   blockId(int32)
  class Header
  {
  public:
    ~Header();

    int32 getProtocol();
    void setProtocol(int32 newProt);
    int32 getStationId();
    void setStationId(int32 newSid);
    int32 getSeqId();
    void setSeqId(int32 newSid);
    int32 getBlockId();
    void setBlockId(int32 newBid);
    static int getSize();
  protected:
    Header(Header&);
    int32 getInt32(char* memLoc);
    void setInt32(char* memLoc, const char* newValueP);
    friend class EpaPacket;
    friend class EthernetFrame;
    Header(char* bufferSpace);
    char* itsBufferp;

    static const int theirHeaderSize = 16;
    static const int theirStatIDOffset = 4;
    static const int theirSeqIDOffset = 8;
    static const int theirBlockIDOffset = 12;
  };

  // EpaPacket represents 1 packet in an ethernet frame
  // it contains:
  //    1 Header 
  //    a matrix containing:
  //       NoSubbands * noPolarisations(=2) * complex<int16>
  class EpaPacket
  {
  public:
    ~EpaPacket();

    RectMatrix<RSPDataType>& getMatrix();
    static int getSize(ParameterSet& ps);
  protected:
    EpaPacket(EpaPacket&);
    friend class EthernetFrame;
    // a packet should be constructed only by a EthernetFrame
    EpaPacket(char* bufferSpace, int bufferSize, ParameterSet ps);

    char* itsBufferp;
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
    Header& getHeader();

  protected:
    EthernetFrame(EthernetFrame&);
    vector<EpaPacket*> itsEpaPackets;
    char* itsBufferp;
    int itsBufferSize;
    bool itsIsMemoryMine;
    Header itsFrameHeader;
  };

  // @}



  //inline functions

  inline int Header::getSize()
  { return theirHeaderSize; };
  inline int32 Header::getStationId()
  { return getInt32(&itsBufferp[theirStatIDOffset]); };
  inline void Header::setStationId(int32 sid)
  { setInt32(&itsBufferp[theirStatIDOffset], (const char*)&sid); };
  inline int32 Header::getSeqId()
  { return getInt32(&itsBufferp[theirSeqIDOffset]); };
  inline void Header::setSeqId(int32 sid)
  { setInt32(&itsBufferp[theirSeqIDOffset], (const char*)&sid); };
  inline int32 Header::getBlockId()
  { return getInt32(&itsBufferp[theirBlockIDOffset]); };
  inline void Header::setBlockId(int32 bid)
  { setInt32(&itsBufferp[theirBlockIDOffset], (const char*)&bid); };

  inline RectMatrix<RSPDataType>& EpaPacket::getMatrix()
    { return *itsMatrix; };
  inline int EpaPacket::getSize(ParameterSet& ps)
    { return sizeof(RSPDataType) * ps.getInt32("Data.NPolarisations") * \
	(ps.getInt32("Input.SzEPApayload") / ( 2 * sizeof(RSPDataType))); }; //this is the number

  inline int EthernetFrame::getNoPacketsInFrame()
    { return itsEpaPackets.size(); };
  inline Header& EthernetFrame::getHeader()
    { return itsFrameHeader; };
  inline EpaPacket& EthernetFrame::getEpaPacket(int index)
    { return *(itsEpaPackets[index]); };
  inline char* EthernetFrame::getPayloadp()
    { return itsBufferp; };
  inline int EthernetFrame::getPayloadSize()
    { return itsBufferSize; };
  inline void EthernetFrame::reset()
    { memset(itsBufferp, 0, itsBufferSize); };
  inline int EthernetFrame::getSize(ParameterSet& ps)
    { return Header::getSize() + EpaPacket::getSize(ps) * ps.getInt32("Input.NPacketsInFrame"); };

} // namespace LOFAR

#endif
