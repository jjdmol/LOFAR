//#  ARATestTask.h: Automatic test of the RegisterAccess application
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

#ifndef ARATestTask_H
#define ARATestTask_H

//# Includes
//# Common Includes

//# GCF Includes
#include <GCF/GCF_Task.h>
#include <GCF/GCF_ExtPropertySet.h>
#include <boost/shared_ptr.hpp>

#include "ARATestAnswer.h"

// forward declaration
class GCFEvent;
namespace ARA
{
  class ARATest;
  
  class ARATestTask : public GCFTask
  {
    public:
      ARATestTask(ARATest& tester);
      virtual ~ARATestTask();
  
    protected:
      // protected copy constructor
      ARATestTask(const ARATestTask&);
      // protected assignment operator
      ARATestTask& operator=(const ARATestTask&);
      
    private: 
      bool isEnabled();
      GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test1(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test2(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test3(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test4(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test5(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test6(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test7(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test8(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test9(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test10(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult finished(GCFEvent& e, GCFPortInterface& p);
      
      
      static string m_taskName;
      static string m_RATestServerName;
      
      ARATest&        m_tester;
      ARATestAnswer   m_answer;
      GCFPort         m_RSPserver;
      
      int             m_test_passCounter;
      int             m_propsetLoadedCounter;
      
      GCFExtPropertySet m_extPropSetAP1;
      GCFExtPropertySet m_extPropSetAP1RCUmaintenance;
      GCFExtPropertySet m_extPropSetAP1RCUalert;
      GCFExtPropertySet m_extPropSetStationMaintenance;
      GCFExtPropertySet m_extPropSetLDS;
      
  };  
};

#endif
