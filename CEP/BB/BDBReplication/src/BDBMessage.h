//#  BDBMessage.h: contains a message that one bdbreplicated site can send to another
//#
//#  Copyright (C) 2005
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

#ifndef LOFAR_BDBREPLICATIONBDBMESSAGE_H
#define LOFAR_BDBREPLICATIONBDBMESSAGE_H

// \file BDBMessage.h
// A message that can be send from one replicated site to another

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Transport/TransportHolder.h>
#include <db_cxx.h>

namespace LOFAR 
{
  namespace BDBReplication 
  {

    // \addtogroup BDBReplication
    // @{

#define BDBTAG 85
    typedef int SyncReqType;

    // Description of class.
    class BDBMessage
    {
    public:
      BDBMessage(int mtype = CONNECT);
      ~BDBMessage();

      enum {
	NO_MESSAGE,
	CONNECT,
	LIBBDB,
	STARTUP_DONE,
	SYNC_REPLY,
        TO_BE_RECEIVED};

      int getType() { return itsType; };
      int getEnvId() { return itsEnvId; };
      void setEnvId(int envId) { itsEnvId = envId; };

      bool receive(TransportHolder* th, bool blocking = false);
      bool send(TransportHolder* th);

      // TODO: set and get functions
      // for LIBBDB
      Dbt& getRec();
      void setRec(const Dbt& rec);
      Dbt& getControl();
      void setControl(const Dbt& control);

      // for CONNECT
      int getPort();
      void setPort(int port);
      string getHostName();
      void setHostName(string hostName);

      // for SYNC_REPLY
      SyncReqType getSyncRequestNumber() { return itsSyncRequestNumber; };
      void setSyncRequestNumber(SyncReqType sr) { itsSyncRequestNumber = sr; };

    private:
      // Copying is not allowed
      BDBMessage(const BDBMessage& that);
      BDBMessage& operator=(const BDBMessage& that);

      //# Datamembers
      int itsType;
      int itsEnvId;
      
      // for LIBBDB
      Dbt itsRec;
      Dbt itsControl;
      char* itsCBuffer;
      char* itsRBuffer;

      // for CONNECT
      int itsPort;
      string itsHostName;

      // for SYNC_REPLY
      SyncReqType itsSyncRequestNumber;
      //      ALLOC_TRACER_ALIAS(BDBSite);
    };

    // @}

  } // namespace BDBReplication
} // namespace LOFAR

#endif
