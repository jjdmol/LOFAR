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
#include <Common/LofarLogger.h>

#include <unistd.h>
#include <boost/shared_array.hpp>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVInteger.h>
#include <GCF/GCF_PVDynArr.h>
#include <GCF/ParameterSet.h>
#include "APLCommon/APL_Defines.h"
#include "APLCommon/APLUtilities.h"
#include "APLCommon/LogicalDevice.h"

using namespace LOFAR::ACC::APS;
using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

namespace LOFAR
{
namespace APLCommon
{

const string LogicalDevice::LD_CONFIG_PREFIX            = string("mac.apl.ld.");

const string LogicalDevice::LD_STATE_STRING_DISABLED    = string("Disabled");
const string LogicalDevice::LD_STATE_STRING_INITIAL     = string("Initial");
const string LogicalDevice::LD_STATE_STRING_IDLE        = string("Idle");
const string LogicalDevice::LD_STATE_STRING_CLAIMING    = string("Claiming");
const string LogicalDevice::LD_STATE_STRING_CLAIMED     = string("Claimed");
const string LogicalDevice::LD_STATE_STRING_PREPARING   = string("Preparing");
const string LogicalDevice::LD_STATE_STRING_SUSPENDED   = string("Suspended");
const string LogicalDevice::LD_STATE_STRING_ACTIVE      = string("Active");
const string LogicalDevice::LD_STATE_STRING_RELEASING   = string("Releasing");
const string LogicalDevice::LD_STATE_STRING_GOINGDOWN   = string("Going Down");

const string LogicalDevice::LD_PROPSET_TYPENAME         = string("TAplLogicalDevice");
const string LogicalDevice::LD_PROPNAME_COMMAND         = string("command");
const string LogicalDevice::LD_PROPNAME_STATUS          = string("status");
const string LogicalDevice::LD_PROPNAME_STATE           = string("state");
const string LogicalDevice::LD_PROPNAME_VERSION         = string("version");
const string LogicalDevice::LD_PROPNAME_CLAIMTIME       = string("claimTime");
const string LogicalDevice::LD_PROPNAME_PREPARETIME     = string("prepareTime");
const string LogicalDevice::LD_PROPNAME_STARTTIME       = string("startTime");
const string LogicalDevice::LD_PROPNAME_STOPTIME        = string("stopTime");
const string LogicalDevice::LD_PROPNAME_CHILDREFS       = string("__childs");

const string LogicalDevice::LD_COMMAND_SCHEDULE         = string("SCHEDULE");
const string LogicalDevice::LD_COMMAND_CANCELSCHEDULE   = string("CANCELSCHEDULE");
const string LogicalDevice::LD_COMMAND_CLAIM            = string("CLAIM");
const string LogicalDevice::LD_COMMAND_PREPARE          = string("PREPARE");
const string LogicalDevice::LD_COMMAND_RESUME           = string("RESUME");
const string LogicalDevice::LD_COMMAND_SUSPEND          = string("SUSPEND");
const string LogicalDevice::LD_COMMAND_RELEASE          = string("RELEASE");

INIT_TRACER_CONTEXT(LogicalDevice,LOFARLOGGER_PACKAGE);

LogicalDevice::LogicalDevice(const string& taskName, 
                             const string& parameterFile, 
                             GCFTask* pStartDaemon,
                             const string& version) throw (APLCommon::ParameterFileNotFoundException, 
                                                           APLCommon::ParameterNotFoundException,
                                                           APLCommon::WrongVersionException) :
  GCFTask((State)&LogicalDevice::initial_state,taskName),
  PropertySetAnswerHandlerInterface(),
  m_startDaemon(pStartDaemon),
  m_propertySetAnswer(*this),
  m_basePropertySet(),
  m_basePropertySetName(),
  m_detailsPropertySet(),
  m_detailsPropertySetName(),
  m_parameterSet(),
  m_serverPortName(string("server")),
  m_serverPort(*this, m_serverPortName, GCFPortInterface::MSPP, LOGICALDEVICE_PROTOCOL),
  m_claimTime(0),
  m_prepareTime(0),
  m_startTime(0),
  m_stopTime(0),
  m_parentPorts(),
  m_parentReconnectTimerId(0),
  m_childPorts(),
  m_connectedChildPorts(),
  m_childStartDaemonPorts(),
  m_apcLoaded(false),
  m_logicalDeviceState(LOGICALDEVICE_STATE_DISABLED),
  m_claimTimerId(0),
  m_prepareTimerId(0),
  m_startTimerId(0),
  m_stopTimerId(0),
  m_retrySendTimerId(0),
  m_eventBuffer(),
  m_globalError(LD_RESULT_NO_ERROR),
  m_version(version),
  m_childTypes(),
  m_childStates()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  LOG_INFO(formatString("Constructing %s",getName().c_str()));
  
#ifdef USE_TCPPORT_INSTEADOF_PVSSPORT
  LOG_WARN("Using GCFTCPPort in stead of GCFPVSSPort");
#endif

  registerProtocol(LOGICALDEVICE_PROTOCOL, LOGICALDEVICE_PROTOCOL_signalnames);

  char localHostName[300];
  gethostname(localHostName,300);
  m_serverPort.setHostName(localHostName);
  
  string psDetailsType("");
  
  adoptParameterFile(parameterFile);
  
  try
  {
    m_basePropertySetName = m_parameterSet.getString("propertysetBaseName");
    psDetailsType = m_parameterSet.getString("propertysetDetailsType");
  }
  catch(Exception& e)
  {
    THROW(APLCommon::ParameterNotFoundException,e.message());
  }

  m_detailsPropertySetName = m_basePropertySetName + string("_details");
  
  m_basePropertySet = boost::shared_ptr<GCFMyPropertySet>(new GCFMyPropertySet(
      m_basePropertySetName.c_str(),
      LD_PROPSET_TYPENAME.c_str(),
      PS_CAT_TEMPORARY,
      &m_propertySetAnswer));
  m_basePropertySet->enable();
  m_detailsPropertySet = boost::shared_ptr<GCFMyPropertySet>(new GCFMyPropertySet(
      m_detailsPropertySetName.c_str(),
      psDetailsType.c_str(),
      PS_CAT_TEMPORARY,
      &m_propertySetAnswer));
  m_detailsPropertySet->enable();
  
  m_logicalDeviceState = LOGICALDEVICE_STATE_INITIAL;
}


LogicalDevice::~LogicalDevice()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  LOG_INFO(formatString("Destructing %s",getName().c_str()));

//  m_detailsPropertySet->disable();
//  m_basePropertySet->disable();

  // clear all vectors and maps
  m_eventBuffer.clear();
  m_childStartDaemonPorts.clear();
  m_connectedChildPorts.clear();
  m_childPorts.clear();
}

void LogicalDevice::adoptParameterFile(const string& parameterFile)
{
  try
  {
    m_parameterSet.adoptFile(_getShareLocation() + parameterFile);
  }
  catch(Exception& e)
  {
    THROW(APLCommon::ParameterFileNotFoundException,e.message());
  }
  
  // check version number
  LOG_WARN("Version checking not implemented: need to get the version from the node in the OTDB database");
#if 0
  try
  {
    string receivedVersion = m_parameterSet.getString(string("versionnr"));
    if(receivedVersion != m_version)
    {
      THROW(APLCommon::WrongVersionException,string("Expected version ") + m_version + string("; received version ") + receivedVersion);
    }
  }
  catch(Exception& e)
  {
    THROW(APLCommon::ParameterNotFoundException,e.message());
  }
#endif
}


