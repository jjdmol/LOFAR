//#  AVTTestTask.cc: Implementation of the Virtual Telescope test
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

#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVDouble.h>
#include "AVTTest.h"
#include "AVTTestTask.h"
#include "PropertyDefines.h" 

#include "../src/LogicalDevice_Protocol.ph"

#include <stdio.h>

string AVTTestTask::m_taskName("AVTTest");

AVTTestTask::AVTTestTask(AVTTest& tester) :
  GCFTask((State)&AVTTestTask::initial, m_taskName),
  m_tester(tester),
  m_answer(*this),
  m_propertyLDScommand(scopedPropertyLDScommand),
  m_propertyLDSstatus(scopedPropertyLDSstatus),
  m_propertySBFdirectionType(scopedPropertySBFdirectionType),
  m_propertySBFdirectionAngle1(scopedPropertySBFdirectionAngle1),
  m_propertySBFdirectionAngle2(scopedPropertySBFdirectionAngle2),
  m_propertySBFstatus(scopedPropertySBFstatus)
{
}

AVTTestTask::~AVTTestTask()
{
}

GCFEvent::TResult AVTTestTask::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT_SIG:
      m_propertyLDSstatus.subscribe();
      m_propertySBFstatus.subscribe();
      TRAN(AVTTestTask::test1);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: set antenna orhtogonalization weights
 */
GCFEvent::TResult AVTTestTask::test1(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      // ignore during first increments
      m_tester._avttest(true);
      TRAN(AVTTestTask::test2);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: initialize EPA waveform generator
 */
GCFEvent::TResult AVTTestTask::test2(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      m_tester._avttest(true);
      TRAN(AVTTestTask::test3);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: create virtual telescope
 */
GCFEvent::TResult AVTTestTask::test3(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
    {
      // SCHEDULE <vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
      //          <frequency>,<subbands>,<direction>
      string cmd("SCHEDULE ");
      string devices("VT1,SBF1,SRG1,");
      string times("0,0,");
      string freq("110.0,");
      string subbands("0|1|2|3|4|5|6|7|8|9|10|11,");
      string direction("AZEL,0.0,0.0");
      GCFPVString command(cmd+devices+times+freq+subbands+direction);
      if (m_propertyLDScommand.setValue(command) != GCF_NO_ERROR)
      {
        m_tester._avttest(false);
        TRAN(AVTTestTask::test4);
      }
      break;
    }
    
    case F_VCHANGEMSG_SIG:
    {
      GCFPropValueEvent* pResponse = static_cast<GCFPropValueEvent*>(&e);
      assert(pResponse);
      string expectedProperty=m_propertyLDSstatus.getFullName();
      if(strncmp(pResponse->pPropName, expectedProperty.c_str(), expectedProperty.length()) == 0)
      {
        if(((GCFPVString*)pResponse->pValue)->getValue() == string("OK"))
        {
          m_tester._avttest(true);
        }
        else
        {
          m_tester._avttest(true);
        }
        TRAN(AVTTestTask::test4);
      }
    }
      
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: select beamlets
 */
GCFEvent::TResult AVTTestTask::test4(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
    {
//      GCFPVString command(string("0"));
//      if (m_propertySBFsubbandSelection.setValue(command) != GCF_NO_ERROR)
      {
        m_tester._avttest(false);
        TRAN(AVTTestTask::test5);
      }
      break;
    }
    
    case F_VCHANGEMSG_SIG:
    {
      GCFPropValueEvent* pResponse = static_cast<GCFPropValueEvent*>(&e);
      assert(pResponse);
      string expectedProperty=m_propertyLDSstatus.getFullName();
      if(strncmp(pResponse->pPropName, expectedProperty.c_str(), expectedProperty.length()) == 0)
      {
        if(((GCFPVString*)pResponse->pValue)->getValue() == string("OK"))
        {
          m_tester._avttest(true);
        }
        else
        {
          m_tester._avttest(true);
        }
        TRAN(AVTTestTask::test5);
      }
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: select subbands
 */
GCFEvent::TResult AVTTestTask::test5(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      m_tester._avttest(true);
      TRAN(AVTTestTask::test6);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: set beam direction
 */
GCFEvent::TResult AVTTestTask::test6(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
    {
      GCFPVString directionType(string("AZEL"));
      GCFPVDouble directionAngle1(45.0);
      GCFPVDouble directionAngle2(45.0);
      if(m_propertySBFdirectionType.setValue(directionType) != GCF_NO_ERROR ||
         m_propertySBFdirectionAngle1.setValue(directionAngle1) != GCF_NO_ERROR ||
         m_propertySBFdirectionAngle2.setValue(directionAngle2) != GCF_NO_ERROR)
      {
        m_tester._avttest(false);
        TRAN(AVTTestTask::test7);
      }
      break;
    }
    
    case F_VCHANGEMSG_SIG:
    {
      GCFPropValueEvent* pResponse = static_cast<GCFPropValueEvent*>(&e);
      assert(pResponse);
      string expectedProperty=m_propertySBFstatus.getFullName();
      if(strncmp(pResponse->pPropName, expectedProperty.c_str(), expectedProperty.length()) == 0)
      {
        if(((GCFPVString*)pResponse->pValue)->getValue() == string("OK"))
        {
          m_tester._avttest(true);
        }
        else
        {
          m_tester._avttest(true);
        }
        TRAN(AVTTestTask::test7);
      }
    }

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: monitor statistics
 */
GCFEvent::TResult AVTTestTask::test7(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      m_tester._avttest(true);
      TRAN(AVTTestTask::test8);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: to be done
 */
GCFEvent::TResult AVTTestTask::test8(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      m_tester._avttest(true);
      TRAN(AVTTestTask::test9);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case: cleanup
 */
GCFEvent::TResult AVTTestTask::test9(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      m_tester._avttest(true);
      TRAN(AVTTestTask::finished);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * End of all tests
 */
GCFEvent::TResult AVTTestTask::finished(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_ENTRY_SIG:
      m_propertyLDSstatus.unsubscribe();
      m_propertySBFstatus.unsubscribe();
      GCFTask::stop();
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

