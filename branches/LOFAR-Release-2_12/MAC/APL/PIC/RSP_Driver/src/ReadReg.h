//#  -*- mode: c++ -*-
//#
//#  ReadReg.h: Write a register on a RSP board. Read status to verify correct writing.
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

#ifndef READREG_H_
#define READREG_H_

#include <Common/LofarTypes.h>
#include <APL/RSP_Protocol/MEPHeader.h>

#include "SyncAction.h"

namespace LOFAR {
  namespace RSP {

    class ReadReg : public SyncAction
    {
    public:
      /**
       * Constructors for a ReadReg object.
       */
      ReadReg(GCFPortInterface& board_port, int board_id,
	      uint16 dstid, uint8 pid, uint8 regid, uint16 size,
	      uint16 offset = 0);
	  
      /* Destructor for ReadReg. */
      virtual ~ReadReg();

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
      uint16 m_dstid;
      uint8  m_pid;
      uint8  m_regid;
      uint16 m_size;
      uint16 m_offset;

      EPA_Protocol::MEPHeader m_hdr;
    };
  };
};
     
#endif /* READREG_H_ */
