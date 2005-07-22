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
#include <lofar_vector.h>
#include <TFC_Interface/RectMatrix.h>

namespace LOFAR
{
  struct EpaHeader
  {
    int16 protocol;
    int32 stationId;
    int32 seqId;
    int32 blockId;
  }

  class EpaPacket
  {
  public:
    ~EpaPacket();

    EpaHeader& getHeader();
    void setHeader(EpaHeader& newHeader);
    RectMatrix< complex<int16> >& getMatrix();
  protected:
    friend class EthernetFrame;
    // a packet should be constructed only by a EthernetFrame
    EpaPacket(char* bufferSpace, int bufferSize, ParameterSet ps);

    char* itsBufferp;
    RectMatrix* itsMatrix;
  }

  class EthernetFrame
  {
  public:
    EthernetFrame(itsPS);
    ~EthernetFrame();
    EpaPacket& getEpaPacket(int index);
    char* getPayloadp();
    int getPayloadSize();
    void reset();
  protected:
    vector<EpaPacket*> itsEpaPackets;
    char* itsBufferp;
    int itsBufferSize;
  };

  inline EpaHeader& EpaPacket::getHeader()
    { return *((EpaHeader*)itsBufferp); };
  inline void EpaPacket::setHeader(EpaHeader& newHeader)
    { memcpy(itsBufferp, &newHeader, sizeof(EpaHeader));};
  inline RectMatrix< complex<int16> >& EpaPacket::getMatrix()
    { return *itsMatrix; };

  inline EpaPacket& EthernetFrame::getEpaPacket(int index)
    { return *(itsEpaPackets[index]); };
  inline char* EthernetFrame::getPayloadp()
    { return itsBufferp; };
  inline int EthernetFrame::getPayloadSize()
    { return itsBufferSize; };
  inline void EthernetFrame::reset()
    { memset(itsBufferp, 0, itsBufferSize);};
  }   

} // namespace LOFAR

#endif