void LogicalDevice::updateParameterFile(const string& parameterFile)
{
  // this method adopts the new parameter file,
  // widens the schedule times if necessary,
  // connects to the parent if it is a new parent
  // sends the new schedule to all its childs
  // This method is called by LogicalDeviceFactories that support LD-sharing:
  // SRG, SO
  adoptParameterFile(parameterFile);

  _connectParent();
  _schedule();
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

void LogicalDevice::handlePropertySetAnswer(GCFEvent& answer)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(answer)).c_str());
  switch(answer.signal)
  {
    case F_MYPS_ENABLED:
    {
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
      
      // first check if the property is from the details propertyset
      if(strstr(pPropAnswer->pScope, m_detailsPropertySetName.c_str()) != 0)
      {
        // let the specialized class handle this
        concrete_handlePropertySetAnswer(answer);
      }
      else if(strstr(pPropAnswer->pScope, m_basePropertySetName.c_str()) != 0)
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

      // first check if the property is from the details propertyset
      if(strstr(pConfAnswer->pScope, m_detailsPropertySetName.c_str()) != 0)
      {
        // let the specialized class handle this
        concrete_handlePropertySetAnswer(answer);
      }
      else if(strstr(pConfAnswer->pScope, m_basePropertySetName.c_str()) != 0)
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
      GCFPropValueEvent* pPropValueAnswer=static_cast<GCFPropValueEvent*>(&answer);

      // first check if the property is from the details propertyset
      if(strstr(pPropValueAnswer->pPropName, m_detailsPropertySetName.c_str()) != 0)
      {
        // let the specialized class handle this
        concrete_handlePropertySetAnswer(answer);
      }
      else if(strstr(pPropValueAnswer->pPropName, m_basePropertySetName.c_str()) != 0)
      {
        // it is my own propertyset
        if((pPropValueAnswer->pValue->getType() == LPT_STRING) &&
           (strstr(pPropValueAnswer->pPropName, LD_PROPNAME_COMMAND.c_str()) != 0))
        {
          // command received
          string commandString(((GCFPVString*)pPropValueAnswer->pValue)->getValue());
          vector<string> parameters;
          string command;
          APLUtilities::decodeCommand(commandString,command,parameters);
          
          // SCHEDULE <fileName>
          if(command==string(LD_COMMAND_SCHEDULE))
          {
            if(parameters.size()==1)
            {
              m_parameterSet.adoptFile(_getShareLocation() + parameters[0]);
              _schedule();
            }
            else
            {
              TLDResult result = LD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS;
              m_basePropertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
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
              m_basePropertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
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
              m_basePropertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
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
              m_basePropertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
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
              m_basePropertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
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
              m_basePropertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
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
              m_basePropertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
            }
          }
          else
          {
            TLDResult result = LD_RESULT_UNKNOWN_COMMAND;
            m_basePropertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
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

time_t LogicalDevice::getClaimTime() const
{
  return m_claimTime;
}

time_t LogicalDevice::getPrepareTime() const
{
  return m_prepareTime;
}

time_t LogicalDevice::getStartTime() const
{
  return m_startTime;
}

time_t LogicalDevice::getStopTime() const
{
  return m_stopTime;
}

void LogicalDevice::copyParentValue(ACC::APS::ParameterSet& psSubset, const string& key)
{
  if(m_parameterSet.find(key) != m_parameterSet.end())
  {
    psSubset.add(key,m_parameterSet.getString(key));
  }
}

void LogicalDevice::concreteAddExtraKeys(ACC::APS::ParameterSet& /*psSubset*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  // do nothing
}

void LogicalDevice::_schedule()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  // (erase and) fill the childTypes and childStates maps
  TString2LDTypeMap  newChildTypes;
  TString2LDStateMap newChildStates;
  // create the childs
  vector<string> childKeys = _getChildKeys();
  vector<string>::iterator chIt;
  for(chIt=childKeys.begin(); chIt!=childKeys.end();++chIt)
  {
    // add new childs
    if(m_childTypes.find(*chIt) != m_childTypes.end())
    {
      newChildTypes[*chIt]=m_childTypes[*chIt];
      newChildStates[*chIt]=m_childStates[*chIt];
    }
    else
    {
      try
      {
        string ldTypeString = m_parameterSet.getString((*chIt) + ".logicalDeviceType");
        TLogicalDeviceTypes ldType = APLUtilities::convertLogicalDeviceType(ldTypeString);
        newChildTypes[*chIt] = ldType;
        newChildStates[*chIt] = LOGICALDEVICE_STATE_IDLE;
      }
      catch(Exception& e)
      {
        LOG_FATAL(formatString("(%s) Unable to create child %s",e.message().c_str(),(*chIt).c_str()));
      }
    }
  }
  m_childTypes.clear();
  m_childStates.clear();
  m_childTypes = newChildTypes;
  m_childStates = newChildStates;

  // adjust scheduling times
  // specified times are in UTC, seconds since 1-1-1970
  time_t timeNow = APLUtilities::getUTCtime();
  time_t claimTime   = APLUtilities::decodeTimeString(m_parameterSet.getString("claimTime"));
  time_t prepareTime = APLUtilities::decodeTimeString(m_parameterSet.getString("prepareTime"));
  time_t startTime   = APLUtilities::decodeTimeString(m_parameterSet.getString("startTime"));
  time_t stopTime    = APLUtilities::decodeTimeString(m_parameterSet.getString("stopTime"));
  
  // timerId's can be zero if the LD has been released
  if((claimTime >= timeNow && claimTime < m_claimTime) || m_claimTimerId==0)
  {
    char timeString1[200];
    char timeString2[200];
    struct tm* tmTime=localtime(&m_claimTime);
    strcpy(timeString1,asctime(tmTime));
    timeString1[strlen(timeString1)-1]=0;
    tmTime=localtime(&claimTime);
    strcpy(timeString2,asctime(tmTime));
    timeString2[strlen(timeString2)-1]=0;
    LOG_INFO(formatString("Changing claim time from %s to %s",timeString1,timeString2));

    // earlier claim
    m_claimTime = claimTime;
    m_serverPort.cancelTimer(m_claimTimerId);
    m_claimTimerId = m_serverPort.setTimer(m_claimTime - timeNow);
  }
  if((prepareTime >= timeNow && prepareTime < m_prepareTime) || m_prepareTimerId==0)
  {
    char timeString1[200];
    char timeString2[200];
    struct tm* tmTime=localtime(&m_prepareTime);
    strcpy(timeString1,asctime(tmTime));
    timeString1[strlen(timeString1)-1]=0;
    tmTime=localtime(&prepareTime);
    strcpy(timeString2,asctime(tmTime));
    timeString2[strlen(timeString2)-1]=0;
    LOG_INFO(formatString("Changing prepare time from %s to %s",timeString1,timeString2));

    // earlier prepare
    m_prepareTime = prepareTime;
    m_serverPort.cancelTimer(m_prepareTimerId);
    m_prepareTimerId = m_serverPort.setTimer(m_prepareTime - timeNow);
  }
  if((startTime >= timeNow && startTime < m_startTime) || m_startTimerId==0)
  {
    char timeString1[200];
    char timeString2[200];
    struct tm* tmTime=localtime(&m_startTime);
    strcpy(timeString1,asctime(tmTime));
    timeString1[strlen(timeString1)-1]=0;
    tmTime=localtime(&startTime);
    strcpy(timeString2,asctime(tmTime));
    timeString2[strlen(timeString2)-1]=0;
    LOG_INFO(formatString("Changing start time from %s to %s",timeString1,timeString2));

    // earlier start
    m_startTime = startTime;
    m_serverPort.cancelTimer(m_startTimerId);
    m_startTimerId = m_serverPort.setTimer(m_startTime - timeNow);
  }
  if((stopTime >= timeNow && stopTime > m_stopTime) || m_stopTimerId==0)
  {
    char timeString1[200];
    char timeString2[200];
    struct tm* tmTime=localtime(&m_stopTime);
    strcpy(timeString1,asctime(tmTime));
    timeString1[strlen(timeString1)-1]=0;
    tmTime=localtime(&stopTime);
    strcpy(timeString2,asctime(tmTime));
    timeString2[strlen(timeString2)-1]=0;
    LOG_INFO(formatString("Changing stop time from %s to %s",timeString1,timeString2));

    // later stop
    m_stopTime = stopTime;
    m_serverPort.cancelTimer(m_stopTimerId);
    m_stopTimerId = m_serverPort.setTimer(m_stopTime - timeNow);
  }

  // set properties
  m_basePropertySet->setValue(LD_PROPNAME_CLAIMTIME,GCFPVInteger(m_claimTime));
  m_basePropertySet->setValue(LD_PROPNAME_PREPARETIME,GCFPVInteger(m_prepareTime));
  m_basePropertySet->setValue(LD_PROPNAME_STARTTIME,GCFPVInteger(m_startTime));
  m_basePropertySet->setValue(LD_PROPNAME_STOPTIME,GCFPVInteger(m_stopTime));

  _sendScheduleToClients();
}

void LogicalDevice::_cancelSchedule(const TLDResult& errorCode)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  m_globalError = errorCode;
  m_serverPort.cancelTimer(m_claimTimerId);
  m_claimTimerId = 0;
  m_serverPort.cancelTimer(m_prepareTimerId);
  m_prepareTimerId = 0;
  m_serverPort.cancelTimer(m_startTimerId);
  m_startTimerId = 0;
  m_serverPort.cancelTimer(m_stopTimerId);
  m_stopTimerId = 0;
  
  // propagate to childs
  boost::shared_ptr<LOGICALDEVICECancelscheduleEvent> cancelEvent(new LOGICALDEVICECancelscheduleEvent);
  _sendToAllChilds(cancelEvent);
  
  _release();
}

void LogicalDevice::_claim()
{
  LOGICALDEVICEClaimEvent claimEvent;
  dispatch(claimEvent,m_serverPort);
}

void LogicalDevice::_prepare()
{
  LOGICALDEVICEPrepareEvent prepareEvent;
  dispatch(prepareEvent,m_serverPort);
}

void LogicalDevice::_resume()
{
  LOGICALDEVICEResumeEvent resumeEvent;
  dispatch(resumeEvent,m_serverPort);
}

void LogicalDevice::_suspend()
{
  LOGICALDEVICESuspendEvent suspendEvent;
  dispatch(suspendEvent,m_serverPort);
}

void LogicalDevice::_release()
{
  LOGICALDEVICEReleaseEvent releaseEvent;
  dispatch(releaseEvent,m_serverPort);
}

LogicalDevice::TPortMap::iterator LogicalDevice::_findParentPort(GCFPortInterface& port)
{
  TPortMap::iterator it=m_parentPorts.begin();
  bool found=false;
  while(!found && it != m_parentPorts.end())
  {
    if(&port == it->second.get()) // comparing two pointers. yuck?
    {
      found=true;
    }
    else
    {
      ++it;
    }
    
  }
  return it;
}
   
bool LogicalDevice::_isServerPort(GCFPortInterface& port)
{
  return (&port == &m_serverPort); // comparing two pointers. yuck?
}
   
bool LogicalDevice::_isChildPort(GCFPortInterface& port)
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

LogicalDevice::TPortVector::iterator LogicalDevice::_getChildPort(GCFPortInterface& port)
{
  bool found=false;
  TPortVector::iterator it=m_childPorts.begin();
  while(!found && it != m_childPorts.end())
  {
    found = (&port == (*it).get()); // comparing two pointers. yuck?
    if(!found)
    {
      ++it;
    }
  }
  return it;
}

bool LogicalDevice::_isChildStartDaemonPort(GCFPortInterface& port, string& startDaemonKey)
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

void LogicalDevice::_sendToAllChilds(GCFEventSharedPtr eventPtr)
{
  // send to all childs
  TPortWeakPtrMap::iterator it=m_connectedChildPorts.begin();
  while(it != m_connectedChildPorts.end())
  {
    try
    {
      if(TPortSharedPtr pChildPort = it->second.lock())
      {
        _sendEvent(eventPtr,*pChildPort);
      }
    }
    catch(Exception& e)
    {
      LOG_FATAL(formatString("(%s) Fatal error while sending message to child %s",e.message().c_str(),it->first.c_str()));
    }
    ++it;
  }
}

void LogicalDevice::_setChildStates(TLogicalDeviceState ldState)
{
  // set all child states
  TString2LDStateMap::iterator it=m_childStates.begin();
  while(it != m_childStates.end())
  {
    it->second = ldState;
    ++it;
  }
}

void LogicalDevice::_setConnectedChildState(GCFPortInterface& port, TLogicalDeviceState ldState)
{
  bool found=false;
  TPortWeakPtrMap::iterator it=m_connectedChildPorts.begin();
  while(!found && it != m_connectedChildPorts.end())
  {
    string childKey = it->first;
    if(TPortSharedPtr pChildPort = it->second.lock())
    {
      if(&port == pChildPort.get())
      {
        found=true;
        m_childStates[childKey] = ldState;
      }
    }
    ++it;
  }
}

string LogicalDevice::_getConnectedChildName(GCFPortInterface& port)
{
  string childKey;
  bool found=false;
  TPortWeakPtrMap::iterator it=m_connectedChildPorts.begin();
  while(!found && it != m_connectedChildPorts.end())
  {
    if(TPortSharedPtr pChildPort = it->second.lock())
    {
      if(&port == pChildPort.get())
      {
        found=true;
        childKey = it->first;
      }
    }
    ++it;
  }
  return childKey;
}

// check if enough LD's of a specific type are in a specific state
bool LogicalDevice::_childsInState(const double requiredPercentage, const TLogicalDeviceTypes& type, const TLogicalDeviceState& state)
{
  double totalLDs(0.0);
  double ldsInState(0.0);
  
  for(TString2LDTypeMap::iterator typesIt = m_childTypes.begin();typesIt != m_childTypes.end();++typesIt)
  {
    if(typesIt->second == type || type == LDTYPE_NO_TYPE)
    {
      totalLDs += 1.0;
      TString2LDStateMap::iterator statesIt = m_childStates.find(typesIt->first);
      if(statesIt != m_childStates.end())
      {
        LOG_DEBUG(formatString("%s is in state %s",typesIt->first.c_str(),_state2String(statesIt->second).c_str()));
        if(statesIt->second == state)
        {
          ldsInState += 1.0;
        }
      }
    }
  }
  
  double resultingPercentage(0.0);
  if(totalLDs > 0.0)
  {
    resultingPercentage = ldsInState/totalLDs*100.0;
  }
  
  return (resultingPercentage >= requiredPercentage);
}

// check if enough LD's of a specific type are not in a specific state
bool LogicalDevice::_childsNotInState(const double requiredPercentage, const TLogicalDeviceTypes& type, const TLogicalDeviceState& state)
{
  double totalLDs(0.0);
  double ldsNotInState(0.0);
  
  for(TString2LDTypeMap::iterator typesIt = m_childTypes.begin();typesIt != m_childTypes.end();++typesIt)
  {
    if(typesIt->second == type || type == LDTYPE_NO_TYPE)
    {
      totalLDs += 1.0;
      TString2LDStateMap::iterator statesIt = m_childStates.find(typesIt->first);
      if(statesIt != m_childStates.end())
      {
        LOG_DEBUG(formatString("%s is in state %s",typesIt->first.c_str(),_state2String(statesIt->second).c_str()));
        if(statesIt->second != state)
        {
          ldsNotInState += 1.0;
        }
      }
    }
  }
  
  double resultingPercentage(0.0);
  if(totalLDs > 0.0)
  {
    resultingPercentage = ldsNotInState/totalLDs*100.0;
  }
  
  return (resultingPercentage >= requiredPercentage);
}

void LogicalDevice::_disconnectedHandler(GCFPortInterface& port)
{
  LOG_WARN(formatString("port '%s' disconnected, retry in 3 seconds...", port.getName().c_str()));

  string startDaemonKey;
  port.close();
  if(_isServerPort(port))
  {
    LOG_ERROR(formatString("Server port of task %s could not be opened",getName().c_str()));
  }
  else if(_isChildPort(port))
  {
    LOG_ERROR(formatString("Connection with child %s failed",_getConnectedChildName(port).c_str()));
    concreteChildDisconnected(port);
  }
  else if(_findParentPort(port)!=m_parentPorts.end())
  {
    LOG_ERROR(formatString("Connection with parent %s failed",getName().c_str()));
    concreteParentDisconnected(port);
  }
  else if(_isChildStartDaemonPort(port, startDaemonKey))
  {
    LOG_ERROR(formatString("Connection with child's startdaemon %s failed",startDaemonKey.c_str()));
  }
  port.setTimer(3L); // retry to open the port
}

void LogicalDevice::_acceptChildConnection()
{
  TPortSharedPtr server(new TRemotePort);
  server->init(*this, m_serverPortName, GCFPortInterface::SPP, LOGICALDEVICE_PROTOCOL);
  m_serverPort.accept(*(server.get()));
  _addChildPort(server);
}

bool LogicalDevice::_isAPCLoaded() const
{
  return m_apcLoaded;
}

void LogicalDevice::_apcLoaded()
{
  m_apcLoaded=true;
}

void LogicalDevice::_doStateTransition(const TLogicalDeviceState& newState, const TLDResult& errorCode)
{
  m_globalError = errorCode;
  switch(newState)
  {
    case LOGICALDEVICE_STATE_DISABLED:
    case LOGICALDEVICE_STATE_INITIAL:
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
    case LOGICALDEVICE_STATE_GOINGDOWN:
      TRAN(LogicalDevice::goingdown_state);
      break;
    default:
      // no transition
      break;
  }
}

string LogicalDevice::_state2String(const TLogicalDeviceState& state)
{
  string stateString("unknown");
  switch(state)
  {
    case LOGICALDEVICE_STATE_DISABLED:
      stateString = LD_STATE_STRING_DISABLED;
      break;
    case LOGICALDEVICE_STATE_INITIAL:
      stateString = LD_STATE_STRING_INITIAL;
      break;
    case LOGICALDEVICE_STATE_IDLE:
      stateString = LD_STATE_STRING_IDLE;
      break;
    case LOGICALDEVICE_STATE_CLAIMING:
      stateString = LD_STATE_STRING_CLAIMING;
      break;
    case LOGICALDEVICE_STATE_CLAIMED:
      stateString = LD_STATE_STRING_CLAIMED;
      break;
    case LOGICALDEVICE_STATE_PREPARING:
      stateString = LD_STATE_STRING_PREPARING;
      break;
    case LOGICALDEVICE_STATE_SUSPENDED:
      stateString = LD_STATE_STRING_SUSPENDED;
      break;
    case LOGICALDEVICE_STATE_ACTIVE:
      stateString = LD_STATE_STRING_ACTIVE;
      break;
    case LOGICALDEVICE_STATE_RELEASING:
      stateString = LD_STATE_STRING_RELEASING;
      break;
    case LOGICALDEVICE_STATE_GOINGDOWN:
      stateString = LD_STATE_STRING_GOINGDOWN;
      break;
  default:
      break;
  }
  return stateString;
}

void LogicalDevice::_handleTimers(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  if(event.signal == F_TIMER)
  {
    GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
    TPortMap::iterator it = _findParentPort(port);
    if(timerEvent.id == m_parentReconnectTimerId && it != m_parentPorts.end())
    {
      m_parentReconnectTimerId = 0;
      it->second->cancelTimer(timerEvent.id);
      it->second->open();
    }
    else if(timerEvent.id == m_claimTimerId)
    {
      m_claimTimerId = 0;
      port.cancelTimer(timerEvent.id);
      LOG_DEBUG(formatString("(%s) ClaimTimer %d triggered and cancelled",__func__,timerEvent.id));
      // this is a claim timer for the schedule of a logical device. claim the device
      _claim();
    }
    else if(timerEvent.id == m_prepareTimerId)
    {
      m_prepareTimerId = 0;
      port.cancelTimer(timerEvent.id);
      LOG_DEBUG(formatString("(%s) PrepareTimer %d triggered and cancelled",__func__,timerEvent.id));
      // this is a prepare timer for the schedule of a logical device. prepare the device
      _prepare();
    }
    else if(timerEvent.id == m_startTimerId)
    {
      m_startTimerId = 0;
      port.cancelTimer(timerEvent.id);
      LOG_DEBUG(formatString("(%s) StartTimer %d triggered and cancelled",__func__,timerEvent.id));
      // this is a start timer for the schedule of a logical device. resume the device
      _resume();
    }
    else if(timerEvent.id == m_stopTimerId)
    {
      m_stopTimerId = 0;
      port.cancelTimer(timerEvent.id);
      LOG_DEBUG(formatString("(%s) StopTimer %d triggered and cancelled",__func__,timerEvent.id));
      // this is a stop timer for the schedule of a logical device. release the device
      _release();
    }
    else if(timerEvent.id == m_retrySendTimerId)
    {
      int32 retryPeriod = 10; // retry sending buffered events every 10 seconds
      if(m_eventBuffer.size() > 0)
      {
        m_retrySendTimerId = 0;
        int32 retryTimeout = 1*60*60; // retry sending buffered events for 1 hour
        GCF::ParameterSet* pParamSet = GCF::ParameterSet::instance();
        try
        {
          retryTimeout = pParamSet->getInt(string(LD_CONFIG_PREFIX + string("retryTimeout")));
          retryPeriod  = pParamSet->getInt(LD_CONFIG_PREFIX + string("retryPeriod"));
        } 
        catch(Exception& e)
        {
          LOG_INFO("Using default retry period of 10s");
        }
  
        // loop through the buffered events and try to send each one.
        TEventBufferVector::iterator it = m_eventBuffer.begin();
        while(it != m_eventBuffer.end())
        {
          ssize_t sentBytes = it->port->send(*(it->event));
          time_t timeNow = time(0);
          
          if((sentBytes == 0 && timeNow - it->entryTime > retryTimeout) || sentBytes != 0)
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
      }
      // keep on polling
      m_retrySendTimerId = m_serverPort.setTimer(static_cast<long int>(retryPeriod));
    }
    else
    {
      concreteHandleTimers(timerEvent,port);
      if(!port.isConnected())
      {
        // try to open the port
        port.open();
      }
    }
  }
}

void LogicalDevice::_handleServerConnected()
{
  // replace my server port (assigned by the ServiceBroker) in all child sections    
  uint16 serverPort = m_serverPort.getPortNumber();
  
  // create the childs
  vector<string> childKeys = _getChildKeys();
  vector<string>::iterator chIt;
  for(chIt=childKeys.begin(); chIt!=childKeys.end();++chIt)
  {
    // connect to child startdaemon.
    try
    {
      string key = (*chIt) + string(".parentPort");
      KVpair kvPair(key,serverPort);
      m_parameterSet.replace(kvPair);
      
      string remoteSystemName    = m_parameterSet.getString((*chIt) + string(".remoteSystem"));
      string startDaemonHostName = m_parameterSet.getString((*chIt) + string(".startDaemonHost"));
      uint16 startDaemonPortNr   = m_parameterSet.getUint16((*chIt) + string(".startDaemonPort"));
      string startDaemonTaskName = m_parameterSet.getString((*chIt) + string(".startDaemonTask"));
      string childPsName         = remoteSystemName + string(":") + m_parameterSet.getString((*chIt) + string(".propertysetBaseName"));
      
      TPortSharedPtr startDaemonPort(new TRemotePort(*this,startDaemonTaskName,GCFPortInterface::SAP,0));
      startDaemonPort->setHostName(startDaemonHostName);
      startDaemonPort->setPortNumber(startDaemonPortNr);
      startDaemonPort->open();
      m_childStartDaemonPorts[(*chIt)] = startDaemonPort;
      
      // add reference in propertyset
      boost::shared_ptr<GCFPVDynArr> childRefs(static_cast<GCFPVDynArr*>(m_basePropertySet->getValue(LD_PROPNAME_CHILDREFS)));
      if(childRefs != 0)
      {
        GCFPValueArray refsVector(childRefs->getValue()); // create a copy 
        GCFPVString newRef((*chIt) + string("=") + childPsName);
        refsVector.push_back(&newRef);
        GCFPVDynArr newChildRefs(LPT_STRING,refsVector);
        m_basePropertySet->setValue(LD_PROPNAME_CHILDREFS,newChildRefs);
        LOG_DEBUG(formatString("Added child reference %s to %s",newRef.getValue().c_str(),m_basePropertySetName.c_str()));
      }
    }
    catch(Exception& e)
    {
      LOG_FATAL(formatString("(%s) Unable to create child %s",e.message().c_str(),(*chIt).c_str()));
    }
  }
  
  _connectParent();
  _schedule();
}

void LogicalDevice::_connectParent()
{
  // connect to the parent if it is a new parent
  string parentTaskName = m_parameterSet.getString("parentTask");
  
  if(m_parentPorts.find(parentTaskName) == m_parentPorts.end())
  {  
    string parentHost     = m_parameterSet.getString("parentHost");//TiMu
    uint16 parentPort     = m_parameterSet.getUint16("parentPort");//TiMu
    
    // it is possible that parents do not yet exist while constructing this LD
    // (e.g. SO starts when a station starts, even if there are no CCU's)
    TPortSharedPtr parentPortPtr(new TRemotePort);
    if(parentTaskName.length() > 0)
    {
      parentPortPtr->init(*this,parentTaskName,GCFPortInterface::SAP, LOGICALDEVICE_PROTOCOL);
      parentPortPtr->setHostName(parentHost);//TiMu
      parentPortPtr->setPortNumber(parentPort);//TiMu
      parentPortPtr->open();
      m_parentPorts[parentTaskName] = parentPortPtr;
    }
/*
    if(parentTaskName.length() > 0)
    {
      // send initialized event to the parent
      boost::shared_ptr<LOGICALDEVICEScheduledEvent> scheduledEvent(new LOGICALDEVICEScheduledEvent);
      scheduledEvent->result=LD_RESULT_NO_ERROR;//OK
      _sendEvent(scheduledEvent,*(parentPortPtr.get()));
    }
*/
  } 
}
 
vector<string> LogicalDevice::_getChildKeys()
{
  string childs;
  vector<string> childKeys;
  try
  {
    childKeys = m_parameterSet.getStringVector("childs");
  }
  catch(Exception& e)
  {
  }
  return childKeys;
}

// Send the event to the port. If it fails, the event is added to a buffer
// The logical device periodically retries to send the events in the buffer
void LogicalDevice::_sendEvent(GCFEventSharedPtr eventPtr, GCFPortInterface& port)
{
  ssize_t sentBytes = port.send(*eventPtr);
  if(sentBytes == 0)
  {
    // add to buffer and keep retrying until it succeeds
    TBufferedEventInfo bufferedEvent(time(0),&port,eventPtr);
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
        ACC::APS::ParameterSet psSubset = m_parameterSet.makeSubset(startDaemonKey + string("."));
        
        concreteAddExtraKeys(psSubset);
        
        string parameterFileName = startDaemonKey+string(".param"); 
        string remoteSystem = psSubset.getString("startDaemonHost");
        
        string tempFileName = APLUtilities::getTempFileName();
        psSubset.writeFile(tempFileName);
        APLUtilities::remoteCopy(tempFileName,remoteSystem,_getShareLocation()+parameterFileName);
        remove(tempFileName.c_str());
  
        // send the schedule to the startdaemon of the child
        string ldTypeString = psSubset.getString("logicalDeviceType");
        TLogicalDeviceTypes ldType = APLUtilities::convertLogicalDeviceType(ldTypeString);
        boost::shared_ptr<STARTDAEMONScheduleEvent> scheduleEvent(new STARTDAEMONScheduleEvent);
        scheduleEvent->logicalDeviceType = ldType;
	//        scheduleEvent->taskName = getName() + string(".") + startDaemonKey;
        scheduleEvent->taskName = startDaemonKey;
        scheduleEvent->fileName = parameterFileName;

        _sendEvent(scheduleEvent,*startDaemonPort);
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
    TPortWeakPtrMap::iterator it = m_connectedChildPorts.begin();
    while(it != m_connectedChildPorts.end())
    {
      try
      {
        // extract the parameterset for the child
        string childKey = it->first;
        if(TPortSharedPtr pChildPort = it->second.lock())
        {
          ACC::APS::ParameterSet psSubset = m_parameterSet.makeSubset(childKey + string("."));
          
          concreteAddExtraKeys(psSubset);
          
          string parameterFileName = childKey+string(".param"); 
          string remoteSystem = psSubset.getString("startDaemonHost");

          string tempFileName = APLUtilities::getTempFileName();
          psSubset.writeFile(tempFileName);
          APLUtilities::remoteCopy(tempFileName,remoteSystem,_getShareLocation()+parameterFileName);
          remove(tempFileName.c_str());
  
          // send the schedule to the child
          boost::shared_ptr<LOGICALDEVICEScheduleEvent> scheduleEvent(new LOGICALDEVICEScheduleEvent);
          scheduleEvent->fileName = parameterFileName;

          _sendEvent(scheduleEvent,*pChildPort);
        }
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
  string shareLocation("/opt/lofar/MAC/parametersets/");
  GCF::ParameterSet* pParamSet = GCF::ParameterSet::instance();
  try
  {
    string tempShareLocation = pParamSet->getString(LD_CONFIG_PREFIX + string("shareLocation"));
    if(tempShareLocation.length()>0)
    {
      if(tempShareLocation[tempShareLocation.length()-1] != '/')
      {
        tempShareLocation+=string("/");
      }
      shareLocation=tempShareLocation;
    }
  } 
  catch(Exception& e)
  {
    LOG_WARN(formatString("(%s) Sharelocation parameter not found. Using %s",e.message().c_str(),shareLocation.c_str()));
  }
  return shareLocation;
}

GCFEvent::TResult LogicalDevice::initial_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case F_INIT:
      break;
    case F_EXIT:
      break;

    case F_ENTRY:
    {
      GCFPVString pvVersion(m_version);
      m_basePropertySet->setValue(LD_PROPNAME_VERSION,pvVersion);
      
      if(m_logicalDeviceState == LOGICALDEVICE_STATE_DISABLED)
      {
        // construction failed, never go out of this state
        GCFPVString state(LD_STATE_STRING_DISABLED);
        m_basePropertySet->setValue(LD_PROPNAME_STATE,state);
      }
      else
      {
        GCFPVString state(LD_STATE_STRING_INITIAL);
        m_basePropertySet->setValue(LD_PROPNAME_STATE,state);
        
        // open the server port to allow childs to connect
        m_serverPort.open();
      }
      break;
    }
    
    case F_CONNECTED:
    {
      if(_findParentPort(port)!=m_parentPorts.end())
      {
        boost::shared_ptr<LOGICALDEVICEConnectEvent> connectEvent(new LOGICALDEVICEConnectEvent);
        connectEvent->nodeId = getName();
        _sendEvent(connectEvent,port);
      }
      else if(_isServerPort(port))
      {
        _handleServerConnected();
        
        // poll retry buffer every 10 seconds
        int32 retryPeriod = 10; // retry sending buffered events every 10 seconds
        m_retrySendTimerId = 0;
        GCF::ParameterSet* pParamSet = GCF::ParameterSet::instance();
        try
        {
          retryPeriod  = pParamSet->getInt(LD_CONFIG_PREFIX + string("retryPeriod"));
        }
        catch(Exception& e)
        {
          LOG_INFO("Using default retry period of 10s");
        }
        m_retrySendTimerId = m_serverPort.setTimer(static_cast<long int>(retryPeriod));
      }
      break;
    }
  
    case LOGICALDEVICE_CONNECT:
    {
      // received from one of the clients
      TPortVector::iterator it = _getChildPort(port);
      if(it != m_childPorts.end())
      {
        LOGICALDEVICEConnectEvent connectEvent(event);
        m_connectedChildPorts[connectEvent.nodeId] = TPortWeakPtr(*it);
        
        boost::shared_ptr<LOGICALDEVICEConnectedEvent> connectedEvent(new LOGICALDEVICEConnectedEvent);
        if(m_logicalDeviceState == LOGICALDEVICE_STATE_DISABLED)
        {
          connectedEvent->result = LD_RESULT_DISABLED;
        }
        else
        {
          connectedEvent->result = LD_RESULT_NO_ERROR;
        }
        _sendEvent(connectedEvent,port);
      }
      break;
    }
      
    case LOGICALDEVICE_CONNECTED:
    {
      // received from parent
      LOGICALDEVICEConnectedEvent connectedEvent(event);
      if(connectedEvent.result == LD_RESULT_NO_ERROR)
      {
        _doStateTransition(LOGICALDEVICE_STATE_IDLE,m_globalError);
      }
      break;
    }
      
    case LOGICALDEVICE_SCHEDULE:
    {
      if(m_logicalDeviceState == LOGICALDEVICE_STATE_DISABLED)
      {
        boost::shared_ptr<LOGICALDEVICEScheduledEvent> responseEvent(new LOGICALDEVICEScheduledEvent);
        responseEvent->result = LD_RESULT_DISABLED;
        _sendEvent(responseEvent,port);
      }
      else
      {
        status = GCFEvent::NOT_HANDLED;
      }
      break;
    }
      
    case LOGICALDEVICE_CANCELSCHEDULE:
    {
      if(m_logicalDeviceState == LOGICALDEVICE_STATE_DISABLED)
      {
        boost::shared_ptr<LOGICALDEVICESchedulecancelledEvent> responseEvent(new LOGICALDEVICESchedulecancelledEvent);
        responseEvent->result = LD_RESULT_DISABLED;
        _sendEvent(responseEvent,port);
      }
      else
      {
        status = GCFEvent::NOT_HANDLED;
      }
      break;
    }
      
    case F_ACCEPT_REQ:
      _acceptChildConnection();
      break;

    case F_DISCONNECTED:
    {
      port.close();
      if(_findParentPort(port)!=m_parentPorts.end())
      {
        m_parentReconnectTimerId = port.setTimer(3L);
      }
      break;
    }
    case F_TIMER:
      _handleTimers(event,port);
      break;
    
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::initial_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }    
  TLogicalDeviceState newState=LOGICALDEVICE_STATE_NOSTATE;
  TLDResult errorCode = m_globalError;
  GCFEvent::TResult concreteStatus;
  concreteStatus = concrete_initial_state(event, port, newState, errorCode);
  if(event.signal != F_EXIT)
  {
    _doStateTransition(newState, errorCode);
  }
  return (status==GCFEvent::HANDLED||concreteStatus==GCFEvent::HANDLED?GCFEvent::HANDLED : GCFEvent::NOT_HANDLED);
}

GCFEvent::TResult LogicalDevice::idle_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
    case F_EXIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_IDLE;
      GCFPVString state(LD_STATE_STRING_IDLE);
      m_basePropertySet->setValue(LD_PROPNAME_STATE,state);
      break;
    }
  
    case F_ACCEPT_REQ:
      _acceptChildConnection();
      break;

    case F_CONNECTED:
    {
      if(_findParentPort(port)!=m_parentPorts.end())
      {
        boost::shared_ptr<LOGICALDEVICEConnectEvent> connectEvent(new LOGICALDEVICEConnectEvent);
        connectEvent->nodeId = getName();
        _sendEvent(connectEvent,port);
      }
      else if(_isServerPort(port))
      {
        _handleServerConnected();
      }
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
      TPortVector::iterator it = _getChildPort(port);
      if(it != m_childPorts.end())
      {
        LOGICALDEVICEConnectEvent connectEvent(event);
        m_connectedChildPorts[connectEvent.nodeId] = TPortWeakPtr(*it);
        
        boost::shared_ptr<LOGICALDEVICEConnectedEvent> connectedEvent(new LOGICALDEVICEConnectedEvent);
        connectedEvent->result = LD_RESULT_NO_ERROR;
        _sendEvent(connectedEvent,port);
      }
      break;
    }
      
    case LOGICALDEVICE_SCHEDULE:
    {
      LOGICALDEVICEScheduleEvent scheduleEvent(event);

      m_parameterSet.adoptFile(_getShareLocation() + scheduleEvent.fileName);
      _schedule();
      
      boost::shared_ptr<LOGICALDEVICEScheduledEvent> scheduledEvent(new LOGICALDEVICEScheduledEvent);
      scheduledEvent->result = LD_RESULT_NO_ERROR;
      _sendEvent(scheduledEvent,port);
      break;
    }
      
    case LOGICALDEVICE_CANCELSCHEDULE:
    {
      _cancelSchedule();
      
      boost::shared_ptr<LOGICALDEVICESchedulecancelledEvent> scheduleCancelledEvent(new LOGICALDEVICESchedulecancelledEvent);
      scheduleCancelledEvent->result = LD_RESULT_NO_ERROR;
      _sendEvent(scheduleCancelledEvent,port);
      break;
    }
      
    case LOGICALDEVICE_CLAIM:
    {
      _doStateTransition(LOGICALDEVICE_STATE_CLAIMING,m_globalError);
      break;
    }
     
    case LOGICALDEVICE_RELEASE:  // release in idle state? at the moment necessary for old style VT
    {
      _doStateTransition(LOGICALDEVICE_STATE_RELEASING,m_globalError);
      break;
    }
    
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::idle_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }    
  TLogicalDeviceState newState=LOGICALDEVICE_STATE_NOSTATE;
  TLDResult errorCode = m_globalError;
  GCFEvent::TResult concreteStatus;
  concreteStatus = concrete_idle_state(event, port, newState, errorCode);
  if(event.signal != F_EXIT)
  {
    _doStateTransition(newState,errorCode);
  }
  return (status==GCFEvent::HANDLED||concreteStatus==GCFEvent::HANDLED?GCFEvent::HANDLED : GCFEvent::NOT_HANDLED);
}

GCFEvent::TResult LogicalDevice::claiming_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
    case F_EXIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_CLAIMING;
      GCFPVString state(LD_STATE_STRING_CLAIMING);
      m_basePropertySet->setValue(LD_PROPNAME_STATE,state);
      
      // send claim event to childs
      _setChildStates(LOGICALDEVICE_STATE_CLAIMING);
//      boost::shared_ptr<LOGICALDEVICEClaimEvent> claimEvent(new LOGICALDEVICEClaimEvent);
//      _sendToAllChilds(claimEvent);
      
      concreteClaim(port);
      break;
    }
  
    case F_CONNECTED:
    {
      if(_findParentPort(port)!=m_parentPorts.end())
      {
        boost::shared_ptr<LOGICALDEVICEConnectEvent> connectEvent(new LOGICALDEVICEConnectEvent);
        connectEvent->nodeId = getName();
        _sendEvent(connectEvent,port);
      }
      break;
    }
    
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
      
    case F_TIMER:
      _handleTimers(event,port);
      break;

    case LOGICALDEVICE_CANCELSCHEDULE:
    {
      _cancelSchedule();
      
      boost::shared_ptr<LOGICALDEVICESchedulecancelledEvent> scheduleCancelledEvent(new LOGICALDEVICESchedulecancelledEvent);
      scheduleCancelledEvent->result = LD_RESULT_NO_ERROR;
      _sendEvent(scheduleCancelledEvent,port);
      break;
    }
      
    case LOGICALDEVICE_RELEASE:
    {
      _doStateTransition(LOGICALDEVICE_STATE_RELEASING,m_globalError);
      break;
    }
    
    case LOGICALDEVICE_CLAIMED:
    {
      LOGICALDEVICEClaimedEvent claimedEvent(event);
      if(claimedEvent.result == LD_RESULT_NO_ERROR)
      {
        _setConnectedChildState(port,LOGICALDEVICE_STATE_CLAIMED);
      }
      else
      {
        _setConnectedChildState(port,LOGICALDEVICE_STATE_IDLE);
      }
      break;
    }
    
    // the LOGICALDEVICE_CLAIMED event cannot result in a transition to 
    // the claimed state here, because the logical device may have several 
    // children that all send their LOGICALDEVICE_CLAIMED message. 
    // The parent can only enter the claimed state when all children have
    // sent their LOGICALDEVICE_CLAIMED message. That cannot be decided in
    // this baseclass.
    
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::claiming_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  TLogicalDeviceState newState=LOGICALDEVICE_STATE_NOSTATE;
  TLDResult errorCode = m_globalError;
  GCFEvent::TResult concreteStatus;
  concreteStatus = concrete_claiming_state(event, port, newState, errorCode);
  if(event.signal != F_EXIT)
  {
    _doStateTransition(newState, errorCode);
  }
  return (status==GCFEvent::HANDLED||concreteStatus==GCFEvent::HANDLED?GCFEvent::HANDLED : GCFEvent::NOT_HANDLED);
}

