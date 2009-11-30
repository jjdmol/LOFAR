//# MemConnectionSet.cc: Set of Memory connections
//#
//# Copyright (c) 2007
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

#include <MWCommon/MemConnectionSet.h>
#include <MWCommon/MWError.h>
#include <Common/LofarLogger.h>


namespace LOFAR { namespace CEP {

  MemConnectionSet::MemConnectionSet()
  {}

  MemConnectionSet::~MemConnectionSet()
  {}

  MWConnectionSet::ShPtr
  MemConnectionSet::clone (const std::vector<int>& inx) const
  {
    int nrconn = size();
    MemConnectionSet* set = new MemConnectionSet();
    MWConnectionSet::ShPtr mwset(set);
    for (std::vector<int>::const_iterator it=inx.begin();
         it!=inx.end();
         ++it) {
      int i = *it;
      ASSERT (i>=0 && i<nrconn);
      set->itsConns.push_back (itsConns[i]);
    }
    return mwset;
  }

  int MemConnectionSet::addConnection (const WorkerProxy::ShPtr& worker)
  {
    int seqnr = itsConns.size();
    itsConns.push_back (MemConnection::ShPtr (new MemConnection(worker)));
    return seqnr;
  }

  int MemConnectionSet::size() const
  {
    return itsConns.size();
  }

  int MemConnectionSet::getReadyConnection()
  {
    return -1;
  }

  void MemConnectionSet::read (int seqnr, LOFAR::BlobString& buf)
  {
    itsConns[seqnr]->read (buf);
  }

  void MemConnectionSet::write (int seqnr, const LOFAR::BlobString& buf)
  {
    itsConns[seqnr]->write (buf);
  }

  void MemConnectionSet::writeAll (const LOFAR::BlobString& buf)
  {
    for (unsigned i=0; i<itsConns.size(); ++i) {
      itsConns[i]->write (buf);
    }
  }

}} // end namespaces
