//#  LogicalDevice.cc: Implementation of the Virtual Telescope task
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
#include <boost/shared_array.hpp>
#include <Common/LofarLogger.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVInteger.h>
#include <GCF/GCF_PVDynArr.h>
#include <GCF/ParameterSet.h>
#include "APLCommon/APL_Defines.h"
#include "APLCommon/APLUtilities.h"
#include "APLCommon/LogicalDevice.h"

#define DECLARE_SIGNAL_NAMES
#include "APLCommon/LogicalDevice_Protocol.ph"
#include "APLCommon/StartDaemon_Protocol.ph"

using namespace LOFAR::ACC;

namespace LOFAR
{
namespace APLCommon
{

const string LogicalDevice::LD_STATE_STRING_INITIAL     = string("Initial");
const string LogicalDevice::LD_STATE_STRING_IDLE        = string("Idle");
const string LogicalDevice::LD_STATE_STRING_CLAIMING    = string("Claiming");
const string LogicalDevice::LD_STATE_STRING_CLAIMED     = string("Claimed");
const string LogicalDevice::LD_STATE_STRING_PREPARING   = string("Preparing");
const string LogicalDevice::LD_STATE_STRING_SUSPENDED   = string("Suspended");
const string LogicalDevice::LD_STATE_STRING_ACTIVE      = string("Active");
const string LogicalDevice::LD_STATE_STRING_RELEASING   = string("Releasing");
const string LogicalDevice::LD_STATE_STRING_RELEASED    = string("Released");

const string LogicalDevice::LD_PROPNAME_COMMAND         = string("command");
const string LogicalDevice::LD_PROPNAME_STATUS          = string("status");
const string LogicalDevice::LD_PROPNAME_CHILDREFS       = string("childrefs");

const string LogicalDevice::LD_COMMAND_SCHEDULE         = string("SCHEDULE");
const string LogicalDevice::LD_COMMAND_CANCELSCHEDULE   = string("CANCELSCHEDULE");
const string LogicalDevice::LD_COMMAND_CLAIM            = string("CLAIM");
const string LogicalDevice::LD_COMMAND_PREPARE          = string("PREPARE");
const string LogicalDevice::LD_COMMAND_RESUME           = string("RESUME");
const string LogicalDevice::LD_COMMAND_SUSPEND          = string("SUSPEND");
const string LogicalDevice::LD_COMMAND_RELEASE          = string("RELEASE");

INIT_TRACER_CONTEXT(LogicalDevice,LOFARLOGGER_PACKAGE);

LogicalDevice::LogicalDevice(const string& taskName, const string& parameterFile) throw (APLCommon::ParameterFileNotFoundException, APLCommon::ParameterNotFoundException) :
  ::GCFTask((State)&LogicalDevice::initial_state,taskName),
  PropertySetAnswerHandlerInterface(),
  m_propertySetAnswer(*this),
  m_propertySet(),
  m_parameterSet(),
  m_serverPortName(string("server")),
  m_parentPort(),
  m_serverPort(*this, m_serverPortName, ::GCFPortInterface::MSPP, LOGICALDEVICE_PROTOCOL),
  m_childPorts(),
  m_connectedChildPorts(),
  m_childStartDaemonPorts(),
  m_apcLoaded(false),
  m_logicalDeviceState(LOGICALDEVICE_STATE_IDLE),
  m_prepareTimerId(0),
  m_startTimerId(0),
  m_stopTimerId(0),
  m_retrySendTimerId(0),
  m_eventBuffer()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
#ifdef USE_TCPPORT_INSTEADOF_PVSSPORT
  LOG_WARN("Using GCFTCPPort in stead of GCFPVSSPort");
#endif

  registerProtocol(LOGICALDEVICE_PROTOCOL, LOGICALDEVICE_PROTOCOL_signalnames);

  string host("");
  string psName("");
  string psType("");
  
  try
  {
    m_parameterSet.adoptFile(_getShareLocation() + string("share/") + parameterFile);
  }
  catch(Exception& e)
  {
    THROW(APLCommon::ParameterFileNotFoundException,e.message());
  }
  
  try
  {
    psName = m_parameterSet.getString("propertysetName");
    psType = m_parameterSet.getString("propertysetType");
  }
  catch(Exception& e)
  {
    THROW(APLCommon::ParameterNotFoundException,e.message());
  }
  try
  {
    host   = m_parameterSet.getString("startDaemonHost");
    psName = host + string(":") + psName;
  }
  catch(Exception& e)
  {
  }
  
