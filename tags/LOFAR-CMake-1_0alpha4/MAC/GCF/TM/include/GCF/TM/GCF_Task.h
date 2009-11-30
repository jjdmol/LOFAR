//#  GCF_Task.h: handles all events for a task.
//#
//#  Copyright (C) 2002-2003
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

#ifndef GCF_TASK_H
#define GCF_TASK_H

#include <Common/lofar_list.h>
#include <Common/lofar_map.h>
#include <GCF/TM/GCF_Fsm.h>

namespace LOFAR {
  namespace GCF {
    namespace TM {

// forward declaration

/**
 * This is the base class for all tasks in an application. Different 
 * specialisations of this class results in a number of concurrent finite state 
 * machines with own ports to other tasks (in other processes). 
 * Note: This is not a representation of a 'thread' related to the 
 * multithreading concept.
 */

class GCFTask : public GCFFsm
{
public:  
	// constuctors
    virtual ~GCFTask() { }

    // "starts" this task; see code example from run method
    void start () { initFsm(); }

	// Gives task a final change to end their actions
	void quit () { quitFsm();	}
    
    // Get the name of the task.
    const string& getName () const {return _name;}
    // Set the name of the task.
    void setName (string& name) {_name = name;}

protected:
    explicit GCFTask (State initial, const string& name) :
		GCFFsm(initial), _name(name) { }

private:
    // Is private to avoid initialising a task without giving an inital state and the task name
    GCFTask();
  
	// --- DATA MEMBERS ---
    // the task name
    string _name;
};  

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

#endif
