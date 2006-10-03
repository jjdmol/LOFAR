//# MSDesc.cc: Program write the description of an MS
//#
//# Copyright (C) 2005
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <lofar_config.h>
#include <MS/MSDesc.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobOBufStream.h>
#include <Blob/BlobArray.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <casa/Quanta/MVTime.h>

#include <iostream>

using namespace casa;
using namespace std;

namespace LOFAR {

BlobOStream& operator<< (BlobOStream& bos, const MSDesc& msd)
{
  bos.putStart("MSDesc", 1);
  bos << msd.msPath << msd.msName << msd.npart;
  bos << msd.ra << msd.dec << msd.startTime << msd.endTime << msd.corrTypes;
  bos << msd.nchan << msd.startFreq << msd.endFreq;
  bos << msd.times << msd.exposures;
  bos << msd.ant1 << msd.ant2;
  bos << msd.antNames;
  bos << msd.antPos;
  bos << msd.arrayPos;
  bos.putEnd();
  return bos;
}

BlobIStream& operator>> (BlobIStream& bis, MSDesc& msd)
{
  int version = bis.getStart("MSDesc");
  ASSERTSTR (version==1, "Unknown version in MSDesc blob");
  bis >> msd.msPath >> msd.msName >> msd.npart;
  bis >> msd.ra >> msd.dec >> msd.startTime >> msd.endTime >> msd.corrTypes;
  bis >> msd.nchan >> msd.startFreq >> msd.endFreq;
  bis >> msd.times >> msd.exposures;
  bis >> msd.ant1 >> msd.ant2;
  bis >> msd.antNames;
  bis >> msd.antPos;
  bis >> msd.arrayPos;
  bis.getEnd();
  return bis;
}

ostream& operator<< (ostream& os, const MSDesc& msd)
{
  os << "MS: " << msd.msPath << ' ' << msd.msName
     << "   npart=" << msd.npart << endl;
  os << " ncorr=" << msd.corrTypes.size() << "  nstat=" << msd.antNames.size()
     << "  nbl=" << msd.ant1.size() << endl;
  int nband = msd.startFreq.size();
  os << " freq: start=" << msd.startFreq[0]/1e6 << " Mhz  end="
     << msd.endFreq[nband-1]/1e6 << " Mhz  step="
     << (msd.endFreq[0]-msd.startFreq[0])/msd.nchan[0]/1e3 << " Khz" << endl;
  os << "       nband=" << nband
     << "    1st band: nchan=" << msd.nchan[0] << endl;
  int ntime = msd.times.size();
  os << " time: start="
       << MVTime::Format(MVTime::DMY) << MVTime(Quantity(msd.startTime,"s"))
       << "  end="
       << MVTime::Format(MVTime::DMY) << MVTime(Quantity(msd.endTime,"s"))
       << "  step=" << (msd.endTime-msd.startTime)/ntime << " sec" << endl;
  os << "       ntime=" << ntime << endl;
  os << " stations:";
  for (uint i=0; i<msd.antNames.size(); ++i) {
    os << ' ' << msd.antNames[i];
  }
  os << endl;
  return os;
}

void MSDesc::writeDesc (ostream& os) const
{
  os << "npart=" << npart << endl;
  os << "subsetMSPath=" << msPath << endl;
  os << "corrTypes=" << corrTypes << endl;
  os << "stations=" << antNames << endl;
  int nband = startFreq.size();
  os << "freq.nband=" << nband << endl;
  os << "freq.start=" << startFreq[0]/1e6 << " Mhz" << endl;
  os << "freq.end=" << endFreq[0]/1e6 << " Mhz" << endl;
  os << "freq.step=" << (endFreq[0]-startFreq[0]) / nchan[0] / 1e3
     << " Khz" << endl;
  os << "freq.nchan=" << nchan[0] << endl;
  int ntime = times.size();
  os << "time.start="
     << MVTime::Format(MVTime::DMY) << MVTime(Quantity(startTime,"s")) << endl;
  os << "time.end=  "
     << MVTime::Format(MVTime::DMY) << MVTime(Quantity(endTime,"s")) << endl;
  os << "time.step=" << (endTime-startTime) / ntime << " #sec" << endl;
  os << "time.nsteps=" << ntime << endl;
}

}  //# namespace LOFAR