  m_propertySet = boost::shared_ptr<GCFMyPropertySet>(new GCFMyPropertySet(
      psName.c_str(),
      psType.c_str(),
      PS_CAT_TEMPORARY,
      &m_propertySetAnswer));
  m_propertySet->enable();
}


LogicalDevice::~LogicalDevice()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  m_propertySet->disable();
}

string& LogicalDevice::getServerPortName()
{
  return m_serverPortName;
}

void LogicalDevice::_addChildPort(TPortSharedPtr childPort)
{
  m_childPorts.push_back(childPort);
}

bool LogicalDevice::isPrepared(vector<string>& /*parameters*/)
{
  return false;
}

LogicalDevice::TLogicalDeviceState LogicalDevice::getLogicalDeviceState() const
{
  return m_logicalDeviceState;
}

void LogicalDevice::handlePropertySetAnswer(::GCFEvent& answer)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(answer)).c_str());
  switch(answer.signal)
  {
    case F_MYPS_ENABLED:
    {
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
      if(strstr(pPropAnswer->pScope, m_parameterSet.getString("propertysetName").c_str()) != 0)
      {
        if(pPropAnswer->result == GCF_NO_ERROR)
        {
          // property set loaded, now load apc?
        }
        else
        {
          LOG_ERROR(formatString("%s : PropertySet %s NOT ENABLED",getName().c_str(),pPropAnswer->pScope));
        }
      }
      else
      {
        concrete_handlePropertySetAnswer(answer);
      }
      break;
    }
    
    case F_PS_CONFIGURED:
    {
      GCFConfAnswerEvent* pConfAnswer=static_cast<GCFConfAnswerEvent*>(&answer);
      if(strstr(pConfAnswer->pApcName, m_parameterSet.getString("propertysetName").c_str()) != 0)
      {
        if(pConfAnswer->result == GCF_NO_ERROR)
        {
          LOG_DEBUG(formatString("%s : apc %s Loaded",getName().c_str(),pConfAnswer->pApcName));
          //apcLoaded();
        }
        else
        {
          LOG_ERROR(formatString("%s : apc %s NOT LOADED",getName().c_str(),pConfAnswer->pApcName));
        }
      }
      else
      {
        concrete_handlePropertySetAnswer(answer);
      }
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);
      if(strstr(pPropAnswer->pPropName, m_parameterSet.getString("propertysetName").c_str()) != 0)
      {
        // it is my own propertyset
        if((pPropAnswer->pValue->getType() == LPT_STRING) &&
           (strstr(pPropAnswer->pPropName, LD_PROPNAME_COMMAND.c_str()) != 0))
        {
          // command received
          string commandString(((GCFPVString*)pPropAnswer->pValue)->getValue());
          vector<string> parameters;
          string command;
          APLUtilities::decodeCommand(commandString,command,parameters);
          
          // SCHEDULE <fileName>
          if(command==string(LD_COMMAND_SCHEDULE))
          {
            if(parameters.size()==1)
            {
              m_parameterSet.adoptFile(_getShareLocation() + string("share/") + parameters[0]);
              _schedule();
            }
            else
            {
              TLDResult result = LD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS;
              m_propertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
            }
          }
          // CANCELSCHEDULE
          else if(command==string(LD_COMMAND_CANCELSCHEDULE))
          {
            if(parameters.size()==0)
            {
              _cancelSchedule();
            }
            else
            {
              TLDResult result = LD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS;
              m_propertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
            }
          }
          // CLAIM
          else if(command==string(LD_COMMAND_CLAIM))
          {
            if(parameters.size()==0)
            {
              _claim();
            }
            else
            {
              TLDResult result = LD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS;
              m_propertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
            }
          }
          // PREPARE
          else if(command==string(LD_COMMAND_PREPARE))
          {
            if(parameters.size()==0)
            {
              _prepare();
            }
            else
            {
              TLDResult result = LD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS;
              m_propertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
            }
          }
          // RESUME
          else if(command==string(LD_COMMAND_RESUME))
          {
            if(parameters.size()==0)
            {
              _resume();
            }
            else
            {
              TLDResult result = LD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS;
              m_propertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
            }
          }
          // SUSPEND
          else if(command==string(LD_COMMAND_SUSPEND))
          {
            if(parameters.size()==0)
            {
              _suspend();
            }
            else
            {
              TLDResult result = LD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS;
              m_propertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
            }
          }
          // RELEASE
          else if(command==string(LD_COMMAND_RELEASE))
          {
            if(parameters.size()==0)
            {
              _release();
            }
            else
            {
              TLDResult result = LD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS;
              m_propertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
            }
          }
          else
          {
            TLDResult result = LD_RESULT_UNKNOWN_COMMAND;
            m_propertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
          }
        }
      }
      else
      {
        concrete_handlePropertySetAnswer(answer);
      }
      break;
    }  

    default:
      concrete_handlePropertySetAnswer(answer);
      break;
  }  
}

