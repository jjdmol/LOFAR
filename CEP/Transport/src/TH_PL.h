//# TH_PL.h: TransportHolder using the Persistency Layer
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

#ifndef TRANSPORT_TH_PL_H
#define TRANSPORT_TH_PL_H

#include <Transport/TransportHolder.h>
#include <Transport/DH_PL.h>
#include <PL/PersistenceBroker.h>		// for PersistenceBroker
#include <Common/LofarTypes.h>                  // for int64


namespace LOFAR
{

class TH_PL: public TransportHolder
{
public:
  explicit TH_PL (const string& tableName = "defaultTable");
  virtual ~TH_PL ();

  virtual TH_PL* make () const;

  virtual bool init();

  virtual bool recvBlocking (void* buf, int nbytes, int tag);
  virtual bool recvNonBlocking (void* buf, int nbytes, int tag);

  virtual bool waitForReceived (void* buf, int nbytes, int tag);

  virtual bool sendBlocking (void* buf, int nbytes, int tag);
  virtual bool sendNonBlocking (void* buf, int nbytes, int tag);

  virtual bool waitForSent (void* buf, int nbytes, int tag);

  virtual string getType () const;

  // Get the type of BlobString needed for the DataHolder (which is a string).
  virtual BlobStringType blobStringType() const;

  virtual bool connectionPossible (int srcRank, int dstRank) const;

  static TH_PL proto;
  
  static void finalize ();
  static void waitForBroadCast ();
  static void waitForBroadCast (unsigned long& aVar);
  static void sendBroadCast (unsigned long timeStamp);
  static int  getCurrentRank ();
  static int  getNumberOfNodes ();
  static void synchroniseAllProcesses ();

  // Specify the data source name and account for PL. Usually dbDSN =
  // <YourName> and userName="postgres".
  static void useDatabase (const string& dbDSN,
			   const string& userName="postgres");
  
protected:
   static PL::PersistenceBroker theirPersistenceBroker;
   static int theirInstanceCount;
private:
  // methods to manage the connection to the dbms
  void connectDatabase();
  void disconnectDatabase();

  /// Strings containing the name specs describing the ODBC connection.
  static string theirDSN;
  static string theirUserName;
  string        itsTableName;
    
  int64  itsWriteSeqNo;
  int64  itsReadSeqNo;
  DH_PL* itsDHPL;
};
 
} // end namespace

#endif