GCFEvent::TResult LogicalDevice::claimed_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
    case F_EXIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_CLAIMED;
      GCFPVString state(LD_STATE_STRING_CLAIMED);
      m_basePropertySet->setValue(LD_PROPNAME_STATE,state);

      // send claimed message to all parents.
      boost::shared_ptr<LOGICALDEVICEClaimedEvent> claimedEvent(new LOGICALDEVICEClaimedEvent);
      claimedEvent->result = m_globalError;
      for(TPortMap::iterator it = m_parentPorts.begin();it != m_parentPorts.end();++it)
      {
        _sendEvent(claimedEvent,*(it->second.get()));
      }
      break;
    }
    
    case F_CONNECTED:
    {
      if(_findParentPort(port)!=m_parentPorts.end())
      {
        boost::shared_ptr<LOGICALDEVICEConnectEvent> connectEvent(new LOGICALDEVICEConnectEvent);
        connectEvent->nodeId = getName();
        _sendEvent(connectEvent,port);
      }
      break;
    }
    
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
    
    case F_TIMER:
      _handleTimers(event,port);
      break;
   
    case LOGICALDEVICE_SCHEDULE:
    {
      LOGICALDEVICEScheduleEvent scheduleEvent(event);

      m_parameterSet.adoptFile(_getShareLocation() + scheduleEvent.fileName);
      _schedule();
      
      boost::shared_ptr<LOGICALDEVICEScheduledEvent> scheduledEvent(new LOGICALDEVICEScheduledEvent);
      scheduledEvent->result = LD_RESULT_NO_ERROR;
      _sendEvent(scheduledEvent,port);
      break;
    }
      
    case LOGICALDEVICE_CANCELSCHEDULE:
    {
      _cancelSchedule();
      
      boost::shared_ptr<LOGICALDEVICESchedulecancelledEvent> scheduleCancelledEvent(new LOGICALDEVICESchedulecancelledEvent);
      scheduleCancelledEvent->result = LD_RESULT_NO_ERROR;
      _sendEvent(scheduleCancelledEvent,port);
      break;
    }
      
    case LOGICALDEVICE_PREPARE:
    {
      _doStateTransition(LOGICALDEVICE_STATE_PREPARING,m_globalError);
      break;
    }
    
    case LOGICALDEVICE_RELEASE:
    {
      _doStateTransition(LOGICALDEVICE_STATE_RELEASING, m_globalError);
      break;
    }
    
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::claimed_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  TLogicalDeviceState newState=LOGICALDEVICE_STATE_NOSTATE;
  TLDResult errorCode = m_globalError;
  GCFEvent::TResult concreteStatus;
  concreteStatus = concrete_claimed_state(event, port, newState, errorCode);
  if(event.signal != F_EXIT)
  {
    _doStateTransition(newState, errorCode);
  }
  return (status==GCFEvent::HANDLED||concreteStatus==GCFEvent::HANDLED?GCFEvent::HANDLED : GCFEvent::NOT_HANDLED);
}