time_t LogicalDevice::_decodeTimeParameter(const string& timeStr) const
{
  // specified times are in UTC, seconds since 1-1-1970
  time_t returnTime=APLUtilities::getUTCtime();
  string::size_type plusPos = timeStr.find('+');
  if(plusPos != string::npos)
  {
    returnTime += atoi(timeStr.substr(plusPos+1).c_str());
  }
  else
  {
    returnTime = atoi(timeStr.c_str());
  }
  return returnTime;
}

void LogicalDevice::_schedule()
{
  //
  // set timers
  // specified times are in UTC, seconds since 1-1-1970
  time_t timeNow = APLUtilities::getUTCtime();
  time_t prepareTime = _decodeTimeParameter(m_parameterSet.getString("prepareTime"));
  time_t startTime   = _decodeTimeParameter(m_parameterSet.getString("startTime"));
  time_t stopTime    = _decodeTimeParameter(m_parameterSet.getString("stopTime"));
  
  m_prepareTimerId = m_serverPort.setTimer(prepareTime - timeNow);
  m_startTimerId = m_serverPort.setTimer(startTime - timeNow);
  m_stopTimerId = m_serverPort.setTimer(stopTime - timeNow);

  _sendScheduleToClients();
}

void LogicalDevice::_cancelSchedule()
{
  m_serverPort.cancelTimer(m_prepareTimerId);
  m_serverPort.cancelTimer(m_startTimerId);
  m_serverPort.cancelTimer(m_stopTimerId);
  
  // propagate to childs
  LOGICALDEVICECancelscheduleEvent cancelEvent;
  _sendToAllChilds(cancelEvent);
  
  _suspend();
}

void LogicalDevice::_claim()
{
  LOGICALDEVICEClaimEvent claimEvent;
  dispatch(claimEvent,m_parentPort);
}

void LogicalDevice::_prepare()
{
  LOGICALDEVICEPrepareEvent prepareEvent;
  dispatch(prepareEvent,m_parentPort);
}

void LogicalDevice::_resume()
{
  LOGICALDEVICEResumeEvent resumeEvent;
  dispatch(resumeEvent,m_parentPort);
}

void LogicalDevice::_suspend()
{
  LOGICALDEVICESuspendEvent suspendEvent;
  dispatch(suspendEvent,m_parentPort);
}

void LogicalDevice::_release()
{
  LOGICALDEVICEReleaseEvent releaseEvent;
  dispatch(releaseEvent,m_parentPort);
}

bool LogicalDevice::_isParentPort(::GCFPortInterface& port)
{
  return (&port == &m_parentPort); // comparing two pointers. yuck?
}
   
bool LogicalDevice::_isServerPort(::GCFPortInterface& port)
{
  return (&port == &m_serverPort); // comparing two pointers. yuck?
}
   
bool LogicalDevice::_isChildPort(::GCFPortInterface& port)
{
  bool found=false;
  TPortVector::iterator it=m_childPorts.begin();
  while(!found && it != m_childPorts.end())
  {
    found = (&port == (*it).get()); // comparing two pointers. yuck?
    ++it;
  }
  return found;
}

bool LogicalDevice::_isChildStartDaemonPort(::GCFPortInterface& port, string& startDaemonKey)
{
  bool found=false;
  TPortMap::iterator it=m_childStartDaemonPorts.begin();
  while(!found && it != m_childStartDaemonPorts.end())
  {
    found = (&port == it->second.get()); // comparing two pointers. yuck?
    if(found)
    {
      startDaemonKey = it->first;
    }
    ++it;
  }
  return found;
}

void LogicalDevice::_sendToAllChilds(::GCFEvent& event)
{
  // send to all childs
  TPortMap::iterator it=m_connectedChildPorts.begin();
  while(it != m_connectedChildPorts.end())
  {
    try
    {
      it->second->send(event);
    }
    catch(Exception& e)
    {
      LOG_FATAL(formatString("(%s) Fatal error while sending message to child %s",e.message().c_str(),it->first.c_str()));
    }
    ++it;
  }
}

void LogicalDevice::_disconnectedHandler(::GCFPortInterface& port)
{
  port.close();
  if(_isServerPort(port))
  {
  }
  else if(_isChildPort(port))
  {
    concreteChildDisconnected(port);
  }
  else if(_isParentPort(port))
  {
    concreteParentDisconnected(port);
  }
}

