//# SocketConnectionSet.cc: Set of socket connections
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <lofar_config.h>

#include <MWCommon/SocketConnectionSet.h>
#include <MWCommon/MWError.h>
#include <Common/LofarLogger.h>


namespace LOFAR { namespace CEP {

  SocketConnectionSet::SocketConnectionSet (const std::string& port)
    : itsListener (port)
  {}

  SocketConnectionSet::SocketConnectionSet (const SocketListener& listener)
    : itsListener (listener)
  {}

  SocketConnectionSet::~SocketConnectionSet()
  {}

  MWConnectionSet::ShPtr
  SocketConnectionSet::clone (const std::vector<int>& inx) const
  {
    int nrconn = size();
    SocketConnectionSet* set = new SocketConnectionSet(itsListener);
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

  void SocketConnectionSet::addConnections (int nr)
  {
    itsConns.reserve (itsConns.size() + nr);
    for (int i=0; i<nr; ++i) {
      itsConns.push_back (itsListener.accept());
    }
  }

  int SocketConnectionSet::size() const
  {
    return itsConns.size();
  }

  int SocketConnectionSet::getReadyConnection()
  {
    return -1;
  }

  void SocketConnectionSet::read (int seqnr, LOFAR::BlobString& buf)
  {
    itsConns[seqnr]->read (buf);
  }

  void SocketConnectionSet::write (int seqnr, const LOFAR::BlobString& buf)
  {
    itsConns[seqnr]->write (buf);
  }

  void SocketConnectionSet::writeAll (const LOFAR::BlobString& buf)
  {
    for (unsigned i=0; i<itsConns.size(); ++i) {
      itsConns[i]->write (buf);
    }
  }

}} // end namespaces
