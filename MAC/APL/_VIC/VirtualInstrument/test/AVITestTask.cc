//#  AVITestTask.cc: Implementation of the Virtual Telescope test
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

const char SCOPE_VIC_VIStartDaemon[] = "CCU1_VIC_VIStartDaemon";
const char SCOPE_VIC_VI1[] = "VIC_VI1";

#define NEXT_TEST(_test_, _descr_) \
  { \
    setCurSubTest(#_test_, _descr_); \
    TRAN(AVITestTask::test##_test_); \
  }

#define FINISH \
  { \
    reportSubTest(); \
    TRAN(AVITestTask::finished); \
  }

#define ABORT_TESTS \
  { \
    cout << "TESTS ABORTED due to an ERROR or terminated" << endl; \
    FINISH; \
  }

#define FAIL_AND_ABORT(_txt_) \
  { \
    FAIL(_txt_);  \
    ABORT_TESTS; \
  }

#define TESTC_ABORT_ON_FAIL(cond) \
  if (!TESTC(cond)) \
  { \
    ABORT_TESTS; \
    break; \
  }

#define TESTC_DESCR_ABORT_ON_FAIL(cond, _descr_) \
  if (!TESTC_DESCR(cond, _descr_)) \
  { \
    ABORT_TESTS; \
    break; \
  }

#include <math.h>
#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVUnsigned.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVDouble.h>
#include "APLCommon/APL_Defines.h"
#include "APLCommon/StartDaemon.h"
#include "AVITestTask.h"

#include <stdio.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <bitset>
#include <time.h>

using namespace std;

namespace LOFAR
{

using namespace APLCommon;

namespace AVI
{

string AVITestTask::m_taskName("AVITest");

AVITestTask::AVITestTask() :
  GCFTask((State)&AVITestTask::initial, m_taskName),
  Test("VirtualInstrumentTest"),
  m_answer(),
  m_test_passCounter(0),
  m_propsetLoadedCounter(0),
  m_extPropSetCCU1VISD(SCOPE_VIC_VIStartDaemon,StartDaemon::PSTYPE_STARTDAEMON.c_str(),&m_answer),
  m_extPropSetVI1(SCOPE_VIC_VI1,"TAplVicVI",&m_answer)
{
  m_answer.setTask(this);
}

AVITestTask::~AVITestTask()
{
}

void AVITestTask::run()
{
  start(); // make initial transition
  GCFTask::run();
}

GCFEvent::TResult AVITestTask::initial(GCFEvent& event, GCFPortInterface& port)
{
  LOG_DEBUG(formatString("AVITestTask(%s)::initial (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      m_extPropSetCCU1VISD.load();
      m_extPropSetVI1.load();
      break;
      
    case F_EXTPS_LOADED:
      m_propsetLoadedCounter++;
      if(m_propsetLoadedCounter==2)
      {
        NEXT_TEST(1,"TBD");
      }
      break;
    
    case F_CONNECTED:
    {
      break;
    }
  
    case F_DISCONNECTED:
    {
      break;
    }

    case F_TIMER:
    {
      break;
    }

    case F_EXIT:
    {
      break;
    }
      
    default:
      LOG_DEBUG(formatString("AVITestTask(%s)::initial, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * Test case 1: TBD
 */
GCFEvent::TResult AVITestTask::test1(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVITestTask(%s)::test1 (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      LOG_INFO("TDB");
      FINISH
      break;
    }

    default:
      LOG_DEBUG(formatString("AVITestTask(%s)::test1, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

/* 
 * End of all tests
 */
GCFEvent::TResult AVITestTask::finished(GCFEvent& event, GCFPortInterface& /*p*/)
{
  LOG_DEBUG(formatString("AVITestTask(%s)::finished (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      GCFTask::stop();
      break;

    default:
      LOG_DEBUG(formatString("AVITestTask(%s)::finished, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}


} // namespace AVI


} // namespace LOFAR