bool LogicalDevice::_isAPCLoaded() const
{
  return m_apcLoaded;
}

void LogicalDevice::_apcLoaded()
{
  m_apcLoaded=true;
}

void LogicalDevice::_doStateTransition(const TLogicalDeviceState& newState)
{
  switch(newState)
  {
    case LOGICALDEVICE_STATE_NOSTATE:
      // no transition
      break;
    case LOGICALDEVICE_STATE_IDLE:
      TRAN(LogicalDevice::idle_state);
      break;
    case LOGICALDEVICE_STATE_CLAIMING:
      TRAN(LogicalDevice::claiming_state);
      break;
    case LOGICALDEVICE_STATE_CLAIMED:
      TRAN(LogicalDevice::claimed_state);
      break;
    case LOGICALDEVICE_STATE_PREPARING:
      TRAN(LogicalDevice::preparing_state);
      break;
    case LOGICALDEVICE_STATE_SUSPENDED:
      TRAN(LogicalDevice::suspended_state);
      break;
    case LOGICALDEVICE_STATE_ACTIVE:
      TRAN(LogicalDevice::active_state);
      break;
    case LOGICALDEVICE_STATE_RELEASING:
      TRAN(LogicalDevice::releasing_state);
      break;
    case LOGICALDEVICE_STATE_RELEASED:
      break;
    default:
      // no transition
      break;
  }
}

void LogicalDevice::_handleTimers(::GCFEvent& event, ::GCFPortInterface& port)
{
  if(event.signal == F_TIMER)
  {
    GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
    if(timerEvent.id == m_prepareTimerId)
    {
      port.cancelTimer(timerEvent.id);
      LOG_DEBUG(formatString("(%s) PrepareTimer %d triggered and cancelled",__func__,timerEvent.id));
      // this is a prepare timer for the schedule of a logical device. claim the device
      LOGICALDEVICEClaimEvent claimEvent;
      port.send(claimEvent);
      // when the device (and all it's children) is claimed, the prepare message is sent
      // automatically
    }
    else if(timerEvent.id == m_startTimerId)
    {
      port.cancelTimer(timerEvent.id);
      LOG_DEBUG(formatString("(%s) StartTimer %d triggered and cancelled",__func__,timerEvent.id));
      // this is a start timer for the schedule of a logical device. resume the device
      LOGICALDEVICEResumeEvent resumeEvent;
      port.send(resumeEvent);
    }
    else if(timerEvent.id == m_stopTimerId)
    {
      port.cancelTimer(timerEvent.id);
      LOG_DEBUG(formatString("(%s) StopTimer %d triggered and cancelled",__func__,timerEvent.id));
      // this is a stop timer for the schedule of a logical device. suspend the device
      LOGICALDEVICESuspendEvent suspendEvent;
      port.send(suspendEvent);
    }
    else if(timerEvent.id == m_retrySendTimerId)
    {
      int retryTimeout = 1*60*60; // retry sending buffered events for 1 hour
      int retryPeriod = 30; // retry sending buffered events every 30 seconds
      GCF::ParameterSet* pParamSet = GCF::ParameterSet::instance();
      try
      {
        retryTimeout = pParamSet->getInt("retryTimeout");
        retryPeriod  = pParamSet->getInt("retryPeriod");
      } 
      catch(Exception& e)
      {
      }

      // loop through the buffered events and try to send each one.
      TEventBufferVector::iterator it = m_eventBuffer.begin();
      while(it != m_eventBuffer.end())
      {
        ssize_t sentBytes = it->port->send(*(it->event));
        time_t timeNow = time(0);
        
        if((sentBytes == 0 && timeNow - it->entryTime > retryPeriod) || sentBytes != 0)
        {
          // events are removed from the buffer if:
          // a. the event was sent or
          // b. the event was not sent AND it is longer than 1 hour in the buffer
          if(sentBytes != 0)
          {
            LOG_INFO(formatString("Buffered event successfully sent to %s:%s",it->port->getTask()->getName().c_str(),it->port->getName().c_str()));
          }
          else
          {
            LOG_FATAL(formatString("Unable to send event to %s:%s",it->port->getTask()->getName().c_str(),it->port->getName().c_str()));
          }
          it = m_eventBuffer.erase(it);  // erase() returns an iterator that points to the next item
        }
        else 
        {
          ++it;
        }
      }

      // keep on polling
      m_retrySendTimerId = m_serverPort.setTimer(static_cast<long int>(retryPeriod));
    }
  }
}

