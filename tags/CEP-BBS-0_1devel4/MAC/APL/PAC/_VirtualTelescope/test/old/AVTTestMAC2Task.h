//#  AVTTestMAC2Task.h: Automatic test of the RegisterAccess application
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

#ifndef AVTTestMAC2Task_H
#define AVTTestMAC2Task_H

//# Includes
//# Common Includes
#include <Suite/test.h>

//# GCF Includes
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Port.h>
#include <GCF/PAL/GCF_ExtPropertySet.h>
#include <boost/shared_ptr.hpp>
#include <vector>

#include "AVTTestAnswer.h"

// forward declaration

namespace LOFAR
{
namespace AVT
{
class GCF::TM::GCFEvent;
  
  class AVTTestMAC2Task : public GCF::TM::GCFTask, public Test
  {
    public:
      // as of GCF_Fsm.h version 1.4, the GCFFsm::State typedef is protected.
      // The workaround is simple yet highly undesirable: copy & paste
      typedef GCF::TM::GCFEvent::TResult (AVTTestMAC2Task::*State)(GCF::TM::GCFEvent& event, GCF::TM::GCFPortInterface& port); // ptr to state handler type

      AVTTestMAC2Task();
      virtual ~AVTTestMAC2Task();
      virtual void run();
  
    protected:
      // protected copy constructor
      AVTTestMAC2Task(const AVTTestMAC2Task&);
      // protected assignment operator
      AVTTestMAC2Task& operator=(const AVTTestMAC2Task&);
      
    private: 
      GCF::TM::GCFEvent::TResult initial(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test_3_2_4_1(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test_3_2_4_2(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test_3_2_4_3(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test_3_2_4_4(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test_3_2_4_5(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test_3_2_4_6(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test_3_2_4_7(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test_3_2_4_8(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test_3_2_5_1(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test_3_2_5_2(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test_3_2_5_3(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test_3_2_5_4(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test_3_2_5_5(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test_3_2_6_1(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test_3_2_7_1(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test_3_2_7_2(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test_3_2_7_3(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult finished(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      
      static string m_taskName;
      
      AVTTestAnswer                       m_answer;
      GCF::TM::GCFPort                             m_timerPort;
      
      GCF::PAL::GCFExtPropertySet     m_propertysetLDS;
      GCF::PAL::GCFExtPropertySet     m_propertysetBoard1Maintenance;
      GCF::PAL::GCFExtPropertySet     m_propertysetAP1RCU1Maintenance;
      GCF::PAL::GCFExtPropertySet     m_propertysetAP1RCU2Maintenance;
      GCF::PAL::GCFExtPropertySet     m_propertysetAP1RCU1;
      GCF::PAL::GCFExtPropertySet     m_propertysetAP2RCU1;
      GCF::PAL::GCFExtPropertySet     m_propertysetAP3RCU1;
      GCF::PAL::GCFExtPropertySet     m_propertysetVT1;
      GCF::PAL::GCFExtPropertySet     m_propertysetVT2;
      GCF::PAL::GCFExtPropertySet     m_propertysetVT3;
      int                   m_maintenanceChangedCounter;
      int                   m_suspendedCounter;
      int 									m_propsetLoadedCounter;
		  int 									m_nrOfPropsets;
      
  };  
};
};

#endif
