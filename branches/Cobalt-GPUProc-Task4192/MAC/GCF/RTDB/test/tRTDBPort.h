//  tRTDBPort.h: Definition of the DPservice task class.
//
//  Copyright (C) 2007
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id: tRTDBPort.h 13125 2009-04-19 12:32:55Z overeem $
//

#ifndef _RTDB_T_RTDBPORT_H_
#define _RTDB_T_RTDBPORT_H_

#include <GCF/TM/GCF_Control.h>
#include <GCF/RTDB/GCF_RTDBPort.h>

namespace LOFAR {
 namespace GCF {
  using TM::GCFTask;
  using TM::GCFPortInterface;
  using TM::GCFTimerPort;
  namespace RTDB {

class tWriter : public GCFTask
{
public:

	tWriter (const string& name);
	virtual ~tWriter();

	GCFEvent::TResult final	    (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult openPort  (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult writeTest (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult closeTest (GCFEvent& e, GCFPortInterface& p);

private:
	GCFRTDBPort*		itsRTDBPort;
	GCFTimerPort*		itsTimerPort;
};

class tReader : public GCFTask
{
public:

	tReader (const string& name);
	virtual ~tReader();

	GCFEvent::TResult final	    (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult openPort  (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult readTest  (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult closeTest (GCFEvent& e, GCFPortInterface& p);

private:
	GCFRTDBPort*		itsRTDBPort;
	GCFTimerPort*		itsTimerPort;
};

  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR

#endif