vector<string> LogicalDevice::_getChildKeys()
{
  string childs;
  vector<string> childKeys;
  try
  {
    childs = m_parameterSet.getString("childs");
  }
  catch(Exception& e)
  {
  }
  char* pch;
  boost::shared_array<char> childsCopy(new char[childs.length()+1]);
  strcpy(childsCopy.get(),childs.c_str());
  pch = strtok (childsCopy.get(),",");
  while (pch != NULL)
  {
    childKeys.push_back(string(pch));
    pch = strtok (NULL, ",");
  }
  return childKeys;
}

// Send the event to the port. If it fails, the event is added to a buffer
// The logical device periodically retries to send the events in the buffer
void LogicalDevice::_sendEvent(::GCFEvent& event, ::GCFPortInterface& port)
{
  ssize_t sentBytes = port.send(event);
  if(sentBytes == 0)
  {
    // add to buffer and keep retrying until it succeeds
    TBufferedEventInfo bufferedEvent(time(0),&port,&event);
    m_eventBuffer.push_back(bufferedEvent);
  }
}

void LogicalDevice::_sendScheduleToClients()
{
  if(m_connectedChildPorts.empty())
  {
    // no childs available: send schedule to startdaemons
    TPortMap::iterator it = m_childStartDaemonPorts.begin();
    while(it != m_childStartDaemonPorts.end())
    {
      try
      {
        // extract the parameterset for the child
        string startDaemonKey = it->first;
        TPortSharedPtr startDaemonPort = it->second;
        ACC::ParameterSet psSubset = m_parameterSet.makeSubset(startDaemonKey + string("."));
        string parameterFileName = startDaemonKey+string(".ps"); 
        string remoteSystem = psSubset.getString("startDaemonHost");
        psSubset.writeFile(_getShareLocation() + string("mnt/") + remoteSystem + string("/") + parameterFileName);
  
        // send the schedule to the startdaemon of the child
        TLogicalDeviceTypes ldType = static_cast<TLogicalDeviceTypes>(psSubset.getInt("logicalDeviceType"));
        STARTDAEMONScheduleEvent scheduleEvent;
        scheduleEvent.logicalDeviceType = ldType;
        scheduleEvent.taskName = startDaemonKey;
        scheduleEvent.fileName = parameterFileName;
        startDaemonPort->send(scheduleEvent);
      }
      catch(Exception& e)
      {
        LOG_FATAL(formatString("(%s) Fatal error while scheduling child",e.message().c_str()));
      }
      ++it;
    }
  }
  else
  {
    // send schedule to clients
    TPortMap::iterator it = m_connectedChildPorts.begin();
    while(it != m_connectedChildPorts.end())
    {
      try
      {
        // extract the parameterset for the child
        string childKey = it->first;
        TPortSharedPtr childPort = it->second;
        ACC::ParameterSet psSubset = m_parameterSet.makeSubset(childKey + string("."));
        string parameterFileName = childKey+string(".ps"); 
        string remoteSystem = psSubset.getString("startDaemonHost");
        psSubset.writeFile(_getShareLocation() + string("mnt/") + remoteSystem + string("/") + parameterFileName);
  
        // send the schedule to the child
        LOGICALDEVICEScheduleEvent scheduleEvent;
        scheduleEvent.fileName = parameterFileName;
        childPort->send(scheduleEvent);
      }
      catch(Exception& e)
      {
        LOG_FATAL(formatString("(%s) Fatal error while scheduling child",e.message().c_str()));
      }
      ++it;
    }
  }
}

string LogicalDevice::_getShareLocation() const
{
  string shareLocation("/home/lofar/MACTransport/");
  GCF::ParameterSet* pParamSet = GCF::ParameterSet::instance();
  try
  {
    shareLocation = pParamSet->getString("shareLocation");
  } 
  catch(Exception& e)
  {
    LOG_WARN(formatString("(%s) Sharelocation parameter not found. Using /home/lofar/MACTransport/",e.message().c_str()));
  }
  return shareLocation;
}

