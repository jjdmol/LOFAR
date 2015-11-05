//#  testTask.h: one_line_description
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
//#  $Id: $

#ifndef GCF_TM_TESTTASK_H
#define GCF_TM_TESTTASK_H

// \file 
// one_line_description

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Control.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  using MACIO::GCFEvent;
  using GCF::TM::GCFTimerPort;
  using GCF::TM::GCFTask;
  using GCF::TM::GCFPortInterface;
  namespace GCF {
	namespace TM {

// \addtogroup GCF
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.


// class_description
// ...
class testTask : public GCFTask
{
public:
	testTask (const string&	name, double	timerInterval);
	~testTask();

	GCFEvent::TResult	mainTask(GCFEvent&	event, GCFPortInterface&	port);

private:
	testTask();
	testTask(const testTask&	that);
	testTask& operator=(const testTask& that);

	//# --- Datamembers ---
	GCFTimerPort*	itsTimerPort;
	double			itsTimerInterval;
	uint32			itsStopTimer;
};

// @}
    } // namespace TM
  } // namespace GCF
} // namespace LOFAR

#endif
