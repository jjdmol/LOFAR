//  WH_Src.cc: WorkHolder class using DH_FixedSize() objects and
//                 measuring performance
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//
//////////////////////////////////////////////////////////////////////

#include "3BlockPerf/WH_Src.h"

namespace LOFAR {

WH_Src::WH_Src (const string& name, 
                unsigned int size,    // size of the packet in bytes
		unsigned int packetsPerMeasurement,
	        unsigned int flopsPerByte)
		: WorkHolder    (0, 1, name),
		  itsSize (size),
		  itsIteration(0),
		  itsPacketsPerMeasurement(packetsPerMeasurement),
		  itsFlopsPerByte(flopsPerByte)
{
  DH_FixedSize* dh = new DH_FixedSize("out_of_WH_Src", itsSize);
  getDataManager().addOutDataHolder(0, dh);
  watch = new StopWatch();
  itsDataSize = dh->getNoValues() * sizeof(DH_FixedSize::valueType);
}


WH_Src::~WH_Src()
{
}

WorkHolder* WH_Src::make(const string& name)
{
  return new WH_Src(name,
		    itsSize,
		    itsPacketsPerMeasurement,
		    itsFlopsPerByte
                    );
}

void WH_Src::process()
{
  if (itsIteration == 0) {
    watch->stop();
    double time = watch->elapsed();
    double speed = itsPacketsPerMeasurement * itsDataSize/time;
    cout << speed << " B/s "
         << speed * itsFlopsPerByte << " flop/s ("
	 << itsPacketsPerMeasurement << " packets of "
	 << itsDataSize << " (effective) Bytes in "
	 << time << " seconds) at "
	 << itsFlopsPerByte << " flops per Byte)"
	 << endl;
    itsIteration = 0;
    // itsPacketsPerMeasurement *= 2; // this was used to see how many packets would give an accurate measurement
    itsIteration = itsPacketsPerMeasurement;
    watch->start();
  }
  itsIteration--;
  // wait until last DH was completely sent
  //getDataManager().getOutHolder(0)->getTransporter().getTransportHolder()->waitForSent(NULL, 0, 0);

  memset(((DH_FixedSize*)getDataManager().getOutHolder(0))->getPtr2Data(), 0, itsDataSize);
}

}
