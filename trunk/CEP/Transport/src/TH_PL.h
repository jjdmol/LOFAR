//# TH_PL.h: An example simulator
//#
//# Copyright (C) 2000, 2002
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

#ifndef LIBTRANSPORT_TH_PL_H
#define LIBTRANSPORT_TH_PL_H

#include <TransportHolder.h>
#include <DH_PL.h>
#include <PL/PersistenceBroker.h>		// for PersistenceBroker
namespace LOFAR
{

class TH_PL: public TransportHolder
{
public:
  TH_PL ();
  virtual ~TH_PL ();

  virtual TH_PL* make () const;

  virtual bool recvBlocking (void* buf, int nbytes, int source, int tag);
  virtual bool sendBlocking (void* buf, int nbytes, int destination, int tag);

  virtual string getType () const;

  virtual bool connectionPossible (int srcRank, int dstRank) const;

  static TH_PL proto;
  
  static void init (int argc, const char *argv[]);
  static void finalize ();
  static void waitForBroadCast ();
  static void waitForBroadCast (unsigned long& aVar);
  static void sendBroadCast (unsigned long timeStamp);
  static int  getCurrentRank ();
  static int  getNumberOfNodes ();
  static void synchroniseAllProcesses ();

protected:
   static PL::PersistenceBroker theirPersistenceBroker;
   static int theirInstanceCount;
private:
  // methods to manage the connection to the dbms
  void ConnectDatabase (void);
  void DisconnectDatabase (void);
  // Specify the data source name and account for PL. Usually dbDSN =
  // <YourName> and userName="postgres".
  static void UseDatabase (char * dbDSN, char * userName);
  
  /// Strings containing the name specs describing the ODBC connection.
    static string theirDSN;
    static string theirUserName;
    
    // The results from a query is returned as a collection of
    // DH_PL_MessageRecord. So we need an object for that purpose:
   Collection<PL::TPersistentObject<DH_PL::DataPacket> > Results;  

   long itsWriteSeqNo;
   long itsReadSeqNo;
};
 
}

#endif



