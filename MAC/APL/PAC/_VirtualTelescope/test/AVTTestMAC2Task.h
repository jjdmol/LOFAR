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

typedef struct
{
  GCFFsm::State target;
  const char*   targetName;
  const char*   testNum;
  const char*   description;
} TTranTarget;

// forward declaration
class GCFEvent;
namespace AVT
{
  
  class AVTTestMAC2Task : public GCFTask, public Test
  {
    public:
      AVTTestMAC2Task();
      virtual ~AVTTestMAC2Task();
      virtual void run();
  
    protected:
      // protected copy constructor
      AVTTestMAC2Task(const AVTTestMAC2Task&);
      // protected assignment operator
      AVTTestMAC2Task& operator=(const AVTTestMAC2Task&);
      
    private: 
      GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test_3_2_4_1(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test_3_2_4_2(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test_3_2_4_3(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test_3_2_4_4(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test_3_2_4_5(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test_3_2_4_6(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test_3_2_4_7(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test_3_2_4_8(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test_3_2_5_1(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test_3_2_5_2(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test_3_2_5_3(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test_3_2_5_4(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test_3_2_5_5(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test_3_2_6_1(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test_3_2_7_1(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test_3_2_7_2(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test_3_2_7_3(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult finished(GCFEvent& e, GCFPortInterface& p);
      
      static string m_taskName;
      
      std::vector<TTranTarget>            m_testSequence;
      std::vector<TTranTarget>::iterator  m_testSequenceIt;
      AVTTestAnswer                       m_answer;
      GCFPort                             m_timerPort;
      
      GCFExtPropertySet     m_propertysetLDS;
      GCFExtPropertySet     m_propertysetBoard1Maintenance;
      GCFExtPropertySet     m_propertysetAP1RCU1Maintenance;
      GCFExtPropertySet     m_propertysetAP1RCU2Maintenance;
      GCFExtPropertySet     m_propertysetAP1RCU1;
      GCFExtPropertySet     m_propertysetAP2RCU1;
      GCFExtPropertySet     m_propertysetAP3RCU1;
      GCFExtPropertySet     m_propertysetVT1;
      GCFExtPropertySet     m_propertysetVT2;
      GCFExtPropertySet     m_propertysetVT3;
      int                   m_maintenanceChangedCounter;
      int                   m_suspendedCounter;
  };  
};

#endif