GCFEvent::TResult LogicalDevice::preparing_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
    case F_EXIT:
    {
      // send prepared message to the parent.
      boost::shared_ptr<LOGICALDEVICEPreparedEvent> preparedEvent(new LOGICALDEVICEPreparedEvent);
      preparedEvent->result = m_globalError;
      for(TPortMap::iterator it = m_parentPorts.begin();it != m_parentPorts.end();++it)
      {
        _sendEvent(preparedEvent,*(it->second.get()));
      }
      break;
    }
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_PREPARING;
      GCFPVString state(LD_STATE_STRING_PREPARING);
      m_basePropertySet->setValue(LD_PROPNAME_STATE,state);

      // send prepare event to childs
      _setChildStates(LOGICALDEVICE_STATE_PREPARING);
//      boost::shared_ptr<LOGICALDEVICEPrepareEvent> prepareEvent(new LOGICALDEVICEPrepareEvent);
//      _sendToAllChilds(prepareEvent);

      concretePrepare(port);
      break;
    }
    
    case F_CONNECTED:
    {
      if(_findParentPort(port)!=m_parentPorts.end())
      {
        boost::shared_ptr<LOGICALDEVICEConnectEvent> connectEvent(new LOGICALDEVICEConnectEvent);
        connectEvent->nodeId = getName();
        _sendEvent(connectEvent,port);
      }
      break;
    }
    
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
      
    case F_TIMER:
      _handleTimers(event,port);
      break;

    case LOGICALDEVICE_CANCELSCHEDULE:
    {
      _cancelSchedule();
      
      boost::shared_ptr<LOGICALDEVICESchedulecancelledEvent> scheduleCancelledEvent(new LOGICALDEVICESchedulecancelledEvent);
      scheduleCancelledEvent->result = LD_RESULT_NO_ERROR;
      _sendEvent(scheduleCancelledEvent,port);
      break;
    }
      
    case LOGICALDEVICE_RELEASE:
    {
      _doStateTransition(LOGICALDEVICE_STATE_RELEASING,m_globalError);
      break;
    }
    
    case LOGICALDEVICE_PREPARED:
    {
      LOGICALDEVICEPreparedEvent preparedEvent(event);
      if(preparedEvent.result == LD_RESULT_NO_ERROR)
      {
        _setConnectedChildState(port,LOGICALDEVICE_STATE_SUSPENDED);
      }
      else
      {
        _setConnectedChildState(port,LOGICALDEVICE_STATE_CLAIMED);
      }
      break;
    }
    
    // the LOGICALDEVICE_PREPARED event cannot result in a transition to 
    // the suspended state here, because the logical device may have several 
    // children that all send their LOGICALDEVICE_PREPARED message. 
    // The parent can only enter the prepared state when all children have
    // sent their LOGICALDEVICE_PREPARED message. That cannot be decided in
    // this baseclass.
    
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::preparing_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  TLogicalDeviceState newState=LOGICALDEVICE_STATE_NOSTATE;
  TLDResult errorCode = m_globalError;
  GCFEvent::TResult concreteStatus;
  concreteStatus = concrete_preparing_state(event, port, newState, errorCode);
  if(event.signal != F_EXIT)
  {
    _doStateTransition(newState, errorCode);
  }
  return (status==GCFEvent::HANDLED||concreteStatus==GCFEvent::HANDLED?GCFEvent::HANDLED : GCFEvent::NOT_HANDLED);
}

