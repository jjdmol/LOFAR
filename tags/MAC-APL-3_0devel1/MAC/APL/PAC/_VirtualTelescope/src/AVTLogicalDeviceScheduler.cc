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

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <time.h>

#include <GCF/GCF_Defines.h>
#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_PVUnsigned.h>
#include <GCF/ParameterSet.h>

#include <APLCommon/APL_Defines.h>
#include <APLCommon/APLInterTaskPort.h>
#include "AVTLogicalDeviceScheduler.h"
#include "LogicalDevice_Protocol.ph"
#include "AVTDefines.h"
#include "AVTStationBeamformer.h"
#include "AVTVirtualTelescope.h"
#include "AVTStationReceptor.h"
#include "AVTStationReceptorGroup.h"
#include "AVTUtilities.h"
#include "AVTPropertyDefines.h"

#undef PACKAGE
#undef VERSION
#define DECLARE_SIGNAL_NAMES
#include "ABS_Protocol.ph"

using namespace LOFAR;
using namespace GCF;
using namespace AVT;
using namespace std;
using namespace boost;

string AVTLogicalDeviceScheduler::m_schedulerTaskName("LogicalDeviceScheduler");
string timerPortName("timerPort");

AVTLogicalDeviceScheduler::AVTLogicalDeviceScheduler() :
  GCFTask((State)&AVTLogicalDeviceScheduler::initial_state,m_schedulerTaskName),
  AVTPropertySetAnswerHandlerInterface(),
  m_propertySetAnswer(*this),
  m_properties(SCOPE_PAC_LogicalDeviceScheduler,TYPE_LCU_PAC_LogicalDeviceScheduler.c_str(),false,&m_propertySetAnswer),
  m_propertiesWG(SCOPE_PAC_LogicalDeviceScheduler_WaveFormGenerator,TYPE_LCU_PAC_WaveformGenerator.c_str(),false,&m_propertySetAnswer),
  m_propsetConfigured(false),
  m_propsetWGConfigured(false),
  m_pBeamServer(0),
  m_WGfrequency(0.0),
  m_WGamplitude(0),
  m_WGsamplePeriod(0),
  m_logicalDeviceMap(),
  m_logicalDeviceSchedule(),
  m_maintenanceSchedule(),
  m_timerPort(*this, timerPortName, GCFPortInterface::SPP, LOGICALDEVICE_PROTOCOL),
  m_resourceManager(AVTResourceManager::instance())
{
  registerProtocol(LOGICALDEVICE_PROTOCOL, LOGICALDEVICE_PROTOCOL_signalnames);
  LOG_DEBUG(formatString("AVTLogicalDeviceScheduler::%s(%s)",__func__,getName().c_str()));
  m_properties.enable();
  m_propertiesWG.enable();
}


AVTLogicalDeviceScheduler::~AVTLogicalDeviceScheduler()
{
  LOG_DEBUG(formatString("AVTLogicalDeviceScheduler::%s(%s)",__func__,getName().c_str()));
}

shared_ptr<AVTStationReceptor> AVTLogicalDeviceScheduler::addReceptor(string srName,const list<TPropertyInfo>& requiredResources)
{
  LOG_DEBUG(formatString("%s(%s)(%s)",__func__,getName().c_str(),srName.c_str()));
  // create receptor logical device
  LogicalDeviceInfoT srInfo;
  string srApcName(APC_SR);
  if(ParameterSet::instance()->isDefined(string(PARAM_APC)+srName))
  {
    srApcName = ParameterSet::instance()->getString(string(PARAM_APC)+srName);
  }
  shared_ptr<AVTStationReceptor> sr(new AVTStationReceptor(srName,string("PAC")+string(SCOPESEPARATOR)+srName,srApcName,requiredResources));
  srInfo.logicalDevice = sr;
  srInfo.permanent = true;
  srInfo.clientPort.reset(new APLInterTaskPort(*sr,*this, sr->getServerPortName(), GCFPortInterface::SAP, LOGICALDEVICE_PROTOCOL));
  sr->addClientInterTaskPort(srInfo.clientPort.get());
  
  m_logicalDeviceMap[srName]=srInfo;
  sr->start();
  srInfo.clientPort->open();
  
  return sr;
}

void AVTLogicalDeviceScheduler::addReceptorGroup(string srgName,vector<shared_ptr<AVTStationReceptor> >& receptors)
{
  LOG_DEBUG(formatString("%s(%s)(%s)",__func__,getName().c_str(),srgName.c_str()));
  LogicalDeviceInfoT srgInfo;
  string srgApcName(APC_SRG);
  if(ParameterSet::instance()->isDefined(string(PARAM_APC)+srgName))
  {
    srgApcName = ParameterSet::instance()->getString(string(PARAM_APC)+srgName);
  }
  
  // search receptor logical devices
  vector<shared_ptr<AVTStationReceptor> >::iterator rIt;
  for(rIt=receptors.begin();rIt!=receptors.end();++rIt)
  {
    string ldName = (*rIt)->getName();
    LogicalDeviceMapIterT ldMapIt = m_logicalDeviceMap.find(ldName);
    if(ldMapIt != m_logicalDeviceMap.end())
    {
      srgInfo.children[ldName]=ldMapIt->second;
    }
  }
  
  // create receptor group logical device
  shared_ptr<AVTStationReceptorGroup> srg(new AVTStationReceptorGroup(srgName,string("PAC")+string(SCOPESEPARATOR)+srgName,srgApcName,receptors));
  srgInfo.logicalDevice = srg;
  srgInfo.permanent = true;
  // create the connection with the new logical device
  srgInfo.clientPort.reset(new APLInterTaskPort(*srg,*this, srg->getServerPortName(), GCFPortInterface::SAP, LOGICALDEVICE_PROTOCOL));

  m_logicalDeviceMap[srgName]=srgInfo;
  srg->start();
  srgInfo.clientPort->open();
}

bool AVTLogicalDeviceScheduler::isInitialized()
{
  return (m_propsetConfigured && m_propsetWGConfigured);
;
}