::GCFEvent::TResult LogicalDevice::initial_state(::GCFEvent& event, ::GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  ::GCFEvent::TResult status = ::GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_IDLE;
      ::GCFPVString status(LD_STATE_STRING_INITIAL);
      m_propertySet->setValue(LD_PROPNAME_STATUS,status);
      
      // create the childs
      vector<string> childKeys = _getChildKeys();
      vector<string>::iterator chIt;
      for(chIt=childKeys.begin(); chIt!=childKeys.end();++chIt)
      {
        // connect to child startdaemon.
        try
        {
          string startDaemonHostName = m_parameterSet.getString((*chIt) + string(".startDaemonHost"));
          string startDaemonPortName = m_parameterSet.getString((*chIt) + string(".startDaemonPort"));
          string startDaemonTaskName = m_parameterSet.getString((*chIt) + string(".startDaemonTask"));
          string childPsName         = startDaemonHostName + string(":") + m_parameterSet.getString((*chIt) + string(".propertysetName"));
          
          TPortSharedPtr startDaemonPort(new TRemotePort(*this,startDaemonTaskName,::GCFPortInterface::SAP,0));
          TPeerAddr peerAddr;
          peerAddr.taskname = startDaemonTaskName;
          peerAddr.portname = startDaemonPortName;
          startDaemonPort->setAddr(peerAddr);
          startDaemonPort->open();
          m_childStartDaemonPorts[(*chIt)] = startDaemonPort;
          
          // add reference in propertyset
          GCFPVDynArr* childRefs = static_cast<GCFPVDynArr*>(m_propertySet->getValue(LD_PROPNAME_CHILDREFS));
          if(childRefs != 0)
          {
            GCFPValueArray refsVector(childRefs->getValue()); // create a copy 
            GCFPVString newRef((*chIt) + string("=") + childPsName);
            refsVector.push_back(&newRef);
            GCFPVDynArr newChildRefs(LPT_STRING,refsVector);
            m_propertySet->setValue(LD_PROPNAME_CHILDREFS,newChildRefs);
          }
        }
        catch(Exception& e)
        {
          LOG_FATAL(formatString("(%s) Unable to create child %s",e.message().c_str(),(*chIt).c_str()));
        }
      }
      
      // connect to parent.
      string parentPortName = m_parameterSet.getString("parentPort");
      string parentTaskName = m_parameterSet.getString("parentTask");
      m_parentPort.init(*this,parentTaskName,::GCFPortInterface::SAP, LOGICALDEVICE_PROTOCOL);
      TPeerAddr peerAddr;
      peerAddr.taskname = parentTaskName;
      peerAddr.portname = parentPortName;
      m_parentPort.setAddr(peerAddr);
      m_parentPort.open();

      _schedule();
      
      // poll retry buffer every 10 seconds
      m_retrySendTimerId = m_serverPort.setTimer(10L);
      break;
    }
  
    case F_CONNECTED:
    {
      string startDaemonKey;
      if(_isParentPort(port))
      {
        LOGICALDEVICEConnectEvent connectEvent;
        connectEvent.nodeId = getName();
        port.send(connectEvent);
      }
      break;
    }
    
    case LOGICALDEVICE_CONNECTED:
    {
      LOGICALDEVICEConnectedEvent connectedEvent(event);
      if(connectedEvent.result == LD_RESULT_NO_ERROR)
      {
        _doStateTransition(LOGICALDEVICE_STATE_IDLE);
      }
      break;
    }
      
    case F_DISCONNECTED:
      port.close();
      break;

    case F_TIMER:
      _handleTimers(event,port);
      break;
    
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::initial_state, default",getName().c_str()));
      status = ::GCFEvent::NOT_HANDLED;
      break;
  }    
  TLogicalDeviceState newState=LOGICALDEVICE_STATE_NOSTATE;
  ::GCFEvent::TResult concreteStatus;
  concreteStatus = concrete_initial_state(event, port, newState);
  _doStateTransition(newState);
  return (status==::GCFEvent::HANDLED||concreteStatus==::GCFEvent::HANDLED?::GCFEvent::HANDLED : ::GCFEvent::NOT_HANDLED);
}

::GCFEvent::TResult LogicalDevice::idle_state(::GCFEvent& event, ::GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  ::GCFEvent::TResult status = ::GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_IDLE;
      // open the server port to allow childs to connect
      m_serverPort.open();
      
      // send initialized event to the parent
      LOGICALDEVICEScheduledEvent scheduledEvent;
      scheduledEvent.result=LD_RESULT_NO_ERROR;//OK
      m_parentPort.send(scheduledEvent);
      
      ::GCFPVString status(LD_STATE_STRING_IDLE);
      m_propertySet->setValue(LD_PROPNAME_STATUS,status);
      break;
    }
  
    case F_ACCEPT_REQ:
    {
      TPortSharedPtr server(new TRemotePort);
      server->init(*this, m_serverPortName, GCFPortInterface::SPP, LOGICALDEVICE_PROTOCOL);
      m_serverPort.accept(*(server.get()));
      _addChildPort(server);
      break;
    }

    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
    
    case F_TIMER:
      _handleTimers(event,port);
      break;

    case LOGICALDEVICE_CONNECT:
    {
      LOGICALDEVICEConnectEvent connectEvent(event);
      TPortSharedPtr portPtr(static_cast<TRemotePort*>(&port));
      m_connectedChildPorts[connectEvent.nodeId] = TPortSharedPtr(portPtr);
      
      LOGICALDEVICEConnectedEvent connectedEvent;
      connectedEvent.result = LD_RESULT_NO_ERROR;
      port.send(connectedEvent);
      break;
    }
      
    case LOGICALDEVICE_CLAIM:
      TRAN(LogicalDevice::claiming_state);
      concreteClaim(port);
      break;
      
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::idle_state, default",getName().c_str()));
      status = ::GCFEvent::NOT_HANDLED;
      break;
  }    
  TLogicalDeviceState newState=LOGICALDEVICE_STATE_NOSTATE;
  ::GCFEvent::TResult concreteStatus;
  concreteStatus = concrete_idle_state(event, port, newState);
  _doStateTransition(newState);
  return (status==::GCFEvent::HANDLED||concreteStatus==::GCFEvent::HANDLED?::GCFEvent::HANDLED : ::GCFEvent::NOT_HANDLED);
}

