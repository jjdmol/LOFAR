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

//# GCF Includes
#include <GCF/GCF_Task.h>
#include <GCF/GCF_Port.h>
#include <GCF/GCF_Property.h>
#include <boost/shared_ptr.hpp>
#include <vector>

#include "AVTTest.h"
#include "AVTTestAnswer.h"

typedef struct
{
  GCFFsm::State target;
  const char*   targetName;
} TTranTarget;

#define ADDTRANTARGET(_targetvector_,_target_) \
  { \
    TTranTarget tt={static_cast<State>(&_target_), #_target_}; \
    _targetvector_.push_back(tt); \
  }
  
#define TESTTRAN(_trantarget_) \
  { \
    tran(static_cast<State>(_trantarget_->target), __func__, _trantarget_->targetName); \
  }

#define NEXTTEST(_iterator_) \
  { \
    printf("press enter ..."); \
    getchar(); \
    std::vector<TTranTarget>::iterator tempIt = _iterator_; \
    _iterator_++; \
    TESTTRAN(tempIt); \
  }
  
// forward declaration
class GCFEvent;
namespace AVT
{
  
  class AVTTestMAC2Task : public GCFTask
  {
    public:
      AVTTestMAC2Task(AVTTest<AVTTestMAC2Task>& tester);
      virtual ~AVTTestMAC2Task();
  
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
      AVTTest<AVTTestMAC2Task>&           m_tester;
      AVTTestAnswer                       m_answer;
      GCFPort                             m_timerPort;
      
      GCFProperty     m_propertyLDScommand;
      GCFProperty     m_propertyLDSstatus;
      GCFProperty     m_propBoard1MaintenanceStatus;
      GCFProperty     m_propAP1RCU1MaintenanceStatus;
      GCFProperty     m_propAP1RCU2MaintenanceStatus;
      GCFProperty     m_propSRG1Filter;
      GCFProperty     m_propAP1RCU1Status;
      GCFProperty     m_propAP2RCU1Status;
      GCFProperty     m_propAP2RCU2Status;
      GCFProperty     m_propVT1Command;
      GCFProperty     m_propVT1Status;
      GCFProperty     m_propVT2Command;
      GCFProperty     m_propVT2Status;
      int             m_maintenanceChangedCounter;
  };  
};

#endif