AVTLogicalDeviceScheduler::LogicalDeviceMapIterT AVTLogicalDeviceScheduler::findLogicalDevice(const unsigned long scheduleId)
{
  LOG_DEBUG(formatString("%s(%s)(scheduleId=%d)",__func__,getName().c_str(),scheduleId));
  LogicalDeviceMapIterT ldIt = m_logicalDeviceMap.end();
  
  // find the schedule
  LogicalDeviceScheduleIterT scheduleIt = m_logicalDeviceSchedule.find(scheduleId);
  if(scheduleIt != m_logicalDeviceSchedule.end())
  {
    // find the logical device of the schedule
    ldIt=m_logicalDeviceMap.find(scheduleIt->second.deviceName);
  }
  return ldIt;
}

AVTLogicalDeviceScheduler::LogicalDeviceMapIterT AVTLogicalDeviceScheduler::findClientPort(GCFPortInterface& port)
{
  LOG_DEBUG(formatString("%s(%s)",__func__,getName().c_str()));
  LogicalDeviceMapIterT it;
  for(it=m_logicalDeviceMap.begin();it!=m_logicalDeviceMap.end()&&(&port!=it->second.clientPort.get());++it);
  return it;
}

AVTLogicalDeviceScheduler::LogicalDeviceScheduleIterT AVTLogicalDeviceScheduler::findSchedule(const string& deviceName,LogicalDeviceScheduleIterT beginIt)
{
  LOG_DEBUG(formatString("%s(%s)(ld=%s)",__func__,getName().c_str(),deviceName.c_str()));
  LogicalDeviceScheduleIterT scheduleIt;
  for(scheduleIt=beginIt;scheduleIt!=m_logicalDeviceSchedule.end()&&(deviceName!=scheduleIt->second.deviceName);++scheduleIt);
  return scheduleIt;
}

bool AVTLogicalDeviceScheduler::submitSchedule(const unsigned long scheduleId,const string& deviceName, const vector<string> scheduleParameters, boost::shared_ptr<APLInterTaskPort> clientPort)
{
  bool result=false;

  int rawStartTime = atoi(scheduleParameters[0].c_str()); // starttime
  int rawStopTime  = atoi(scheduleParameters[1].c_str()); // stoptime
  
  int prepareDelayInt = ParameterSet::instance()->getInt(PARAM_PREPARETIME);
  
  boost::posix_time::time_duration prepareDelay = boost::posix_time::seconds(prepareDelayInt);
  boost::posix_time::time_duration startDelay   = boost::posix_time::seconds(2*prepareDelayInt);
  boost::posix_time::time_duration stopDelay    = boost::posix_time::seconds(3*prepareDelayInt);
  
  time_t timeNow = time(0);
  struct tm* utcTimeStruct = gmtime(&timeNow);
  time_t utcTime = mktime(utcTimeStruct);
  boost::posix_time::ptime curUTCtime   = boost::posix_time::from_time_t(utcTime);
  boost::posix_time::ptime startTime    = boost::posix_time::from_time_t(rawStartTime);
  boost::posix_time::ptime stopTime     = boost::posix_time::from_time_t(rawStopTime);
  boost::posix_time::ptime prepareTime  = startTime - prepareDelay; //  - 10 seconds

  // check validity of the times
  if(startTime < curUTCtime + startDelay)
  {
    startTime = curUTCtime + startDelay;
    prepareTime = curUTCtime + prepareDelay;
  }
  if(rawStopTime == 0) // infinite. let's say 100 years
  {
    stopTime = curUTCtime + boost::gregorian::date_duration(100*365);
  }
  else if(stopTime < curUTCtime + startDelay)
  {
    stopTime = curUTCtime + stopDelay;
  }
  LOG_DEBUG(formatString("(%s)\ncurrent time:          %s\nscheduled preparetime: %s\nscheduled starttime:   %s\nscheduled stoptime:    %s\n",__func__,posix_time::to_simple_string(curUTCtime).c_str(),posix_time::to_simple_string(prepareTime).c_str(),posix_time::to_simple_string(startTime).c_str(),posix_time::to_simple_string(stopTime).c_str()));

  // check if the schedule already exists
  LogicalDeviceScheduleIterT schIt = m_logicalDeviceSchedule.find(scheduleId);
  if(schIt != m_logicalDeviceSchedule.end())
  {
	  LOG_DEBUG(formatString("(%s) Schedule %d already exists. Changing parameters",__func__,scheduleId));
    // it exists.
    if(prepareTime < curUTCtime)
    {
      // can't change prepare time because it has passed
    }
    else
    {
      // update the preparation parameters
      schIt->second.parameters = scheduleParameters;
      // cancel current prepare timer and create a new one
      clientPort->cancelTimer(schIt->second.prepareTimerId);
		  LOG_DEBUG(formatString("(%s) PrepareTimer %d cancelled",__func__,schIt->second.prepareTimerId));
      
      boost::posix_time::time_duration delay;
      delay = prepareTime - curUTCtime; 
      schIt->second.prepareTimerId = clientPort->setTimer(static_cast<long int>(delay.total_seconds())); // set the prepareTimer
		  LOG_DEBUG(formatString("(%s) PrepareTimer %d created",__func__,schIt->second.prepareTimerId));
    }
    if(startTime < curUTCtime)
    {
      // can't change start time because it has passed
    }
    else
    {
      schIt->second.startTime = rawStartTime;
      // cancel current start timer and create a new one
      clientPort->cancelTimer(schIt->second.startTimerId);
		  LOG_DEBUG(formatString("(%s) StartTimer %d cancelled",__func__,schIt->second.startTimerId));
      
      boost::posix_time::time_duration delay;
      delay = startTime - curUTCtime; 
      schIt->second.startTimerId = clientPort->setTimer(static_cast<long int>(delay.total_seconds())); // set the startTimer
		  LOG_DEBUG(formatString("(%s) StartTimer %d created",__func__,schIt->second.startTimerId));
    }
    if(stopTime < curUTCtime)
    {
      // can't change stop time because it has passed
    }
    else
    {
      schIt->second.stopTime = rawStopTime;
      // cancel current stop timer and create a new one
      clientPort->cancelTimer(schIt->second.stopTimerId);
		  LOG_DEBUG(formatString("(%s) StopTimer %d cancelled",__func__,schIt->second.stopTimerId));
      
      boost::posix_time::time_duration delay;
      delay = stopTime - curUTCtime; 
      schIt->second.stopTimerId = clientPort->setTimer(static_cast<long int>(delay.total_seconds())); // set the stopTimer
		  LOG_DEBUG(formatString("(%s) StopTimer %d created",__func__,schIt->second.stopTimerId));
    }
  }
  else
  {
	  LOG_DEBUG(formatString("(%s) Creating Schedule %d",__func__,scheduleId));
    // create the schedule entry
    LogicalDeviceScheduleInfoT vtSchedule;
    
    vtSchedule.deviceName = deviceName;
    vtSchedule.parameters = scheduleParameters;
    vtSchedule.startTime = rawStartTime;
    vtSchedule.stopTime = rawStopTime;
  
    // create the timers
    boost::posix_time::time_duration delay;
    delay = prepareTime - curUTCtime; 
    vtSchedule.prepareTimerId = clientPort->setTimer(static_cast<long int>(delay.total_seconds())); // set the prepareTimer
    LOG_DEBUG(formatString("(%s) PrepareTimer %d created",__func__,vtSchedule.prepareTimerId));

    delay = startTime - curUTCtime; 
    vtSchedule.startTimerId = clientPort->setTimer(static_cast<long int>(delay.total_seconds())); // set the startTimer
    LOG_DEBUG(formatString("(%s) StartTimer %d created",__func__,vtSchedule.startTimerId));

    delay = stopTime - curUTCtime; 
    vtSchedule.stopTimerId = clientPort->setTimer(static_cast<long int>(delay.total_seconds())); // set the stopTimer
    LOG_DEBUG(formatString("(%s) StopTimer %d created",__func__,vtSchedule.stopTimerId));
  
    m_logicalDeviceSchedule[scheduleId] = vtSchedule;
  }  
  
  return result;
}