GCFEvent::TResult LogicalDevice::suspended_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
    case F_EXIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_SUSPENDED;
      GCFPVString state(LD_STATE_STRING_SUSPENDED);
      m_basePropertySet->setValue(LD_PROPNAME_STATE,state);

      // send suspend event to childs
      boost::shared_ptr<LOGICALDEVICESuspendEvent> suspendEvent(new LOGICALDEVICESuspendEvent);
      _sendToAllChilds(suspendEvent);

      concreteSuspend(port);

      // send to parent
      boost::shared_ptr<LOGICALDEVICESuspendedEvent> suspendedEvent(new LOGICALDEVICESuspendedEvent);
      suspendedEvent->result = m_globalError;
      for(TPortMap::iterator it = m_parentPorts.begin();it != m_parentPorts.end();++it)
      {
        _sendEvent(suspendedEvent,*(it->second.get()));
      }
      break;
    }
  
    case F_CONNECTED:
    {
      if(_findParentPort(port)!=m_parentPorts.end())
      {
        boost::shared_ptr<LOGICALDEVICEConnectEvent> connectEvent(new LOGICALDEVICEConnectEvent);
        connectEvent->nodeId = getName();
        _sendEvent(connectEvent,port);
      }
      break;
    }
    
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
    
    case F_TIMER:
      _handleTimers(event,port);
      break;
    
    case LOGICALDEVICE_CANCELSCHEDULE:
    {
      _cancelSchedule();
      
      boost::shared_ptr<LOGICALDEVICESchedulecancelledEvent> scheduleCancelledEvent(new LOGICALDEVICESchedulecancelledEvent);
      scheduleCancelledEvent->result = LD_RESULT_NO_ERROR;
      _sendEvent(scheduleCancelledEvent,port);
      break;
    }
      
    case LOGICALDEVICE_CLAIM:
      _doStateTransition(LOGICALDEVICE_STATE_CLAIMING,m_globalError);
      break;
      
    case LOGICALDEVICE_PREPARE:
    {
      _doStateTransition(LOGICALDEVICE_STATE_PREPARING,m_globalError);
      break;
    }
    
    case LOGICALDEVICE_RESUME:
    {
      _doStateTransition(LOGICALDEVICE_STATE_ACTIVE,m_globalError);
      break;
    }
    
    case LOGICALDEVICE_RELEASE:
    {
      _doStateTransition(LOGICALDEVICE_STATE_RELEASING,m_globalError);
      break;
    }
    
    case LOGICALDEVICE_SUSPENDED:
    {
      LOGICALDEVICESuspendedEvent suspendedEvent(event);
      if(suspendedEvent.result == LD_RESULT_NO_ERROR)
      {
        _setConnectedChildState(port,LOGICALDEVICE_STATE_SUSPENDED);
      }
      break;
    }
    
    // this happens if a child resumes itself just before the parent
    case LOGICALDEVICE_RESUMED:
    {
      LOGICALDEVICEResumedEvent resumedEvent(event);
      if(resumedEvent.result == LD_RESULT_NO_ERROR)
      {
        _setConnectedChildState(port,LOGICALDEVICE_STATE_ACTIVE);
      }
      else
      {
        _setConnectedChildState(port,LOGICALDEVICE_STATE_SUSPENDED);
      }
      break;
    }
    
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::suspended_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

