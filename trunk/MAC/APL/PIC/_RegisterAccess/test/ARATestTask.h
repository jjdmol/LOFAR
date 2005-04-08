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
#include <GCF/TM/GCF_Task.h>
#include <GCF/PAL/GCF_ExtPropertySet.h>
#include <boost/shared_ptr.hpp>
#include <Suite/test.h>

#include "ARATestAnswer.h"

namespace LOFAR
{

// forward declaration
class GCF::TM::GCFEvent;

namespace ARA
{
  class ARATestTask : public GCF::TM::GCFTask, public Test
  {
    public:
      ARATestTask();
      virtual ~ARATestTask();
      void run();
  
    protected:
      // protected copy constructor
      ARATestTask(const ARATestTask&);
      // protected assignment operator
      ARATestTask& operator=(const ARATestTask&);
      
    private: 
      bool isEnabled();
      GCF::TM::GCFEvent::TResult initial(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test1(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test2(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test3(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test4(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test5(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test6(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test7(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test8(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test9(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test10(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult finished(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      
      
      static string m_taskName;
      static string m_RATestServerName;
      
      ARATestAnswer   m_answer;
      GCF::TM::GCFPort         m_RSPserver;
      
      int             m_test_passCounter;
      int             m_propsetLoadedCounter;
      
      GCF::PAL::GCFExtPropertySet m_extPropSetAP1;
      GCF::PAL::GCFExtPropertySet m_extPropSetAP1RCUmaintenance;
      GCF::PAL::GCFExtPropertySet m_extPropSetBoardAlert;
      GCF::PAL::GCFExtPropertySet m_extPropSetStationMaintenance;
      GCF::PAL::GCFExtPropertySet m_extPropSetLDS;
      
  };  
};

} // namespace LOFAR


#endif