bool AVTLogicalDeviceScheduler::checkPrepareTimer(const string& deviceName, unsigned long timerId)
{
  bool isPrepareTimer=false;
  LogicalDeviceScheduleIterT it = findSchedule(deviceName,m_logicalDeviceSchedule.begin());
  while(!isPrepareTimer && it != m_logicalDeviceSchedule.end())
  {
    // schedule for logical device found. check prepareTimerId
    isPrepareTimer = (it->second.prepareTimerId == timerId);
    if(!isPrepareTimer)
    {
      ++it;
      it = findSchedule(deviceName,it);
    }
    else
    {
      LogicalDeviceMapIterT ldIt = m_logicalDeviceMap.find(deviceName);
      if(ldIt != m_logicalDeviceMap.end())
      {
        ldIt->second.currentSchedule = it->first;
      }
    }
  }
  LOG_DEBUG(formatString("Timer %d is %sa schedule prepare timer",timerId,(isPrepareTimer?"":"NOT ")));
  return isPrepareTimer;
}

bool AVTLogicalDeviceScheduler::checkStartTimer(const string& deviceName, unsigned long timerId)
{
  bool isStartTimer=false;
  LogicalDeviceScheduleIterT it = findSchedule(deviceName,m_logicalDeviceSchedule.begin());
  while(!isStartTimer && it != m_logicalDeviceSchedule.end())
  {
    // schedule for logical device found. check startTimerId
    isStartTimer = (it->second.startTimerId == timerId);
    if(!isStartTimer)
    {
      ++it;
      it = findSchedule(deviceName,it);
    }
    else
    {
      LogicalDeviceMapIterT ldIt = m_logicalDeviceMap.find(deviceName);
      if(ldIt != m_logicalDeviceMap.end())
      {
        ldIt->second.currentSchedule = it->first;
      }
    }
  }
  LOG_DEBUG(formatString("Timer %d is %sa schedule start timer",timerId,(isStartTimer?"":"NOT ")));
  return isStartTimer;
}

bool AVTLogicalDeviceScheduler::checkStopTimer(const string& deviceName, unsigned long timerId)
{
  bool isStopTimer=false;
  LogicalDeviceScheduleIterT it = findSchedule(deviceName,m_logicalDeviceSchedule.begin());
  while(!isStopTimer && it != m_logicalDeviceSchedule.end())
  {
    // schedule for logical device found. check StopTimerId
    isStopTimer = (it->second.stopTimerId == timerId);
    if(!isStopTimer)
    {
      ++it;
      it = findSchedule(deviceName,it);
    }
    else
    {
      LogicalDeviceMapIterT ldIt = m_logicalDeviceMap.find(deviceName);
      if(ldIt != m_logicalDeviceMap.end())
      {
        ldIt->second.currentSchedule = 0;
      }
    }
  }
  LOG_DEBUG(formatString("Timer %d is %sa schedule stop timer",timerId,(isStopTimer?"":"NOT ")));
  return isStopTimer;
}

bool AVTLogicalDeviceScheduler::checkMaintenanceStartTimer(unsigned long timerId, MaintenanceScheduleIterT& scheduleIt)
{
  bool isStartTimer=false;
  scheduleIt = m_maintenanceSchedule.begin();
  while(!isStartTimer && scheduleIt != m_maintenanceSchedule.end())
  {
    isStartTimer = (timerId == scheduleIt->second.startTimerId);
    if(!isStartTimer)
    {
      ++scheduleIt;
    }
  }
  LOG_DEBUG(formatString("Timer %d is %sa maintenance start timer",timerId,(isStartTimer?"":"NOT ")));
  return isStartTimer;
}

bool AVTLogicalDeviceScheduler::checkMaintenanceStopTimer(unsigned long timerId, MaintenanceScheduleIterT& scheduleIt)
{
  bool isStopTimer=false;
  scheduleIt = m_maintenanceSchedule.begin();
  while(!isStopTimer && scheduleIt != m_maintenanceSchedule.end())
  {
    isStopTimer = (timerId == scheduleIt->second.stopTimerId);
    if(!isStopTimer)
    {
      ++scheduleIt;
    }
  }
  LOG_DEBUG(formatString("Timer %d is %sa maintenance stop timer",timerId,(isStopTimer?"":"NOT ")));
  return isStopTimer;
}

