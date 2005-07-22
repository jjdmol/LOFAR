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
#include <TFC_Generator/StationData.h>

namespace LOFAR
{
  EpaPacket::EpaPacket(char* bufferSpace, int bufferSize, ParameterSet ps)
    : itsBufferp(bufferSpace)
  {
    int noSubBands = ps.getInt("noSubBands");
    itsBufferSize = sizeof(EpaHeader) + noSubBands * 2 * sizeof(complex<int16>);

    ASSERSTR(bufferSize == itsBufferSize, "EpaPacket received the wrong amount of buffer data");
    
    // create the RectMatrix
    vector<DimDef> vdd;
    vdd.push_back(DimDef("subband", noSubBands));
    vdd.push_back(DimDef("polarisatie", 2));
    itsMatrix = new RectMatrix<complex<int16> >(vdd);
    itsMatrix->setBuffer(itsBufferp + sizeof(EpaHeader), 2*noSubBands);
  };

  EpaPacket::~EpaPacket()
  {
    delete itsMatrix;
  };

  EthernetFrame::EthernetFrame(ParameterSet ps) 
  {
    int noEpaP = ps.getInt("noEpaPackets");
    int noSubBands = ps.getInt("noSubBands");
    int epaSize = sizeof(EpaHeader) + noSubBands * 2 * sizeof(complex<int16>);
    itsBufferp = new char[noEpap * epaSize];
    for (int epap = 0; epap < noEpaP; epap++) {
      itsEpaPackets.append(new EpaPacket(itsBufferp[epap * epaSize], epaSize, ps));
    }
  };

  EthernetFrame::~EthernetFrame()
  {
    vector<EpaPacket*>::iterator epap = itsEpaPackets.begin();
    for (; epap != itsEpaPackets.end(); epap++) {
      delete epap->second;
    }
    delete [] itsBufferp;
  };
  
} // namespace LOFAR

#endif