GCFEvent::TResult LogicalDevice::active_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;      
    case F_EXIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_ACTIVE;
      GCFPVString state(LD_STATE_STRING_ACTIVE);
      m_basePropertySet->setValue(LD_PROPNAME_STATE,state);

      // send resume event to childs
      boost::shared_ptr<LOGICALDEVICEResumeEvent> resumeEvent(new LOGICALDEVICEResumeEvent);
      _sendToAllChilds(resumeEvent);
      
      concreteResume(port);

      // send resumed message to parent.
      boost::shared_ptr<LOGICALDEVICEResumedEvent> resumedEvent(new LOGICALDEVICEResumedEvent);
      resumedEvent->result = m_globalError;
      for(TPortMap::iterator it = m_parentPorts.begin();it != m_parentPorts.end();++it)
      {
        _sendEvent(resumedEvent,*(it->second.get()));
      }
      break;
    }
  
    case F_CONNECTED:
    {
      if(_findParentPort(port)!=m_parentPorts.end())
      {
        boost::shared_ptr<LOGICALDEVICEConnectEvent> connectEvent(new LOGICALDEVICEConnectEvent);
        connectEvent->nodeId = getName();
        _sendEvent(connectEvent,port);
      }
      break;
    }
    
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
    
    case F_TIMER:
      _handleTimers(event,port);
      break;
    
    case LOGICALDEVICE_CANCELSCHEDULE:
    {
      _cancelSchedule();
      
      boost::shared_ptr<LOGICALDEVICESchedulecancelledEvent> scheduleCancelledEvent(new LOGICALDEVICESchedulecancelledEvent);
      scheduleCancelledEvent->result = LD_RESULT_NO_ERROR;
      _sendEvent(scheduleCancelledEvent,port);
      break;
    }
      
    case LOGICALDEVICE_PREPARE:
      // invalid message in this state
      LOG_DEBUG(formatString("LogicalDevice(%s)::active_state, PREPARE NOT ALLOWED",getName().c_str()));
      break;
      
    case LOGICALDEVICE_SUSPEND:
    {
      _doStateTransition(LOGICALDEVICE_STATE_SUSPENDED,m_globalError);
      break;
    }
    
    case LOGICALDEVICE_RELEASE:
    {
      _doStateTransition(LOGICALDEVICE_STATE_RELEASING,m_globalError);
      break;
    }
    
    case LOGICALDEVICE_RESUMED:
    {
      LOGICALDEVICEResumedEvent resumedEvent(event);
      if(resumedEvent.result == LD_RESULT_NO_ERROR)
      {
        _setConnectedChildState(port,LOGICALDEVICE_STATE_ACTIVE);
      }
      else
      {
        _setConnectedChildState(port,LOGICALDEVICE_STATE_SUSPENDED);
      }
      break;
    }
    
    case LOGICALDEVICE_RELEASED:
    {
      _setConnectedChildState(port,LOGICALDEVICE_STATE_IDLE);
      break;
    }
    
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::active_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  GCFEvent::TResult concreteStatus;
  TLDResult errorCode = m_globalError;
  concreteStatus = concrete_active_state(event, port, errorCode);
  return (status==GCFEvent::HANDLED||concreteStatus==GCFEvent::HANDLED?GCFEvent::HANDLED : GCFEvent::NOT_HANDLED);
}