GCFEvent::TResult AVTLogicalDeviceScheduler::initial_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_DEBUG(formatString("%s(%s) (%s)",__func__,getName().c_str(),evtstr(event)));

  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      /*
      *  Receptor and ReceptorGroup configuration
      *  GCF4.0 allows permanent database items, so the RCU's and SRG's can be configured
      *  in the database at that time.
      */

      // create receptors
      vector<shared_ptr<AVTStationReceptor> > receptors;
      
      list<TPropertyInfo> requiredResources;
      getRequiredResources(requiredResources,1,1,1,1,1);
      receptors.push_back(addReceptor(string("SR1"),requiredResources));
      getRequiredResources(requiredResources,1,1,1,1,2);
      receptors.push_back(addReceptor(string("SR2"),requiredResources));
      getRequiredResources(requiredResources,2,1,1,1,1);
      receptors.push_back(addReceptor(string("SR3"),requiredResources));
      getRequiredResources(requiredResources,2,1,1,1,2);
      receptors.push_back(addReceptor(string("SR4"),requiredResources));
      getRequiredResources(requiredResources,3,1,1,1,1);
      receptors.push_back(addReceptor(string("SR5"),requiredResources));
      getRequiredResources(requiredResources,3,1,1,1,2);
      receptors.push_back(addReceptor(string("SR6"),requiredResources));
      getRequiredResources(requiredResources,4,1,1,1,1);
      receptors.push_back(addReceptor(string("SR7"),requiredResources));
      getRequiredResources(requiredResources,4,1,1,1,2);
      receptors.push_back(addReceptor(string("SR8"),requiredResources));

      // create receptor groups
      vector<shared_ptr<AVTStationReceptor> > receptorsSRG1;
      vector<shared_ptr<AVTStationReceptor> > receptorsSRG2;
      vector<shared_ptr<AVTStationReceptor> > receptorsSRG3;
      vector<shared_ptr<AVTStationReceptor> > receptorsSRG4;
      
      receptorsSRG1.push_back(receptors[0]);
      receptorsSRG1.push_back(receptors[1]);
      
      receptorsSRG2.push_back(receptors[2]);
      receptorsSRG2.push_back(receptors[3]);
      
      receptorsSRG3.push_back(receptors[4]);
      receptorsSRG3.push_back(receptors[5]);
      
      receptorsSRG4.push_back(receptors[0]);
      receptorsSRG4.push_back(receptors[1]);
      receptorsSRG4.push_back(receptors[2]);
      receptorsSRG4.push_back(receptors[3]);
      receptorsSRG4.push_back(receptors[4]);
      receptorsSRG4.push_back(receptors[5]);

      addReceptorGroup(string("SRG1"),receptorsSRG1);
      addReceptorGroup(string("SRG2"),receptorsSRG2);
      addReceptorGroup(string("SRG3"),receptorsSRG3);
      addReceptorGroup(string("SRG4"),receptorsSRG4);

      m_timerPort.open();
      break;
    }
    
    case F_CONNECTED:
      break;
    
    case LOGICALDEVICE_INITIALIZED:
    {
      break;
    }

    case LOGICALDEVICE_CLAIMED:
    {
      LogicalDeviceMapIterT it = findClientPort(port);
      if(it!=m_logicalDeviceMap.end())
      {
        // get the corresponding schedule:
        LogicalDeviceScheduleIterT schIt = m_logicalDeviceSchedule.find(it->second.currentSchedule);
        if(schIt != m_logicalDeviceSchedule.end())
        {
          // send prepare message:
          string prepareParameters;
          AVTUtilities::encodeParameters(schIt->second.parameters,prepareParameters);

          LOGICALDEVICEPrepareEvent prepareEvent;
          prepareEvent.parameters=prepareParameters;
          
          // send prepare to Virtual Telescope. VT will send prepare to SBF and SRG
          port.send(prepareEvent);
        }
      }
      break;
    }
    case LOGICALDEVICE_PREPARED:
    {
      break;
    }
        
    case LOGICALDEVICE_RESUMED:
    {
      break;
    }

    case LOGICALDEVICE_SUSPENDED:
    {
      
      LogicalDeviceMapIterT it = findClientPort(port);
      if(it!=m_logicalDeviceMap.end())
      {
        // check if the LD can be deleted if no more schedules exist
        if(!it->second.permanent)
        {
          LogicalDeviceScheduleIterT scheduleIt = findSchedule(it->first,m_logicalDeviceSchedule.begin());
          if(scheduleIt == m_logicalDeviceSchedule.end())
          {
            // send release message because no more schedules exist for this logical device
            LOGICALDEVICEReleaseEvent releaseEvent;
            port.send(releaseEvent);
          }
        }
      }
      break;
    }

    case LOGICALDEVICE_RELEASED:
    {
      
      LogicalDeviceMapIterT it = findClientPort(port);
      if(it!=m_logicalDeviceMap.end())
      {
        // get the corresponding schedule:
        LogicalDeviceScheduleIterT scheduleIt = m_logicalDeviceSchedule.find(it->second.currentSchedule);
        // cancel current timers of this LD
        if(scheduleIt != m_logicalDeviceSchedule.end())
        {
          it->second.clientPort->cancelTimer(scheduleIt->second.prepareTimerId);
          it->second.clientPort->cancelTimer(scheduleIt->second.startTimerId);
          it->second.clientPort->cancelTimer(scheduleIt->second.stopTimerId);
          LOG_DEBUG(formatString("(%s) Schedule %d cancelled. timers %d, %d and %d also cancelled.",__func__,scheduleIt->first,scheduleIt->second.prepareTimerId,scheduleIt->second.startTimerId,scheduleIt->second.stopTimerId));
          m_logicalDeviceSchedule.erase(scheduleIt);
        }
        // check if the LD can be deleted if no more schedules exist
        if(!it->second.permanent)
        {
          // remove the object from the map. The Port instance and children Logical Device instances
          // are destroyed too because they are smart pointers.
//          m_logicalDeviceMap.erase(it);
          LOG_DEBUG(formatString("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
          LOG_DEBUG(formatString("!!!!! NOT DESTROYING RELEASED OBJECT BECAUSE OF DESCTRUCTION PROBLEMS !!!!!!"));
          LOG_DEBUG(formatString("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
        }
      }
      break;
    }
        
    case F_DISCONNECTED:
    {
      LogicalDeviceMapIterT it = findClientPort(port);
      if(it!=m_logicalDeviceMap.end())
      {
        port.setTimer(2.0);
      }
      break;
    }
    
    case F_TIMER:
    {
      GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
      
      LogicalDeviceMapIterT it = findClientPort(port);
      if(it!=m_logicalDeviceMap.end())
      {
        LOG_DEBUG("Logical device timer triggered");
        if(checkPrepareTimer(it->first,timerEvent.id))
        {
          port.cancelTimer(timerEvent.id);
          LOG_DEBUG(formatString("(%s) PrepareTimer %d triggered and cancelled",__func__,timerEvent.id));
          // this is a prepare timer for the schedule of a logical device. claim the device
          LOGICALDEVICEClaimEvent claimEvent;
          port.send(claimEvent);
          // when the device (and all it's children) is claimed, the prepare message is sent
          // automatically
        }
        else if(checkStartTimer(it->first,timerEvent.id))
        {
          port.cancelTimer(timerEvent.id);
          LOG_DEBUG(formatString("(%s) StartTimer %d triggered and cancelled",__func__,timerEvent.id));
          // this is a start timer for the schedule of a logical device. resume the device
          LOGICALDEVICEResumeEvent resumeEvent;
          port.send(resumeEvent);
        }
        else if(checkStopTimer(it->first,timerEvent.id))
        {
          port.cancelTimer(timerEvent.id);
          LOG_DEBUG(formatString("(%s) StopTimer %d triggered and cancelled",__func__,timerEvent.id));
          // this is a stop timer for the schedule of a logical device. suspend the device
          LOGICALDEVICESuspendEvent suspendEvent;
          port.send(suspendEvent);
        }
        else if(!port.isConnected())
        {
          // not a start or stop timer
          port.open();
        }
      }
      else if(&port == &m_timerPort)
      {
        LOG_DEBUG("Maintenance timer triggered");
        
        // check maintenance timers:
        MaintenanceScheduleIterT maintenanceIt;
        if(checkMaintenanceStartTimer(timerEvent.id,maintenanceIt))
        {
          GCFPVUnsigned inMaintenance(1);
          maintenanceIt->second.pMaintenanceProperty->setValue(inMaintenance);
        }
        else if(checkMaintenanceStopTimer(timerEvent.id,maintenanceIt))
        {
          GCFPVUnsigned outofMaintenance(0);
          maintenanceIt->second.pMaintenanceProperty->setValue(outofMaintenance);
          // also remove the schedule
          maintenanceIt->second.pMaintenanceProperty.reset();
          m_maintenanceSchedule.erase(maintenanceIt);
        }
      }
      break;
    }
    
    default:
      LOG_DEBUG(formatString("%s(%s), default",__func__,getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

void AVTLogicalDeviceScheduler::handlePropertySetAnswer(GCFEvent& answer)
{
//  LOG_DEBUG(formatString("%s(%s) (%s)",__func__,getName().c_str(),evtstr(answer)));

  switch(answer.signal)
  {
    case F_MYPS_ENABLED:
    {
      // property set loaded, now load apc
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
      if(pPropAnswer->result == GCF_NO_ERROR)
      {
        if(strstr(pPropAnswer->pScope, SCOPE_PAC_LogicalDeviceScheduler_WaveFormGenerator) != 0)
        {
          m_propertiesWG.configure(APC_WaveformGenerator);
        }
        else if(strstr(pPropAnswer->pScope, SCOPE_PAC_LogicalDeviceScheduler) != 0)
        {
          m_properties.configure(APC_LogicalDeviceScheduler);
        }
      }
      else
      {
        LOG_ERROR(formatString("%s : PropertySet %s NOT ENABLED",getName().c_str(),pPropAnswer->pScope));
      }
      break;
    }
    
    case F_PS_CONFIGURED:
    {
      GCFConfAnswerEvent* pConfAnswer=static_cast<GCFConfAnswerEvent*>(&answer);
      if(pConfAnswer->result == GCF_NO_ERROR)
      {
        if(strstr(pConfAnswer->pScope, SCOPE_PAC_LogicalDeviceScheduler_WaveFormGenerator) != 0)
        {
          m_propsetWGConfigured=true;
        }
        else if(strstr(pConfAnswer->pScope, SCOPE_PAC_LogicalDeviceScheduler) != 0)
        {
          m_propsetConfigured=true;
        }
      }
      else
      {
        LOG_ERROR(formatString("%s : apc %s NOT LOADED",getName().c_str(),pConfAnswer->pApcName));
      }
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);
      if (pPropAnswer->pValue->getType() == LPT_STRING &&
          strstr(pPropAnswer->pPropName, SCOPE_PAC_LogicalDeviceScheduler) != 0 &&
          strstr(pPropAnswer->pPropName, PROPNAME_COMMAND) != 0 )
      {
        // command received
        string commandString(((GCFPVString*)pPropAnswer->pValue)->getValue());
        
        GCFPVString status("OK");

        LOG_DEBUG(formatString("Command received: (%s)",commandString.c_str()));
        
        vector<string> parameters;
        string command;
        AVTUtilities::decodeCommand(commandString,command,parameters);
        
        // SCHEDULE <sch.nr>,VT|SR|SRG,....
        if(command==string(LD_COMMAND_SCHEDULE))
        {
          if(parameters.size()<5)
          {
            // return error in status property
            string statusString("Error SCHEDULE");            
            if(parameters.size()>0)
            {
              statusString += string(",")+parameters[0];
            }
            statusString += string(",incorrect number of parameters");
            status.setValue(statusString);
          }
          else
          {
            // which logical device?
            if(parameters[1] == string("VT"))
            {
              // SCHEDULE <sch.nr>,VT,<vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
              //          <frequency>,<subbands>,<directiontype>,<angle1>,<angle2>
              // it's a virtual telescope, check the number of parameters
              if(parameters.size()!=12)
              {
                status.setValue(string("Error SCHEDULE,")+parameters[0]+string(",incorrect number of parameters"));
              }
              else
              {
                LogicalDeviceInfoT srgInfo;
                LogicalDeviceInfoT sbfInfo;
                LogicalDeviceInfoT vtInfo;
                bool schedulingError=false;

                // Check if the thing has already been created
                bool ldExists=false;
                LogicalDeviceMapIterT ldIt=m_logicalDeviceMap.find(parameters[2]);
                if(ldIt!=m_logicalDeviceMap.end())
                {
                  ldExists=true;
                  vtInfo = ldIt->second;
                }
                if(!ldExists)
                {
                  // find SRG
                  boost::shared_ptr<AVTStationReceptorGroup> srg;
                  LogicalDeviceMapIterT ldIt=m_logicalDeviceMap.find(parameters[4]);
                  if(ldIt!=m_logicalDeviceMap.end())
                  {
                    srgInfo.logicalDevice = ldIt->second.logicalDevice;
                    srg=boost::static_pointer_cast<AVTStationReceptorGroup>(srgInfo.logicalDevice);

                    // create SBF
                    boost::shared_ptr<AVTStationBeamformer> sbf;
                    string sbfApcName(APC_SBF);
                    if(ParameterSet::instance()->isDefined(string(PARAM_APC)+parameters[3]))
                    {
                      sbfApcName = ParameterSet::instance()->getString(string(PARAM_APC)+parameters[3]);
                    }
                    string bsName = ParameterSet::instance()->getString(PARAM_BEAMSERVERPORT);
                    sbf.reset(new AVTStationBeamformer(parameters[3],string("PAC")+string(SCOPESEPARATOR)+parameters[2]+string(SCOPESEPARATOR)+parameters[3],sbfApcName,bsName));
                    sbfInfo.logicalDevice=sbf;
                    sbfInfo.permanent = false;
                    if(m_pBeamServer==0)
                    {
                      m_pBeamServer=&sbf->getBeamServerPort();
                    }

                    // create VT
                    string vtApcName(APC_VT);
                    if(ParameterSet::instance()->isDefined(string(PARAM_APC)+parameters[2]))
                    {
                      vtApcName = ParameterSet::instance()->getString(string(PARAM_APC)+parameters[2]);
                    }
                    boost::shared_ptr<AVTVirtualTelescope> vt(new AVTVirtualTelescope(parameters[2],string("PAC")+string(SCOPESEPARATOR)+parameters[2],vtApcName,*(sbf.get()),*(srg.get())));
                    vtInfo.logicalDevice=vt;
                    vtInfo.permanent = false;
                    vtInfo.clientPort.reset(new APLInterTaskPort(*vt.get(),*this, vt->getServerPortName(), GCFPortInterface::SAP, LOGICALDEVICE_PROTOCOL));
                    vt->addClientInterTaskPort(vtInfo.clientPort.get());
                    vtInfo.children[parameters[3]]=sbfInfo;
                    vtInfo.children[parameters[4]]=srgInfo;

                    // put the new logical device in the map
                    m_logicalDeviceMap[parameters[2]]=vtInfo;
                  }
                  else
                  {
                    status.setValue(string("Error SCHEDULE,")+parameters[0]+string(",SRG not found"));
                    schedulingError=true;
                  }
                }

                if(!schedulingError)
                {
                  // copy parameters[5] to parameters[end] into a new vector
                  vector<string> scheduleParameters(parameters.begin()+5,parameters.end());
                  char *endptr;
                  unsigned long scheduleId = strtoul(parameters[0].c_str(),&endptr,10);
                  submitSchedule(scheduleId,parameters[2],scheduleParameters,vtInfo.clientPort);
  
                  if(!ldExists)
                  {
                    vtInfo.logicalDevice->start();
  
                    // increment1: ignore starting and stopping time, start the damn thing right now!
                    // in the future, the scheduler will claim, prepare and resume the scheduled Logical Devices.
                    // connect and send the claim and prepare messages            
                    vtInfo.clientPort->open();
                    // after receiving connected event, messages are sent            
                  }
                }
              }
            }
            else if(parameters[1] == string("SR"))
            {
            }
            else if(parameters[1] == string("SRG"))
            {
              // SCHEDULE <sch.nr>,SRG,<srg_name>,<starttime>,<stoptime>,
              //          <filter>,<antenna>,<power>
              // it's a virtual telescope, check the number of parameters
              if(parameters.size()!=8)
              {
                status.setValue(string("Error SCHEDULE,")+parameters[0]+string(",incorrect number of parameters"));
              }
              else
              {
                // Check if the thing has already been created
                LogicalDeviceMapIterT ldIt=m_logicalDeviceMap.find(parameters[2]);
                if(ldIt!=m_logicalDeviceMap.end())
                {
                  // copy parameters[3] to parameters[end] into a new vector
                  vector<string> scheduleParameters(parameters.begin()+3,parameters.end());
                  char *endptr;
                  unsigned long scheduleId = strtoul(parameters[0].c_str(),&endptr,10);
                  submitSchedule(scheduleId,parameters[2],scheduleParameters,ldIt->second.clientPort);
                }
                else
                {
                  status.setValue(string("Error SCHEDULE,")+parameters[0]+string(",SRG not found"));
                }
              }
            }
            else
            {
              status.setValue(string("Error SCHEDULE,")+parameters[0]+string(",unknown device type"));
            }
          }
        }
        // CANCEL <scheduleId>
        else if(command==string(LD_COMMAND_CANCEL))
        {
          if(parameters.size()!=1)
          {
            // return error in status property
            string statusString("Error ");
            statusString += commandString + string(",incorrect number of parameters");
            status.setValue(statusString);
          }
          else
          {
            char *endptr;
            unsigned long scheduleId = strtoul(parameters[0].c_str(),&endptr,10);
            LogicalDeviceScheduleIterT scheduleIt = m_logicalDeviceSchedule.find(scheduleId);
            if(scheduleIt != m_logicalDeviceSchedule.end())
            {
              // find the logical device of the schedule
              LogicalDeviceMapIterT ldIt=m_logicalDeviceMap.find(scheduleIt->second.deviceName);
              if(ldIt != m_logicalDeviceMap.end())
              {
                // check begin time, if not yet begun, cancel timers and remove schedule
                time_t timeNow = time(0);
                struct tm* utcTimeStruct = gmtime(&timeNow);
                time_t utcTime = mktime(utcTimeStruct);
                boost::posix_time::ptime curUTCtime   = boost::posix_time::from_time_t(utcTime);
                boost::posix_time::ptime startTime    = boost::posix_time::from_time_t(scheduleIt->second.startTime);
                if(startTime > curUTCtime)
                {
                  ldIt->second.clientPort->cancelTimer(scheduleIt->second.prepareTimerId);
                  ldIt->second.clientPort->cancelTimer(scheduleIt->second.startTimerId);
                  ldIt->second.clientPort->cancelTimer(scheduleIt->second.stopTimerId);
                  LOG_DEBUG(formatString("(%s) Schedule %d cancelled. timers %d, %d and %d also cancelled.",__func__,scheduleId,scheduleIt->second.prepareTimerId,scheduleIt->second.startTimerId,scheduleIt->second.stopTimerId));
                  m_logicalDeviceSchedule.erase(scheduleIt);
                }
                else
                {
                  string statusString("Error ");
                  statusString += commandString + string(",already started");
                  status.setValue(statusString);
                }
              }
              else
              {
                string statusString("Error ");
                statusString += commandString + string(",unknown Logical Device");
                status.setValue(statusString);
              }
            }
            else
            {
              string statusString("Error ");
              statusString += commandString + string(",unknown schedule");
              status.setValue(statusString);
            }
          } 
        }
        // MAINTENANCE <scheduleId>,<resource>,<fromTime>,<toTime>
        else if(command==string(LD_COMMAND_MAINTENANCE))
        {
          if(parameters.size()!=4)
          {
            // return error in status property
            string statusString("Error ");
            statusString += commandString + string(",incorrect number of parameters");
            status.setValue(statusString);
          }
          else
          {
            char *endptr;
            unsigned long scheduleId = strtoul(parameters[0].c_str(),&endptr,10);
            MaintenanceScheduleIterT it=m_maintenanceSchedule.find(scheduleId);
            if(it!=m_maintenanceSchedule.end())
            {
              // schedule exists already. cancel timers, remove from schedule
              m_timerPort.cancelTimer(it->second.startTimerId);
              m_timerPort.cancelTimer(it->second.stopTimerId);
              LOG_DEBUG(formatString("(%s) Maintenance schedule %d cancelled. timers %d and %d also cancelled.",__func__,scheduleId,it->second.startTimerId,it->second.stopTimerId));
              it->second.pMaintenanceProperty.reset();
              m_maintenanceSchedule.erase(it);
            }
            
            int rawStartTime = atoi(parameters[2].c_str()); // starttime
            int rawStopTime  = atoi(parameters[3].c_str()); // stoptime
            
            time_t timeNow = time(0);
            struct tm* utcTimeStruct = gmtime(&timeNow);
            time_t utcTime = mktime(utcTimeStruct);
            boost::posix_time::ptime curUTCtime   = boost::posix_time::from_time_t(utcTime);
            boost::posix_time::ptime startTime    = boost::posix_time::from_time_t(rawStartTime);
            boost::posix_time::ptime stopTime     = boost::posix_time::from_time_t(rawStopTime);
            
            MaintenanceScheduleInfoT scheduleInfo;
            scheduleInfo.resource = parameters[1];
            string propName(scheduleInfo.resource+string(SCOPESEPARATOR)+string("Maintenance")+string(PROPERTYSEPARATOR)+string(PROPNAME_STATUS));
            TPropertyInfo propInfo(propName.c_str(),LPT_UNSIGNED);
            scheduleInfo.pMaintenanceProperty.reset(new GCFExtProperty(propInfo));
            scheduleInfo.startTime = rawStartTime;
            scheduleInfo.stopTime  = rawStopTime;
            
            boost::posix_time::time_duration delay;
            delay = startTime - curUTCtime; 
            scheduleInfo.startTimerId = m_timerPort.setTimer(static_cast<long int>(delay.total_seconds())); // set the startTimer
            LOG_DEBUG(formatString("(%s) MaintenanceStartTimer %d created",__func__,scheduleInfo.startTimerId));
            delay = stopTime - curUTCtime; 
            scheduleInfo.stopTimerId = m_timerPort.setTimer(static_cast<long int>(delay.total_seconds())); // set the startTimer
            LOG_DEBUG(formatString("(%s) MaintenanceStopTimer %d created",__func__,scheduleInfo.stopTimerId));

            m_maintenanceSchedule[scheduleId] = scheduleInfo;
          } 
        }
        else
        {
          status.setValue(string("Error ")+command+string(",unknown command"));
        }
        
        m_properties.setValue(PROPNAME_STATUS,status);
        
      }
      else if (pPropAnswer->pValue->getType() == LPT_DOUBLE &&
               strstr(pPropAnswer->pPropName, SCOPE_PAC_LogicalDeviceScheduler_WaveFormGenerator) != 0 &&
               strstr(pPropAnswer->pPropName, PROPNAME_FREQUENCY) != 0 )
      {
        // wavegen frequency received
        double frequency(((GCFPVDouble*)pPropAnswer->pValue)->getValue());
        if(frequency!=m_WGfrequency)
        {
          if(m_WGfrequency == 0.0)
          {
            // frequency was 0, send enable
            sendWGenable();
          }
          m_WGfrequency = frequency;
          
          if(m_WGfrequency == 0.0)
          {
            // frequency is 0, send disable
            sendWGdisable();
          }
          else
          {
            sendWGsettings();
          }
        }
      }      
      else if (pPropAnswer->pValue->getType() == LPT_DOUBLE &&
               strstr(pPropAnswer->pPropName, SCOPE_PAC_LogicalDeviceScheduler_WaveFormGenerator) != 0 &&
               strstr(pPropAnswer->pPropName, PROPNAME_AMPLITUDE) != 0 )
      {
        // wavegen amplitude received
        unsigned int amplitude(((GCFPVUnsigned*)pPropAnswer->pValue)->getValue());
        if(amplitude!=m_WGamplitude)
        {
          m_WGamplitude = amplitude;
          sendWGsettings();
        }
      }      
      break;
    }  
    default:
      break;
  }  
}

void AVTLogicalDeviceScheduler::sendWGsettings()
{
  ABSWgsettingsEvent wgSettingsEvent;
  wgSettingsEvent.frequency = m_WGfrequency;
  wgSettingsEvent.amplitude = static_cast<unsigned char>(m_WGamplitude);
  
// m_beamServer.send(wgSettingsEvent);
  if(m_pBeamServer!=0)
  {
    m_pBeamServer->send(wgSettingsEvent);
  }
}

void AVTLogicalDeviceScheduler::sendWGenable()
{
  ABSWgenableEvent wgEnableEvent;
  
  if(m_pBeamServer!=0)
  {
    m_pBeamServer->send(wgEnableEvent);
  }
}

void AVTLogicalDeviceScheduler::sendWGdisable()
{
  ABSWgdisableEvent wgDisableEvent;
  
  if(m_pBeamServer!=0)
  {
    m_pBeamServer->send(wgDisableEvent);
  }
}

void AVTLogicalDeviceScheduler::getRequiredResources(list<TPropertyInfo>& requiredResources, int rack, int subrack, int board, int ap, int rcu)
{
  requiredResources.clear();

  char scopeString[300];
  
  {
    string propName(string(SCOPE_PIC)+string(PROPERTYSEPARATOR)+string(PROPNAME_STATUS));
    TPropertyInfo propInfo(propName.c_str(), LPT_UNSIGNED);
    requiredResources.push_back(propInfo);
  }  
  {
    string propName(string(SCOPE_PIC_Maintenance)+string(PROPERTYSEPARATOR)+string(PROPNAME_STATUS));
    TPropertyInfo propInfo(propName.c_str(), LPT_UNSIGNED);
    requiredResources.push_back(propInfo);
  }  
  {
    sprintf(scopeString,SCOPE_PIC_RackN,rack);
    string propName(string(scopeString)+string(PROPERTYSEPARATOR)+string(PROPNAME_STATUS));
    TPropertyInfo propInfo(propName.c_str(), LPT_UNSIGNED);
    requiredResources.push_back(propInfo);
  }  
  {
    sprintf(scopeString,SCOPE_PIC_RackN_Maintenance,rack);
    string propName(string(scopeString)+string(PROPERTYSEPARATOR)+string(PROPNAME_STATUS));
    TPropertyInfo propInfo(propName.c_str(), LPT_UNSIGNED);
    requiredResources.push_back(propInfo);
  }  
  {
    sprintf(scopeString,SCOPE_PIC_RackN_Alert,rack);
    string propName(string(scopeString)+string(PROPERTYSEPARATOR)+string(PROPNAME_STATUS));
    TPropertyInfo propInfo(propName.c_str(), LPT_UNSIGNED);
    requiredResources.push_back(propInfo);
  }  
  {
    sprintf(scopeString,SCOPE_PIC_RackN_SubRackN,rack,subrack);
    string propName(string(scopeString)+string(PROPERTYSEPARATOR)+string(PROPNAME_STATUS));
    TPropertyInfo propInfo(propName.c_str(), LPT_UNSIGNED);
    requiredResources.push_back(propInfo);
  }  
  {
    sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_Maintenance,rack,subrack);
    string propName(string(scopeString)+string(PROPERTYSEPARATOR)+string(PROPNAME_STATUS));
    TPropertyInfo propInfo(propName.c_str(), LPT_UNSIGNED);
    requiredResources.push_back(propInfo);
  }  
  {
    sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_Alert,rack,subrack);
    string propName(string(scopeString)+string(PROPERTYSEPARATOR)+string(PROPNAME_STATUS));
    TPropertyInfo propInfo(propName.c_str(), LPT_UNSIGNED);
    requiredResources.push_back(propInfo);
  }  
  {
    sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN,rack,subrack,board);
    string propName(string(scopeString)+string(PROPERTYSEPARATOR)+string(PROPNAME_STATUS));
    TPropertyInfo propInfo(propName.c_str(), LPT_UNSIGNED);
    requiredResources.push_back(propInfo);
  }  
  {
    sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_Maintenance,rack,subrack,board);
    string propName(string(scopeString)+string(PROPERTYSEPARATOR)+string(PROPNAME_STATUS));
    TPropertyInfo propInfo(propName.c_str(), LPT_UNSIGNED);
    requiredResources.push_back(propInfo);
  }  
  {
    sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_Alert,rack,subrack,board);
    string propName(string(scopeString)+string(PROPERTYSEPARATOR)+string(PROPNAME_STATUS));
    TPropertyInfo propInfo(propName.c_str(), LPT_UNSIGNED);
    requiredResources.push_back(propInfo);
  }  
  {
    sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_ETH,rack,subrack,board);
    string propName(string(scopeString)+string(PROPERTYSEPARATOR)+string(PROPNAME_STATUS));
    TPropertyInfo propInfo(propName.c_str(), LPT_UNSIGNED);
    requiredResources.push_back(propInfo);
  }  
  {
    sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_BP,rack,subrack,board);
    string propName(string(scopeString)+string(PROPERTYSEPARATOR)+string(PROPNAME_STATUS));
    TPropertyInfo propInfo(propName.c_str(), LPT_UNSIGNED);
    requiredResources.push_back(propInfo);
  }  
  {
    sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN,rack,subrack,board,ap);
    string propName(string(scopeString)+string(PROPERTYSEPARATOR)+string(PROPNAME_STATUS));
    TPropertyInfo propInfo(propName.c_str(), LPT_UNSIGNED);
    requiredResources.push_back(propInfo);
  }  
  {
    sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN,rack,subrack,board,ap,rcu);
    string propName(string(scopeString)+string(PROPERTYSEPARATOR)+string(PROPNAME_STATUS));
    TPropertyInfo propInfo(propName.c_str(), LPT_UNSIGNED);
    requiredResources.push_back(propInfo);
  }  
  {
    sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_Maintenance,rack,subrack,board,ap,rcu);
    string propName(string(scopeString)+string(PROPERTYSEPARATOR)+string(PROPNAME_STATUS));
    TPropertyInfo propInfo(propName.c_str(), LPT_UNSIGNED);
    requiredResources.push_back(propInfo);
  }
}
