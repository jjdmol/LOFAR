//#  AVTTestTask.h: Automatic test of the Virtual Telescope logical device
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

#ifndef AVTTestTask_H
#define AVTTestTask_H

//# Includes
//# Common Includes

//# GCF Includes
#include <GCF/GCF_Task.h>
#include <GCF/GCF_Port.h>
#include <GCF/GCF_Property.h>
#include <GCF/GCF_MyPropertySet.h>
#include <boost/shared_ptr.hpp>

#include "AVTTest.h"
#include "AVTTestAnswer.h"

// forward declaration
class GCFEvent;
class GCFPortInterface;

namespace AVT
{
  class AVTTestTask : public GCFTask
  {
    public:
      AVTTestTask(AVTTest<AVTTestTask>& tester);
      virtual ~AVTTestTask();
  
      static bool   m_sBeamServerOnly;
  
    protected:
      // protected copy constructor
      AVTTestTask(const AVTTestTask&);
      // protected assignment operator
      AVTTestTask& operator=(const AVTTestTask&);
      
    private: 
      GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult propertiesLoaded(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test1(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test2(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test3(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test4(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test5(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test6(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test7(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test8(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult test9(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult finished(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult handleBeamServerEvents(GCFEvent& e, GCFPortInterface& p);
      GCFEvent::TResult beamServer(GCFEvent& e, GCFPortInterface& p);
      
      
      static string m_taskName;
      
      AVTTest<AVTTestTask>& m_tester;
      AVTTestAnswer         m_answer;
      GCFPort               m_beamserver;
  
      GCFProperty     m_propertyLDScommand;
      GCFProperty     m_propertyLDSstatus;
      GCFProperty     m_propertyLDSWGFrequency;
      GCFProperty     m_propertyLDSWGAmplitude;
      GCFProperty     m_propertyLDSWGSamplePeriod;
      GCFProperty     m_propertySBFdirectionType;
      GCFProperty     m_propertySBFdirectionAngle1;
      GCFProperty     m_propertySBFdirectionAngle2;
      GCFProperty     m_propertySBFstatus;
  
      GCFMyPropertySet      m_beamServerProperties;
      bool m_BEAMALLOC_received;
      bool m_BEAMFREE_received;
      bool m_BEAMPOINTTO_received;
      bool m_WGSETTINGS_received;
      bool m_WGENABLE_received;
      bool m_WGDISABLE_received;
      unsigned long m_statisticsTimerID;
      double m_beamAngle1;
      double m_beamAngle2;
      unsigned int m_seqnr;
  };
};

#endif
