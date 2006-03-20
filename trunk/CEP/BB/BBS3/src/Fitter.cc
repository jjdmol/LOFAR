//# Fitter.cc: Hold one or more fitter objects.
//#
//# Copyright (C) 2006
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

#include <BBS3/Fitter.h>
#include <Common/LofarLogger.h>
#include <casa/Containers/Record.h>
#include <casa/IO/AipsIO.h>
#include <casa/IO/MemoryIO.h>

#if defined _OPENMP
# include <omp.h>
#endif

using namespace std;
using namespace casa;

namespace LOFAR {

  void Fitter::set (int nfitter, int nunknown)
  {
    itsFitters.resize (nfitter);
    for (vector<LSQFit>::iterator iter = itsFitters.begin();
	 iter != itsFitters.end();
	 ++iter) {
      iter->set (nunknown);
    }
  }

  void Fitter::merge (const Fitter& that)
  {
    ASSERT (itsFitters.size() == that.itsFitters.size());
#pragma omp parallel
    {
#pragma omp for schedule(dynamic)
      for (uint i=0; i<itsFitters.size(); i++) {
	itsFitters[i].merge (that.itsFitters[i]);
      }
    }
  }

  void Fitter::marshall (void* buffer, int bufferSize) const
  {
    // Store the fitter in a few steps:
    // - convert to a Record
    // Make a non-expandable buffer and use it in AipsIO.
    casa::MemoryIO buf(buffer, bufferSize, ByteIO::Update);
    casa::AipsIO aio(&buf);
    aio.putstart ("Fitter", 1);
    aio << itsFitters.size();
    casa::Record rec;
    casa::String str;
    for (vector<LSQFit>::const_iterator iter = itsFitters.begin();
	 iter != itsFitters.end();
	 ++iter) {
      // Convert each fitter to a Record and serialize it into the buffer.
      ASSERT (iter->toRecord (str, rec));
      aio << rec;
    }
    aio.putend();
  }

  void Fitter::demarshall (const void* buffer, int bufferSize)
  {
    casa::MemoryIO buf(buffer, bufferSize);
    casa::AipsIO aio(&buf);
    aio.getstart ("Fitter");
    uint nfitter;
    aio >> nfitter;
    set (nfitter, 0);
    casa::Record rec;
    casa::String str;
    for (vector<LSQFit>::iterator iter = itsFitters.begin();
	 iter != itsFitters.end();
	 ++iter) {
      aio >> rec;
      ASSERT (iter->fromRecord (str, rec));
    }
    aio.getend();
  }

} // namespace LOFAR
