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

#include <GCF/GCF_Defines.h>
#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_PVUnsigned.h>

#include "../../../APLCommon/src/APL_Defines.h"
#include "../../../APLCommon/src/APLInterTaskPort.h"
#include "AVTLogicalDeviceScheduler.h"
#include "LogicalDevice_Protocol.ph"
#include "AVTDefines.h"
#include "AVTStationBeamformer.h"
#include "AVTVirtualTelescope.h"
#include "AVTStationReceptor.h"
#include "AVTStationReceptorGroup.h"
#include "AVTUtilities.h"

#undef PACKAGE
#undef VERSION
#define DECLARE_SIGNAL_NAMES
#include "ABS_Protocol.ph"

#include "../test/PropertyDefines.h"

using namespace LOFAR;
using namespace AVT;
using namespace std;
using namespace boost;

string AVTLogicalDeviceScheduler::m_schedulerTaskName(LDSNAME);
string g_bsName(BSNAME);


AVTLogicalDeviceScheduler::AVTLogicalDeviceScheduler() :
  GCFTask((State)&AVTLogicalDeviceScheduler::initial_state,m_schedulerTaskName),
  AVTPropertySetAnswerHandlerInterface(),
  m_propertySetAnswer(*this),
  m_properties(propertySetLDS,&m_propertySetAnswer),
  m_apcLDS("ApcLogicalDeviceScheduler", "PAC_LogicalDeviceScheduler", &m_propertySetAnswer),
  m_initialized(false),
  m_pBeamServer(0),
  m_WGfrequency(0.0),
  m_WGamplitude(0),
  m_WGsamplePeriod(0),
  m_logicalDeviceMap(),
  m_logicalDeviceSchedule(),
  m_resourceManager(AVTResourceManager::instance())
{
  registerProtocol(LOGICALDEVICE_PROTOCOL, LOGICALDEVICE_PROTOCOL_signalnames);
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::AVTLogicalDeviceScheduler(%s)",getName().c_str()));
  m_properties.load();
}


AVTLogicalDeviceScheduler::~AVTLogicalDeviceScheduler()
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::~AVTLogicalDeviceScheduler(%s)",getName().c_str()));
  m_apcLDS.unload();
}

shared_ptr<AVTStationReceptor> AVTLogicalDeviceScheduler::addReceptor(string srName,const TPropertySet& propertySet)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("%s(%s)(%s)",__func__,getName().c_str(),srName.c_str()));
  // create receptor logical device
  LogicalDeviceInfoT      srInfo;
  string srApcName(SRAPCNAME);
  shared_ptr<AVTStationReceptor> sr(new AVTStationReceptor(srName,propertySet,srApcName,string("PAC_")+srName));
  srInfo.logicalDevice = sr;
  srInfo.clientPort.reset(new APLInterTaskPort(*sr,*this, sr->getServerPortName(), GCFPortInterface::SAP, LOGICALDEVICE_PROTOCOL));
  sr->setClientInterTaskPort(srInfo.clientPort.get());
  
  m_logicalDeviceMap[srName]=srInfo;
  sr->start();
  srInfo.clientPort->open();
  
  return sr;
}

void AVTLogicalDeviceScheduler::addReceptorGroup(string srgName,const TPropertySet& propertySet, vector<shared_ptr<AVTStationReceptor> >& receptors)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("%s(%s)(%s)",__func__,getName().c_str(),srgName.c_str()));
  LogicalDeviceInfoT srgInfo;
  string srgApcName(SRGAPCNAME);
  
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
  shared_ptr<AVTStationReceptorGroup> srg(new AVTStationReceptorGroup(srgName,propertySet,srgApcName,string("PAC_")+srgName,receptors));
  srgInfo.logicalDevice = srg;
  srgInfo.clientPort.reset(new APLInterTaskPort(*srg,*this, srg->getServerPortName(), GCFPortInterface::SAP, LOGICALDEVICE_PROTOCOL));
  srg->setClientInterTaskPort(srgInfo.clientPort.get());

  m_logicalDeviceMap[srgName]=srgInfo;
  srg->start();
  srgInfo.clientPort->open();
}

bool AVTLogicalDeviceScheduler::isInitialized()
{
  return m_initialized;
}