::GCFEvent::TResult LogicalDevice::claiming_state(::GCFEvent& event, ::GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  ::GCFEvent::TResult status = ::GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_CLAIMING;
      ::GCFPVString status(LD_STATE_STRING_CLAIMING);
      m_propertySet->setValue(LD_PROPNAME_STATUS,status);
      
      // send claim event to childs
      LOGICALDEVICEClaimEvent claimEvent;
      _sendToAllChilds(claimEvent);
      break;
    }
  
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
      
    case F_TIMER:
      _handleTimers(event,port);
      break;

    // the LOGICALDEVICE_CLAIMED event cannot result in a transition to 
    // the claimed state here, because the logical device may have several 
    // children that all send their LOGICALDEVICE_CLAIMED message. 
    // The parent can only enter the claimed state when all children have
    // sent their LOGICALDEVICE_CLAIMED message. That cannot be decided in
    // this baseclass.
    
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::claiming_state, default",getName().c_str()));
      status = ::GCFEvent::NOT_HANDLED;
      break;
  }
  TLogicalDeviceState newState=LOGICALDEVICE_STATE_NOSTATE;
  ::GCFEvent::TResult concreteStatus;
  concreteStatus = concrete_claiming_state(event, port, newState);
  _doStateTransition(newState);
  return (status==::GCFEvent::HANDLED||concreteStatus==::GCFEvent::HANDLED?::GCFEvent::HANDLED : ::GCFEvent::NOT_HANDLED);
}

::GCFEvent::TResult LogicalDevice::claimed_state(::GCFEvent& event, ::GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  ::GCFEvent::TResult status = ::GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_CLAIMED;
      ::GCFPVString status(LD_STATE_STRING_CLAIMED);
      m_propertySet->setValue(LD_PROPNAME_STATUS,status);

      // send claimed message to the parent.
      LOGICALDEVICEClaimedEvent claimedEvent;
      m_parentPort.send(claimedEvent);
      break;
    }
    
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
    
    case F_TIMER:
      _handleTimers(event,port);
      break;
   
    case LOGICALDEVICE_PREPARE:
    {
      TRAN(LogicalDevice::preparing_state);
      
      concretePrepare(port);
      break;
    }
    
    case LOGICALDEVICE_RELEASE:
      TRAN(LogicalDevice::releasing_state);
      concreteRelease(port);
      break;

    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::claimed_state, default",getName().c_str()));
      status = ::GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

::GCFEvent::TResult LogicalDevice::preparing_state(::GCFEvent& event, ::GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  ::GCFEvent::TResult status = ::GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_PREPARING;
      ::GCFPVString status(LD_STATE_STRING_PREPARING);
      m_propertySet->setValue(LD_PROPNAME_STATUS,status);

      // send prepare event to childs
      LOGICALDEVICEPrepareEvent prepareEvent;
      _sendToAllChilds(prepareEvent);
      break;
    }
    
    case F_EXIT:
      break;
      
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
      
    case F_TIMER:
      _handleTimers(event,port);
      break;

    // the LOGICALDEVICE_PREPARED event cannot result in a transition to 
    // the suspended state here, because the logical device may have several 
    // children that all send their LOGICALDEVICE_PREPARED message. 
    // The parent can only enter the prepared state when all children have
    // sent their LOGICALDEVICE_PREPARED message. That cannot be decided in
    // this baseclass.
    
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::preparing_state, default",getName().c_str()));
      status = ::GCFEvent::NOT_HANDLED;
      break;
  }
  TLogicalDeviceState newState=LOGICALDEVICE_STATE_NOSTATE;
  ::GCFEvent::TResult concreteStatus;
  concreteStatus = concrete_preparing_state(event, port, newState);
  _doStateTransition(newState);
  return (status==::GCFEvent::HANDLED||concreteStatus==::GCFEvent::HANDLED?::GCFEvent::HANDLED : ::GCFEvent::NOT_HANDLED);
}

