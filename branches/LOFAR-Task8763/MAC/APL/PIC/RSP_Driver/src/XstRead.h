//#  -*- mode: c++ -*-
//#
//#  XstRead.h: Synchronize crosslet statistics with RSP hardware.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

#ifndef XSTREAD_H_
#define XSTREAD_H_

#include <Common/LofarTypes.h>
#include <APL/RSP_Protocol/MEPHeader.h>

#include "SyncAction.h"

namespace LOFAR {
  namespace RSP {

    class XstRead : public SyncAction
    {
    public:
      /**
       * Constructors for a XstRead object.
       */
      XstRead(GCFPortInterface& board_port, int board_id, int regid);
	  
      /* Destructor for XstRead. */
      virtual ~XstRead();

      /**
       * Send the write message.
       */
      virtual void sendrequest();

      /**
       * Send the read request.
       */
      virtual void sendrequest_status();

      /**
       * Handle the read result.
       */
      virtual GCFEvent::TResult handleack(GCFEvent& event, GCFPortInterface& port);

    private:
      int m_regid;
      EPA_Protocol::MEPHeader m_hdr;
    };
  };
};
     
#endif /* XSTREAD_H_ */
