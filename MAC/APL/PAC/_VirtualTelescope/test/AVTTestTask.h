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
#include <GCF/GCF_MyPropertySet.h>
#include <GCF/GCF_Apc.h>

//# VirtualTelescope Includes
#include "../src/AVTVirtualTelescope.h"
#include "../src/AVTStationBeamformer.h"

// forward declaration
class GCFEvent;
class GCFPortInterface;
class AVTTest;

class AVTTestTask : public GCFTask
{
  public:
    AVTTestTask(AVTTest& tester);
    virtual ~AVTTestTask();

  protected:
    // protected copy constructor
    AVTTestTask(const AVTTestTask&);
    // protected assignment operator
    AVTTestTask& operator=(const AVTTestTask&);
    
  private: 
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
    GCFEvent::TResult finished(GCFEvent& e, GCFPortInterface& p);
    
    AVTStationBeamformer m_beamformer;
    AVTVirtualTelescope  m_virtualTelescope;
    GCFPort              m_BFPort;
    GCFPort              m_VTPort;
    
    AVTTest& m_tester;
};

#endif