::GCFEvent::TResult LogicalDevice::suspended_state(::GCFEvent& event, ::GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  ::GCFEvent::TResult status = ::GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_SUSPENDED;
      ::GCFPVString status(LD_STATE_STRING_SUSPENDED);
      m_propertySet->setValue(LD_PROPNAME_STATUS,status);

      // send to parent
      LOGICALDEVICESuspendedEvent suspendedEvent;
      m_parentPort.send(suspendedEvent);
      break;
    }
  
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
    
    case F_TIMER:
      _handleTimers(event,port);
      break;
    
    case LOGICALDEVICE_PREPARE:
    {
      TRAN(LogicalDevice::preparing_state);

      concretePrepare(port);
      break;
    }
    
    case LOGICALDEVICE_RESUME:
    {
      // send resume event to childs
      LOGICALDEVICEResumeEvent resumeEvent;
      _sendToAllChilds(resumeEvent);

      TRAN(LogicalDevice::active_state);
      concreteResume(port);
      break;
    }
    
    case LOGICALDEVICE_RELEASE:
    {
      // send release event to childs
      LOGICALDEVICEResumeEvent releaseEvent;
      _sendToAllChilds(releaseEvent);

      TRAN(LogicalDevice::releasing_state);
      concreteRelease(port);
      break;
    }
    
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::suspended_state, default",getName().c_str()));
      status = ::GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

::GCFEvent::TResult LogicalDevice::active_state(::GCFEvent& event, ::GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  ::GCFEvent::TResult status = ::GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;      
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_ACTIVE;
      ::GCFPVString status(LD_STATE_STRING_ACTIVE);
      m_propertySet->setValue(LD_PROPNAME_STATUS,status);

      // send resumed message to parent.
      LOGICALDEVICEResumedEvent resumedEvent;
      m_parentPort.send(resumedEvent);
      break;
    }
  
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
    
    case F_TIMER:
      _handleTimers(event,port);
      break;
    
    case LOGICALDEVICE_PREPARE:
      // invalid message in this state
      LOG_DEBUG(formatString("LogicalDevice(%s)::active_state, PREPARE NOT ALLOWED",getName().c_str()));
      break;
      
    case LOGICALDEVICE_SUSPEND:
    {
      TRAN(LogicalDevice::suspended_state);

      // send suspend event to childs
      LOGICALDEVICESuspendEvent suspendEvent;
      _sendToAllChilds(suspendEvent);

      concreteSuspend(port);
      break;
    }
    
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::active_state, default",getName().c_str()));
      status = ::GCFEvent::NOT_HANDLED;
      break;
  }
  TLogicalDeviceState newState=LOGICALDEVICE_STATE_NOSTATE;
  ::GCFEvent::TResult concreteStatus;
  concreteStatus = concrete_active_state(event, port, newState);
  return (status==::GCFEvent::HANDLED||concreteStatus==::GCFEvent::HANDLED?::GCFEvent::HANDLED : ::GCFEvent::NOT_HANDLED);
}

::GCFEvent::TResult LogicalDevice::releasing_state(::GCFEvent& event, ::GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  ::GCFEvent::TResult status = ::GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_RELEASING;
      ::GCFPVString status(LD_STATE_STRING_RELEASING);
      m_propertySet->setValue(LD_PROPNAME_STATUS,status);
      break;
    }
  
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;

    case F_TIMER:
      _handleTimers(event,port);
      break;

    // the LOGICALDEVICE_RELEASED event cannot result in a transition to 
    // the idle state here, because the logical device may have several 
    // children that all send their LOGICALDEVICE_RELEASED message. 
    // The parent can only enter the idle state when all children have
    // sent their LOGICALDEVICE_RELEASED message. That cannot be decided in
    // this baseclass.
    
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::releasing_state, default",getName().c_str()));
      status = ::GCFEvent::NOT_HANDLED;
      break;
  }
  TLogicalDeviceState newState=LOGICALDEVICE_STATE_NOSTATE;
  ::GCFEvent::TResult concreteStatus;
  concreteStatus = concrete_releasing_state(event, port, newState);
  _doStateTransition(newState);
  return (status==::GCFEvent::HANDLED||concreteStatus==::GCFEvent::HANDLED?::GCFEvent::HANDLED : ::GCFEvent::NOT_HANDLED);
}

};
};
