//#  MACScheduleTask.h: 
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

#ifndef MACScheduleTask_H
#define MACScheduleTask_H

//# Includes
//# Common Includes

//# GCF Includes
#include <GCF/TM/GCF_Task.h>
#include <GCF/PAL/GCF_Property.h>
#include <GCF/PAL/GCF_ExtPropertySet.h>

#include "MACScheduleAnswer.h"

// forward declaration
namespace LOFAR
{
namespace GSO
{
class GCF::TM::GCFEvent;

  class MACScheduleTask : public GCF::TM::GCFTask
  {
    public:
      MACScheduleTask(const string& schedulez, const string& updateschedule, const string& cancelschedule);
      virtual ~MACScheduleTask();
  
    protected:
      // protected copy constructor
      MACScheduleTask(const MACScheduleTask&);
      // protected assignment operator
      MACScheduleTask& operator=(const MACScheduleTask&);
      
    private: 
      GCF::TM::GCFEvent::TResult initial(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult propertiesLoaded(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      
      static string m_taskName;
      
      MACScheduleAnswer           m_answer;
      GCF::PAL::GCFExtPropertySet m_extPropsetMacScheduler;
      string                      m_scheduleNodeId;
      string                      m_updateScheduleNodeId;
      string                      m_cancelScheduleNodeId;
  };
};
};

#endif