AVTLogicalDeviceScheduler::LogicalDeviceMapIterT AVTLogicalDeviceScheduler::findLogicalDevice(const unsigned long scheduleId)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("%s(%s)(scheduleId=%d)",__func__,getName().c_str(),scheduleId));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("%s(%s)",__func__,getName().c_str()));
  LogicalDeviceMapIterT it;
  for(it=m_logicalDeviceMap.begin();it!=m_logicalDeviceMap.end()&&(&port!=it->second.clientPort.get());++it);
  return it;
}

AVTLogicalDeviceScheduler::LogicalDeviceScheduleIterT AVTLogicalDeviceScheduler::findSchedule(const string& deviceName,LogicalDeviceScheduleIterT beginIt)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("%s(%s)(ld=%s)",__func__,getName().c_str(),deviceName.c_str()));
  LogicalDeviceScheduleIterT scheduleIt;
  for(scheduleIt=beginIt;scheduleIt!=m_logicalDeviceSchedule.end()&&(deviceName!=scheduleIt->second.deviceName);++scheduleIt);
  return scheduleIt;
}

bool AVTLogicalDeviceScheduler::submitSchedule(const unsigned long scheduleId,const string& deviceName, const vector<string> scheduleParameters, boost::shared_ptr<APLInterTaskPort> clientPort)
{
  bool result=false;

  int rawStartTime = atoi(scheduleParameters[0].c_str()); // starttime
  int rawStopTime  = atoi(scheduleParameters[1].c_str()); // stoptime
  
  boost::posix_time::ptime currentTime  = boost::posix_time::second_clock::universal_time();
  boost::posix_time::ptime startTime    = boost::posix_time::from_time_t(rawStartTime);
  boost::posix_time::ptime stopTime     = boost::posix_time::from_time_t(rawStopTime);
  boost::posix_time::ptime prepareTime  = startTime - boost::posix_time::seconds(PREPARE_TIME); //  - 10 seconds

  // check if the schedule already exists
  LogicalDeviceScheduleIterT schIt = m_logicalDeviceSchedule.find(scheduleId);
  if(schIt != m_logicalDeviceSchedule.end())
  {
    // it exists.
    if(prepareTime < currentTime)
    {
      // can't change prepare time because it has passed
    }
    else
    {
      // update the preparation parameters
      schIt->second.parameters = scheduleParameters;
      // cancel current prepare timer and create a new one
      clientPort->cancelTimer(schIt->second.prepareTimerId);
      
      boost::posix_time::time_duration delay;
      delay = prepareTime - currentTime; 
      schIt->second.prepareTimerId = clientPort->setTimer(static_cast<long int>(delay.total_seconds())); // set the prepareTimer
    }
    if(startTime < currentTime)
    {
      // can't change start time because it has passed
    }
    else
    {
      schIt->second.startTime = rawStartTime;
      // cancel current start timer and create a new one
      clientPort->cancelTimer(schIt->second.startTimerId);
      
      boost::posix_time::time_duration delay;
      delay = startTime - currentTime; 
      schIt->second.startTimerId = clientPort->setTimer(static_cast<long int>(delay.total_seconds())); // set the startTimer
    }
    if(stopTime < currentTime)
    {
      // can't change stop time because it has passed
    }
    else
    {
      schIt->second.stopTime = rawStopTime;
      // cancel current stop timer and create a new one
      clientPort->cancelTimer(schIt->second.stopTimerId);
      
      boost::posix_time::time_duration delay;
      delay = stopTime - currentTime; 
      schIt->second.stopTimerId = clientPort->setTimer(static_cast<long int>(delay.total_seconds())); // set the stopTimer
    }
  }
  else
  {
    // create the schedule entry
    LogicalDeviceScheduleInfoT vtSchedule;
    
    vtSchedule.deviceName = deviceName;
    vtSchedule.parameters = scheduleParameters;
    vtSchedule.startTime = rawStartTime;
    vtSchedule.stopTime = rawStopTime;
  
    // create the timers
    boost::posix_time::time_duration delay;
    delay = prepareTime - currentTime; 
    vtSchedule.prepareTimerId = clientPort->setTimer(static_cast<long int>(delay.total_seconds())); // set the prepareTimer

    delay = startTime - currentTime; 
    vtSchedule.startTimerId = clientPort->setTimer(static_cast<long int>(delay.total_seconds())); // set the startTimer

    delay = stopTime - currentTime; 
    vtSchedule.stopTimerId = clientPort->setTimer(static_cast<long int>(delay.total_seconds())); // set the stopTimer
  
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
  return isStopTimer;
}

GCFEvent::TResult AVTLogicalDeviceScheduler::initial_state(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("%s(%s) (%s)",__func__,getName().c_str(),evtstr(event)));

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

      receptors.push_back(addReceptor(string("RCU1"),propertySetSR1));
      receptors.push_back(addReceptor(string("RCU2"),propertySetSR2));
      receptors.push_back(addReceptor(string("RCU3"),propertySetSR3));
      receptors.push_back(addReceptor(string("RCU4"),propertySetSR4));
      receptors.push_back(addReceptor(string("RCU5"),propertySetSR5));
      receptors.push_back(addReceptor(string("RCU6"),propertySetSR6));
      receptors.push_back(addReceptor(string("RCU7"),propertySetSR7));
      receptors.push_back(addReceptor(string("RCU8"),propertySetSR8));

      // create receptor groups
      vector<shared_ptr<AVTStationReceptor> > receptorsSRG1;
      receptorsSRG1.push_back(receptors[0]);
      receptorsSRG1.push_back(receptors[1]);
      receptorsSRG1.push_back(receptors[2]);
      receptorsSRG1.push_back(receptors[3]);
      receptorsSRG1.push_back(receptors[4]);
      receptorsSRG1.push_back(receptors[5]);
      receptorsSRG1.push_back(receptors[6]);
      receptorsSRG1.push_back(receptors[7]);
      vector<shared_ptr<AVTStationReceptor> > receptorsSRG2(receptors.begin(),receptors.begin()+4);
      vector<shared_ptr<AVTStationReceptor> > receptorsSRG3;
      vector<shared_ptr<AVTStationReceptor> > receptorsSRG4(receptors.begin()+5,receptors.begin()+8);
      receptorsSRG3.push_back(receptors[1]);
      receptorsSRG3.push_back(receptors[3]);
      receptorsSRG3.push_back(receptors[5]);
      receptorsSRG3.push_back(receptors[7]);

      addReceptorGroup(string("SRG1"),propertySetSRG1,receptorsSRG1);
      addReceptorGroup(string("SRG2"),propertySetSRG2,receptorsSRG2);
      addReceptorGroup(string("SRG3"),propertySetSRG3,receptorsSRG3);
      addReceptorGroup(string("SRG4"),propertySetSRG4,receptorsSRG4);

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
        LogicalDeviceScheduleIterT scheduleIt = findSchedule(it->first,m_logicalDeviceSchedule.begin());
        if(scheduleIt != m_logicalDeviceSchedule.end())
        {
          // send release message because no more schedules exist for this logical device
          LOGICALDEVICEReleaseEvent releaseEvent;
          port.send(releaseEvent);
        }
      }
      break;
    }

    case LOGICALDEVICE_RELEASED:
    {
      
      LogicalDeviceMapIterT it = findClientPort(port);
      if(it!=m_logicalDeviceMap.end())
      {
        // remove the object from the map. The Port instance and children Logical Device instances
        // are destroyed too because they are smart pointers.
        m_logicalDeviceMap.erase(it);
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
        if(checkPrepareTimer(it->first,timerEvent.id))
        {
          // this is a prepare timer for the schedule of a logical device. claim the device
          LOGICALDEVICEClaimEvent claimEvent;
          port.send(claimEvent);
          // when the device (and all it's children) is claimed, the prepare message is sent
          // automatically
        }
        else if(checkStartTimer(it->first,timerEvent.id))
        {
          // this is a start timer for the schedule of a logical device. resume the device
          LOGICALDEVICEResumeEvent resumeEvent;
          port.send(resumeEvent);
        }
        else if(checkStopTimer(it->first,timerEvent.id))
        {
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
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("%s(%s), default",__func__,getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

void AVTLogicalDeviceScheduler::handlePropertySetAnswer(GCFEvent& answer)
{
//  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("%s(%s) (%s)",__func__,getName().c_str(),evtstr(answer)));

  switch(answer.signal)
  {
    case F_MYPLOADED:
    {
      // property set loaded, now load apc
      m_apcLDS.load(false);
      break;
    }
    
    case F_APCLOADED:
    {
      m_initialized=true;
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&answer);
      assert(pPropAnswer);
      if ((pPropAnswer->pValue->getType() == GCFPValue::LPT_STRING) &&
          (strstr(pPropAnswer->pPropName, PROPERTY_LDS_COMMAND) != 0))
      {
        // command received
        string commandString(((GCFPVString*)pPropAnswer->pValue)->getValue());
        
        GCFPVString status("OK");

        LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("Command received: (%s)",commandString.c_str()));
        
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
                    string sbfApcName(SBFAPCNAME);
                    string bsName(BSNAME);
                    boost::shared_ptr<AVTStationBeamformer> sbf(new AVTStationBeamformer(parameters[3],primaryPropertySetSBF,sbfApcName,string("PAC_")+parameters[2]+string("_")+parameters[3],bsName));
                    sbfInfo.logicalDevice=sbf;
                    if(m_pBeamServer==0)
                    {
                      m_pBeamServer=&sbf->getBeamServerPort();
                    }

                    // create VT
                    string vtApcName(VTAPCNAME);
                    boost::shared_ptr<AVTVirtualTelescope> vt(new AVTVirtualTelescope(parameters[2],primaryPropertySetVT,vtApcName,string("PAC_")+parameters[2],*(sbf.get()),*(srg.get())));
                    vtInfo.logicalDevice=vt;
                    vtInfo.clientPort.reset(new APLInterTaskPort(*vt.get(),*this, vt->getServerPortName(), GCFPortInterface::SAP, LOGICALDEVICE_PROTOCOL));
                    vt->setClientInterTaskPort(vtInfo.clientPort.get());
                    vtInfo.children[parameters[3]]=sbfInfo;
                    vtInfo.children[parameters[4]]=srgInfo;

                    // put the new logical device in the map
                    m_logicalDeviceMap[parameters[2]]=vtInfo;
                  }
                  else
                  {
                    status.setValue(string("Error SCHEDULE,")+parameters[0]+string(",SRG not found"));
                  }
                }

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
          if(parameters.size()==1)
          {
            char *endptr;
            unsigned long scheduleId = strtoul(parameters[0].c_str(),&endptr,10);
            LogicalDeviceMapIterT it=findLogicalDevice(scheduleId);
            if(it!=m_logicalDeviceMap.end())
            {
              // doe ietsjepietsjeflap
            }
          } 
        }
        else
        {
          status.setValue(string("Error ")+command+string(",unknown command"));
        }
        
        m_properties.setValue(PROPERTY_LDS_STATUS,status);
        
      }
      else if ((pPropAnswer->pValue->getType() == GCFPValue::LPT_DOUBLE) &&
               (strstr(pPropAnswer->pPropName, PROPERTYNAME_WAVEGENFREQUENCY) != 0))
      {
        // wavegen frequency received
        double frequency(((GCFPVDouble*)pPropAnswer->pValue)->getValue());
        if(frequency!=m_WGfrequency)
        {
          m_WGfrequency = frequency;
          sendWGsettings();
        }
      }      
      else if ((pPropAnswer->pValue->getType() == GCFPValue::LPT_UNSIGNED) &&
               (strstr(pPropAnswer->pPropName, PROPERTYNAME_WAVEGENAMPLITUDE) != 0))
      {
        // wavegen amplitude received
        unsigned int amplitude(((GCFPVUnsigned*)pPropAnswer->pValue)->getValue());
        if(amplitude!=m_WGamplitude)
        {
          m_WGamplitude = amplitude;
          sendWGsettings();
        }
      }      
      else if ((pPropAnswer->pValue->getType() == GCFPValue::LPT_UNSIGNED) &&
               (strstr(pPropAnswer->pPropName, PROPERTYNAME_WAVEGENSAMPLEPERIOD) != 0))
      {
        // wavegen samplePeriod received
        unsigned int samplePeriod(((GCFPVUnsigned*)pPropAnswer->pValue)->getValue());
        if(samplePeriod!=m_WGsamplePeriod)
        {
          m_WGsamplePeriod = samplePeriod;
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
  wgSettingsEvent.sample_period = static_cast<unsigned char>(m_WGsamplePeriod);
  
// m_beamServer.send(wgSettingsEvent);
  if(m_pBeamServer!=0)
  {
    m_pBeamServer->send(wgSettingsEvent);
  }
}
