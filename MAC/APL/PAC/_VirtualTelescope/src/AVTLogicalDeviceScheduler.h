//#  AVTLogicalDeviceScheduler.h: schedules logical devices
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

#ifndef AVTLogicalDeviceScheduler_H
#define AVTLogicalDeviceScheduler_H

//# Includes
//# Common Includes
#include <lofar_config.h>
#include <Common/lofar_string.h>
#include <Common/lofar_map.h>
#include <boost/shared_ptr.hpp>

//# GCF Includes
#include <GCF/GCF_Port.h>
#include <GCF/GCF_Task.h>
#include <GCF/GCF_MyPropertySet.h>
#include <GCF/GCF_Apc.h>

//# local includes
#include "AVTPropertySetAnswerHandlerInterface.h"
#include "AVTPropertySetAnswer.h"
#include "AVTLogicalDevice.h"
#include "APLInterTaskPort.h"

// forward declaration

class AVTLogicalDeviceScheduler : public GCFTask,
                                         AVTPropertySetAnswerHandlerInterface
{
  public:

    AVTLogicalDeviceScheduler(); 
    virtual ~AVTLogicalDeviceScheduler();

    bool isInitialized();
    bool findClientPort(GCFPortInterface& port,string& key);
    
    /**
     * The initial state handler. This handler is passed to the GCFTask constructor
     * to indicate that the F_INIT event which starts the state machine is handled
     * by this handler.
     * @param e The event that was received and needs to be handled by the state
     * handler.
     * @param p The port interface (see @a GCFPortInterface) on which the event
     * was received.
     */
    GCFEvent::TResult initial_state(GCFEvent& e, GCFPortInterface& p);
  
    /**
     * PropertySet answer handling
     */
    virtual void handlePropertySetAnswer(GCFEvent& answer);
    
  protected:
    // protected copy constructor
    AVTLogicalDeviceScheduler(const AVTLogicalDeviceScheduler&);
    // protected assignment operator
    AVTLogicalDeviceScheduler& operator=(const AVTLogicalDeviceScheduler&);

  private:
    void sendWGsettings();

    static string m_schedulerTaskName;
    

    AVTPropertySetAnswer  m_propertySetAnswer;
    GCFMyPropertySet      m_properties;
    GCFApc                m_apcLDS;
    bool                  m_initialized;
//    GCFPort               m_beamServer;
    GCFPort*              m_pBeamServer;
    double                m_WGfrequency;
    unsigned int          m_WGamplitude;
    unsigned int          m_WGsamplePeriod;
    
    typedef struct LogicalDeviceInfoT
    {
      boost::shared_ptr<AVTLogicalDevice> logicalDevice;
      vector<string>                      parameters;
      map<string,LogicalDeviceInfoT>      children; // recursive
    };
    typedef map<string,LogicalDeviceInfoT> LogicalDeviceMapT;
    
    typedef struct SchedulableLogicalDeviceInfoT
    {
      boost::shared_ptr<AVTLogicalDevice> logicalDevice;
      boost::shared_ptr<APLInterTaskPort> clientPort;
      bool                                toBeReleased;
      int                                 startTime;
      int                                 stopTime;
      vector<string>                      parameters;
      LogicalDeviceMapT                   children; // recursive
    };
    typedef map<string,SchedulableLogicalDeviceInfoT> SchedulableLogicalDeviceMapT;
    
    SchedulableLogicalDeviceMapT  m_logicalDeviceMap;
};
#endif
