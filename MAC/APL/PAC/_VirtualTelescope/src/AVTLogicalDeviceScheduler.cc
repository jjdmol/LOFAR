//#  AVTLogicalDeviceScheduler.cc: Implementation of the Virtual Telescope task
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

#include <GCF/GCF_Defines.h>
#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVString.h>

#include "../../../APLCommon/src/APL_Defines.h"
#include "AVTLogicalDeviceScheduler.h"
#define DECLARE_SIGNAL_NAMES
#include "LogicalDevice_Protocol.ph"

const TProperty propertiesLDS[] =
{
  {"command", GCFPValue::LPT_STRING, (GCF_READABLE_PROP | GCF_WRITABLE_PROP), ""},
  {"status", GCFPValue::LPT_STRING, (GCF_READABLE_PROP | GCF_WRITABLE_PROP), ""},
};

const TPropertySet propertySetLDS = 
{
  2, "Station_LogicalDeviceScheduler", propertiesLDS
};

string gSchedulerTaskName("AVTLogicalDeviceScheduler");


AVTLogicalDeviceScheduler::AVTLogicalDeviceScheduler() :
  GCFTask((State)&AVTLogicalDeviceScheduler::initial_state,gSchedulerTaskName),
  AVTPropertySetAnswerHandlerInterface(),
  m_propertySetAnswer(*this),
  m_properties(propertySetLDS,&m_propertySetAnswer),
  m_serverPortName(string("AVTLogicalDeviceScheduler_server")),
  m_logicalDeviceSchedulerPort(*this, m_serverPortName, GCFPortInterface::SPP, LOGICALDEVICE_PROTOCOL),
  m_logicalDeviceMap()
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::AVTLogicalDeviceScheduler(%s)",getName().c_str()));
}


AVTLogicalDeviceScheduler::~AVTLogicalDeviceScheduler()
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::~AVTLogicalDeviceScheduler(%s)",getName().c_str()));
}

string& AVTLogicalDeviceScheduler::getServerPortName()
{
  return m_serverPortName;
}

GCFEvent::TResult AVTLogicalDeviceScheduler::initial_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::initial_state (%s)",getName().c_str()));

  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case F_INIT_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::initial_state, F_INIT_SIG (%s)",getName().c_str()));
      break;

    case F_ENTRY_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::initial_state, F_ENTRY_SIG (%s)",getName().c_str()));
      // open all ports
      m_logicalDeviceSchedulerPort.open();
      break;

    case F_CONNECTED_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::initial_state, F_CONNECTED_SIG (%s)",getName().c_str()));
      break;

    case F_DISCONNECTED_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::initial_state, F_DISCONNECTED_SIG (%s)",getName().c_str()));
      m_logicalDeviceSchedulerPort.setTimer(1.0); // try again after 1 second
      break;

    case F_TIMER_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::initial_state, F_TIMER_SIG (%s)",getName().c_str()));
      m_logicalDeviceSchedulerPort.open(); // try again
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::initial_state, default (%s)",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

void AVTLogicalDeviceScheduler::handlePropertySetAnswer(GCFEvent& answer)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::handlePropertySetAnswer (%s)",getName().c_str()));
  //todo
  switch(answer.signal)
  {
    case F_VCHANGEMSG_SIG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&answer);
      assert(pPropAnswer);
      if ((pPropAnswer->pValue->getType() == GCFPValue::LPT_STRING) &&
          (strstr(pPropAnswer->pPropName, "VirtualTelescope_command") != 0))
      {
        // command received
        string command(((GCFPVString*)pPropAnswer->pValue)->getValue());
        // decode command
        unsigned int delim=command.find(' ');
        if(delim==string::npos)
        {
          delim=command.length(); // no space found
        }
        string subCommand=command.substr(0,delim);
        // SCHEDULE <vt_name>,<bf_name>,<srg_name>
        if(subCommand==string("SCHEDULE"))
        {
          string parameters[3];
          for(int i=0;i<3;i++)
          {
            unsigned int nextDelim=command.find(',',delim);
            if(nextDelim==string::npos)
            {
              nextDelim=command.length()-delim; // no space found
            }
            if(nextDelim!=0)
            {
              parameters[i] = command.substr(delim,nextDelim);
              delim=nextDelim;
            } 
          }
          if(parameters[0]!=""&&parameters[1]!=""&&parameters[2]!="")
          {
            // create SRG
//            m_logicalDeviceMap[parameters[2]]=new AVTStationReceptorGroup();
            
            // create SBF
//            boost::shared_ptr<AVTStationBeamformer> beamformer=new AVTStationBeamformer(parameters[1],primaryPropertySetSBF,sSBFAPCName,sSBFAPCScope,sBSName);
//            m_logicalDeviceMap[parameters[1]]=beamformer;
            
            // create VT
//            m_logicalDeviceMap[parameters[0]]=new AVTVirtualTelescope(parameters[0],primaryPropertySetVT,sVTAPCName,sVTAPCScope,*beamformer.get());
          }
        }
        // RELEASE <name>
        else if(subCommand==string("RELEASE"))
        {
          string parameter;
          unsigned int nextDelim=command.find(',',delim);
          if(nextDelim==string::npos)
          {
            nextDelim=command.length()-delim; // no space found
          }
          if(nextDelim!=0)
          {
            parameter = command.substr(delim,nextDelim);
            if(parameter!="")
            {
              LogicalDeviceMapT::iterator it=m_logicalDeviceMap.find(parameter);
              if(it!=m_logicalDeviceMap.end())
              {
                it->second.reset(); // destroy object
              }
            }
          } 
        }
      }
      break;
    }  
    default:
      break;
  }  
}