GCFEvent::TResult LogicalDevice::releasing_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
    case F_EXIT:
    {
      // send released message to the parent.
      boost::shared_ptr<LOGICALDEVICEReleasedEvent> releasedEvent(new LOGICALDEVICEReleasedEvent);
      releasedEvent->result = m_globalError;
      for(TPortMap::iterator it = m_parentPorts.begin();it != m_parentPorts.end();++it)
      {
        _sendEvent(releasedEvent,*(it->second.get()));
      }
      break;
    }      
    
    case F_ENTRY:
    {
      // first thing: cancel all timers
      m_serverPort.cancelTimer(m_claimTimerId);
      m_claimTimerId = 0;
      m_serverPort.cancelTimer(m_prepareTimerId);
      m_prepareTimerId = 0;
      m_serverPort.cancelTimer(m_startTimerId);
      m_startTimerId = 0;
      m_serverPort.cancelTimer(m_stopTimerId);
      m_stopTimerId = 0;
      
      m_logicalDeviceState = LOGICALDEVICE_STATE_RELEASING;
      GCFPVString state(LD_STATE_STRING_RELEASING);
      m_basePropertySet->setValue(LD_PROPNAME_STATE,state);

      // send release event to childs
      _setChildStates(LOGICALDEVICE_STATE_RELEASING);
      boost::shared_ptr<LOGICALDEVICEReleaseEvent> releaseEvent(new LOGICALDEVICEReleaseEvent);
      _sendToAllChilds(releaseEvent);

      concreteRelease(port);
      break;
    }
  
    case F_CONNECTED:
    {
      if(_findParentPort(port)!=m_parentPorts.end())
      {
        boost::shared_ptr<LOGICALDEVICEConnectEvent> connectEvent(new LOGICALDEVICEConnectEvent);
        connectEvent->nodeId = getName();
        _sendEvent(connectEvent,port);
      }
      break;
    }
    
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;

    case F_TIMER:
      _handleTimers(event,port);
      break;

    case LOGICALDEVICE_RELEASED:
    {
      LOGICALDEVICEReleasedEvent releasedEvent(event);
      if(releasedEvent.result == LD_RESULT_NO_ERROR)
      {
        _setConnectedChildState(port,LOGICALDEVICE_STATE_IDLE);
      }
      break;
    }
    
    // the LOGICALDEVICE_RELEASED event cannot result in a transition to 
    // the idle state here, because the logical device may have several 
    // children that all send their LOGICALDEVICE_RELEASED message. 
    // The parent can only enter the idle state when all children have
    // sent their LOGICALDEVICE_RELEASED message. That cannot be decided in
    // this baseclass.
    
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::releasing_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  TLogicalDeviceState newState=LOGICALDEVICE_STATE_NOSTATE;
  TLDResult errorCode = m_globalError;
  GCFEvent::TResult concreteStatus;
  concreteStatus = concrete_releasing_state(event, port, newState, errorCode);
  if(event.signal != F_EXIT)
  {
    _doStateTransition(newState, errorCode);
  }
  return (status==GCFEvent::HANDLED||concreteStatus==GCFEvent::HANDLED?GCFEvent::HANDLED : GCFEvent::NOT_HANDLED);
}

GCFEvent::TResult LogicalDevice::goingdown_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
    case F_EXIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_GOINGDOWN;
      GCFPVString state(LD_STATE_STRING_GOINGDOWN);
      m_basePropertySet->setValue(LD_PROPNAME_STATE,state);

      // the startdaemon has created us, so the startdaemon should destroy us.
      STARTDAEMONDestroyLogicaldeviceEvent destroyEvent;
      destroyEvent.name = getName();
      m_startDaemon->dispatch(destroyEvent,port);
      break;
    }
  
    case F_DISCONNECTED:
      port.close();
      break;

    case F_TIMER:
      break;

    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::goingdown_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

};
};
