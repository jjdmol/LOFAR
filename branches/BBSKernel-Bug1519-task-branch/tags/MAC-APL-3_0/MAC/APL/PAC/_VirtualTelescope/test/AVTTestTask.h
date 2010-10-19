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
#include <list>
#include <Suite/test.h>

//# GCF Includes
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/PAL/GCF_Property.h>
#include <GCF/PAL/GCF_ExtPropertySet.h>
#include <boost/shared_ptr.hpp>

#include "AVTTestAnswer.h"

// forward declaration
namespace LOFAR
{
namespace AVT
{
class GCF::TM::GCFEvent;
class GCF::TM::GCFPortInterface;


  class AVTTestTask : public GCF::TM::GCFTask, public Test
  {
    public:
      AVTTestTask();
      virtual ~AVTTestTask();
       virtual void run();
  
      static bool   m_sBeamServerOnly;
  
    protected:
      // protected copy constructor
      AVTTestTask(const AVTTestTask&);
      // protected assignment operator
      AVTTestTask& operator=(const AVTTestTask&);
      
    private: 
      GCF::TM::GCFEvent::TResult initial(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult propertiesLoaded(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test1(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test2(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test3(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test4(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test5(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test6(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test7(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test8(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult test9(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult finished(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult handleBeamServerEvents(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      GCF::TM::GCFEvent::TResult beamServer(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p);
      
      
      static string m_taskName;
      
      AVTTestAnswer         m_answer;
      GCF::TM::GCFTCPPort            m_beamserver;
  
      GCF::PAL::GCFExtPropertySet m_extPropsetLDS;
      GCF::PAL::GCFExtPropertySet m_extPropsetLDSWG;
      GCF::PAL::GCFExtPropertySet m_extPropsetSBF1;
      bool m_propsetLDloaded;
      bool m_propsetLDWGloaded;
      bool m_propsetSBFloaded;
  
      bool m_BEAMALLOC_received;
      bool m_BEAMFREE_received;
      bool m_BEAMPOINTTO_received;
      bool m_WGSETTINGS_received;
      bool m_WGENABLE_received;
      bool m_WGDISABLE_received;
      double m_beamAngle1;
      double m_beamAngle2;
      unsigned int m_seqnr;
      std::list<GCF::TM::GCFPortInterface*> m_client_list;  // list of beamserver clients
      
  };
};
};

#endif
