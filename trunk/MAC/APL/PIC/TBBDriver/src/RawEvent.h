//#  -*- mode: c++ -*-
//#
//#  RawEvent.h: dispatch raw EPA events as GCF events.
//#
//#  Copyright (C) 2002-2004
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

#ifndef RAWEVENT_H_
#define RAWEVENT_H_

#include <GCF/TM/GCF_Control.h>

namespace LOFAR {
  namespace TP_Protocol{
		
		// TP Command Opcode's
		// Data Recording
		static const uint32 TPALLOC   = 0x00000100;  // OUT allocate buffer space to a certain input channel
		static const uint32 TPFREE    = 0x00000101;  // OUT free buffer settings and disable input channel 
		static const uint32 TPRECORD  = 0x00000102;  // OUT record channel
		static const uint32 TPSTOP    = 0x00000103;  // OUT freeze channel
		// Triggering
		static const uint32 TPTRIGGER = 0x00000200;  // IN trigger detected
		static const uint32 TPTRIGCLR = 0x00000201;  // OUT clear trigger flag
		// Data reading
		static const uint32 TPREAD    = 0x00000300;  // OUT send recorded data to CEP
		static const uint32 TPUDP     = 0x00000301;  // OUT configure UDP and IP header
		// Board information
		static const uint32 TPVERSION = 0x00000701;  // IN/OUT returns board version
		static const uint32 TPSIZE    = 0x00000702;  // IN/OUT returns TBB memory size
		// Board status
		static const uint32 TPERROR   = 0x00000703;  // IN error on TBB board
		// Board control
		static const uint32 TPCLEAR   = 0x00000710;  // OUT clear registers
		static const uint32 TPRESET   = 0x00000711;  // OUT reset to facory image
		static const uint32 TPCONFIG  = 0x00000712;  // OUT reconfigure image
		// Remote system update
		static const uint32 TPERASEF  = 0x00000720;  // OUT erase flash memory
		static const uint32 TPREADF   = 0x00000721;  // IN/OUT read flash memory
		static const uint32 TPWRITEF  = 0x00000722;  // OUT write flash memory
		// DDR2 acces
		static const uint32 TPREADW   = 0x00000730;  // IN/OUT read 64bit word from mp
		static const uint32 TPWRITEW  = 0x00000731;  // OUT write 64bit wort to mp
		// Direct register acces 
		static const uint32 TPREADR   = 0x00000740;  // IN/OUT read register(direct access), for debug purpose
		static const uint32 TPWRITER  = 0x00000741;  // OUT write register(direct access), for debug purpose
		
		class RawEvent
		{
			public:
				static LOFAR::GCF::TM::GCFEvent::TResult dispatch(LOFAR::GCF::TM::GCFTask& task,
																				LOFAR::GCF::TM::GCFPortInterface& port);
		};
  };
};
#endif /* RAWEVENT_H_ */
