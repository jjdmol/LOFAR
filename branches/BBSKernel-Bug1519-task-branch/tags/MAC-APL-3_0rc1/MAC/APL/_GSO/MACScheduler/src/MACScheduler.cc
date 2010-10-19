//#  MACScheduler.cc: Implementation of the MAC Scheduler task
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

#include <boost/shared_array.hpp>
#include <GCF/ParameterSet.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVInteger.h>

#include "APLCommon/APLUtilities.h"
#include "MACScheduler.h"

INIT_TRACER_CONTEXT(LOFAR::GSO::MACScheduler,LOFARLOGGER_PACKAGE);

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

namespace LOFAR
{
using namespace APLCommon;

namespace GSO
{
const string MACScheduler::MS_CONFIG_PREFIX            = string("mac.apl.ams.");
const string MACScheduler::MS_TASKNAME                 = string("MACScheduler");

const string MACScheduler::MS_STATE_STRING_INITIAL     = string("Initial");
const string MACScheduler::MS_STATE_STRING_IDLE        = string("Idle");

const string MACScheduler::MS_PROPSET_NAME             = string("GSO_MACScheduler");
const string MACScheduler::MS_PROPSET_TYPE             = string("TAplMacScheduler");
const string MACScheduler::MS_PROPNAME_COMMAND         = string("command");
const string MACScheduler::MS_PROPNAME_STATUS          = string("status");

const string MACScheduler::MS_COMMAND_SCHEDULE         = string("SCHEDULE");
const string MACScheduler::MS_COMMAND_UPDATESCHEDULE   = string("UPDATESCHEDULE");
const string MACScheduler::MS_COMMAND_CANCELSCHEDULE   = string("CANCELSCHEDULE");

MACScheduler::MACScheduler() :
  GCFTask((State)&MACScheduler::initial_state,MS_TASKNAME),
  PropertySetAnswerHandlerInterface(),
  m_propertySetAnswer(*this),
  m_propertySet(),
  m_SASserverPortName(string("SAS_server")),
  m_SASserverPort(*this, m_SASserverPortName, GCFPortInterface::MSPP, SAS_PROTOCOL),
  m_SASclientPorts(),
  m_VISDclientPorts(),
  m_VIparentPortName(string("VIparent_server")),
  m_VIparentPort(*this, m_VIparentPortName, GCFPortInterface::MSPP, LOGICALDEVICE_PROTOCOL),
  m_VIclientPorts(),
  m_connectedVIclientPorts(),
  m_VItoSASportMap(),
#ifndef ACC_CONFIGURATIONMGR_UNAVAILABLE
  m_configurationManager(),
#endif // ACC_CONFIGURATIONMGR_UNAVAILABLE
  m_beamletAllocator(128)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
#ifdef USE_TCPPORT_INSTEADOF_PVSSPORT
  LOG_WARN("Using GCFTCPPort in stead of GCFPVSSPort");
#endif

  registerProtocol(LOGICALDEVICE_PROTOCOL, LOGICALDEVICE_PROTOCOL_signalnames);
  registerProtocol(STARTDAEMON_PROTOCOL, STARTDAEMON_PROTOCOL_signalnames);
  registerProtocol(SAS_PROTOCOL, SAS_PROTOCOL_signalnames);

  m_propertySet = boost::shared_ptr<GCFMyPropertySet>(new GCFMyPropertySet(
      MS_PROPSET_NAME.c_str(),
      MS_PROPSET_TYPE.c_str(),
      PS_CAT_TEMPORARY,
      &m_propertySetAnswer));
  m_propertySet->enable();
  
  GCF::ParameterSet* pParamSet = GCF::ParameterSet::instance();
  try
  {
    string hostname = pParamSet->getString(MS_CONFIG_PREFIX + string("OTDBhostname"));
    string databasename = pParamSet->getString(MS_CONFIG_PREFIX + string("OTDBdatabasename"));
    string password = pParamSet->getString(MS_CONFIG_PREFIX + string("OTDBpassword"));
    
#ifndef ACC_CONFIGURATIONMGR_UNAVAILABLE
    m_configurationManager = new ACC::ConfigurationMgr(hostname,databasename,password);
#else // ACC_CONFIGURATIONMGR_UNAVAILABLE
    LOG_FATAL("TODO: Use ACC::ConfigurationMgr to access OTDB database");
#endif // ACC_CONFIGURATIONMGR_UNAVAILABLE
  } 
  catch(Exception& e)
  {
    LOG_WARN(formatString("OTDB configuration not found. Using parameterfile",e.message().c_str()));
#ifndef ACC_CONFIGURATIONMGR_UNAVAILABLE
    m_configurationManager = new ACC::ConfigurationMgr;
#else // ACC_CONFIGURATIONMGR_UNAVAILABLE
    LOG_FATAL("TODO: Use ACC::ConfigurationMgr to access OTDB database");
#endif // ACC_CONFIGURATIONMGR_UNAVAILABLE
  }
}


MACScheduler::~MACScheduler()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  m_propertySet->disable();
}

void MACScheduler::handlePropertySetAnswer(GCFEvent& answer)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  switch(answer.signal)
  {
    case F_MYPS_ENABLED:
    {
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
      if(pPropAnswer->result == GCF_NO_ERROR)
      {
        // property set loaded, now load apc?
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
        LOG_DEBUG(formatString("%s : apc %s Loaded",getName().c_str(),pConfAnswer->pApcName));
        //apcLoaded();
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
      if ((pPropAnswer->pValue->getType() == LPT_STRING) &&
          (strstr(pPropAnswer->pPropName, MS_PROPNAME_COMMAND.c_str()) != 0))
      {
        // command received
        string commandString(((GCFPVString*)pPropAnswer->pValue)->getValue());
        vector<string> parameters;
        string command;
        APLUtilities::decodeCommand(commandString,command,parameters);
        
        // SCHEDULE <nodeId>
        if(command==string(MS_COMMAND_SCHEDULE))
        {
          if(parameters.size()==1)
          {
            _schedule(parameters[0]);
          }
          else
          {
            TSASResult result = SAS_RESULT_ERROR_INCORRECT_NUMBER_OF_PARAMETERS;
            m_propertySet->setValue(MS_PROPNAME_STATUS,GCFPVInteger(result));
          }
        }
        // UPDATESCHEDULE <nodeId>
        else if(command==string(MS_COMMAND_UPDATESCHEDULE))
        {
          if(parameters.size()==1)
          {
            _updateSchedule(parameters[0]);
          }
          else
          {
            TSASResult result = SAS_RESULT_ERROR_INCORRECT_NUMBER_OF_PARAMETERS;
            m_propertySet->setValue(MS_PROPNAME_STATUS,GCFPVInteger(result));
          }
        }
        // CANCELSCHEDULE <nodeId>
        else if(command==string(MS_COMMAND_CANCELSCHEDULE))
        {
          if(parameters.size()==1)
          {
            _cancelSchedule(parameters[0]);
          }
          else
          {
            TSASResult result = SAS_RESULT_ERROR_INCORRECT_NUMBER_OF_PARAMETERS;
            m_propertySet->setValue(MS_PROPNAME_STATUS,GCFPVInteger(result));
          }
        }
        else
        {
          TSASResult result = SAS_RESULT_ERROR_UNKNOWN_COMMAND;
          m_propertySet->setValue(MS_PROPNAME_STATUS,GCFPVInteger(result));
        }
      }
      break;
    }  

    default:
      break;
  }  
}

bool MACScheduler::_isServerPort(const GCFPortInterface& server, const GCFPortInterface& port) const
{
  return (&port == &server); // comparing two pointers. yuck?
}
   
MACScheduler::TTCPPortVector::const_iterator MACScheduler::_getSASclientPort(const GCFPortInterface& port) const
{
  bool found=false;
  TTCPPortVector::const_iterator it=m_SASclientPorts.begin();
  while(!found && it != m_SASclientPorts.end())
  {
    found = (&port == (*it).get()); // comparing two pointers. yuck?
    if(!found)
      ++it;
  }
  return it;
}

bool MACScheduler::_isSASclientPort(const GCFPortInterface& port) const
{
  return (_getSASclientPort(port) != m_SASclientPorts.end());
}

bool MACScheduler::_isVISDclientPort(const GCFPortInterface& port, string& visd) const
{
  bool found=false;
  TStringRemotePortMap::const_iterator it=m_VISDclientPorts.begin();
  while(!found && it != m_VISDclientPorts.end())
  {
    found = (&port == it->second.get()); // comparing two pointers. yuck?
    if(found)
    {
      visd = it->first;
    }
    ++it;
  }
  return found;
}

bool MACScheduler::_isVIclientPort(const GCFPortInterface& port) const
{
  bool found=false;
  TRemotePortVector::const_iterator it=m_VIclientPorts.begin();
  while(!found && it != m_VIclientPorts.end())
  {
    found = (&port == (*it).get()); // comparing two pointers. yuck?
    ++it;
  }
  return found;
}

string MACScheduler::_getVInameFromPort(const GCF::TM::GCFPortInterface& port) const
{
  string viName("");
  if(_isVIclientPort(port))
  {
    bool found = false;
    TStringRemotePortMap::const_iterator it = m_connectedVIclientPorts.begin();
    while(!found && it != m_connectedVIclientPorts.end())
    {
      found = (&port == it->second.get());
      if(found)
      {
        viName = it->first;
      }
      ++it;
    }
  }
  return viName;
}

void MACScheduler::_disconnectedHandler(GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  string visd;
  port.close();
  if(_isServerPort(m_SASserverPort,port))
  {
    LOG_FATAL("SAS server closed");
    m_SASserverPort.open(); // server closed? reopen it
  }
  else if(_isServerPort(m_VIparentPort,port))
  {
    LOG_FATAL("VI parent server closed");
    m_VIparentPort.open(); // server closed? reopen it
  }
  else if(_isSASclientPort(port))
  {
    LOG_FATAL("SAS client port disconnected");
  }
  else if(_isVISDclientPort(port,visd))
  {
    LOG_FATAL(formatString("VI Startdaemon port disconnected: %s",visd.c_str()));
    port.setTimer(3L);
  }
  else if(_isVIclientPort(port))
  {
    LOG_FATAL("VI client port disconnected");
    // do something with the nodeId?
  }
}

string MACScheduler::_getShareLocation() const
{
  string shareLocation("/opt/lofar/MAC/parametersets/");
  GCF::ParameterSet* pParamSet = GCF::ParameterSet::instance();
  try
  {
    string tempShareLocation = pParamSet->getString(MS_CONFIG_PREFIX + string("shareLocation"));
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

TSASResult MACScheduler::_LDtoSASresult(const TLDResult& ldResult)
{
  TSASResult sasResult;
  switch(ldResult)
  {
    case LD_RESULT_NO_ERROR:
      sasResult = SAS_RESULT_NO_ERROR;
      break;
    case LD_RESULT_UNSPECIFIED:
      sasResult = SAS_RESULT_ERROR_UNSPECIFIED;
      break;
    case LD_RESULT_FILENOTFOUND:
      sasResult = SAS_RESULT_ERROR_VI_NOT_FOUND;
      break;
    case LD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS:
      sasResult = SAS_RESULT_ERROR_INCORRECT_NUMBER_OF_PARAMETERS;
      break;
    case LD_RESULT_UNKNOWN_COMMAND:
      sasResult = SAS_RESULT_ERROR_UNKNOWN_COMMAND;
      break;
    case LD_RESULT_DISABLED:
      sasResult=SAS_RESULT_ERROR_DISABLED;
      break;
    case LD_RESULT_LOW_QUALITY:
      sasResult=SAS_RESULT_ERROR_LOW_QUALITY;
      break;
    case LD_RESULT_TIMING_FAILURE:
      sasResult=SAS_RESULT_ERROR_TIMING_FAILURE;
      break;
    default:
      sasResult=SAS_RESULT_ERROR_UNSPECIFIED;
      break;
  }
  return sasResult;
}

TSASResult MACScheduler::_SDtoSASresult(const TSDResult& sdResult)
{
  TSASResult sasResult;
  switch(sdResult)
  {
    case SD_RESULT_NO_ERROR:
      sasResult = SAS_RESULT_NO_ERROR;
      break;
    case SD_RESULT_UNSPECIFIED_ERROR:
      sasResult = SAS_RESULT_ERROR_UNSPECIFIED;
      break;
    case SD_RESULT_UNSUPPORTED_LD:
      sasResult = SAS_RESULT_ERROR_UNSUPPORTED_LD;
      break;
    case SD_RESULT_FILENOTFOUND:
      sasResult = SAS_RESULT_ERROR_VI_NOT_FOUND;
      break;
    case SD_RESULT_PARAMETERNOTFOUND:
      sasResult = SAS_RESULT_ERROR_PARAMETERNOTFOUND;
      break;
    case SD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS:
      sasResult = SAS_RESULT_ERROR_INCORRECT_NUMBER_OF_PARAMETERS;
      break;
    case SD_RESULT_UNKNOWN_COMMAND:
      sasResult = SAS_RESULT_ERROR_UNKNOWN_COMMAND;
      break;
    case SD_RESULT_ALREADY_EXISTS:
      sasResult = SAS_RESULT_ERROR_ALREADY_EXISTS;
      break;
    case SD_RESULT_LD_NOT_FOUND:
      sasResult = SAS_RESULT_ERROR_LD_NOT_FOUND;
      break;
    case SD_RESULT_WRONG_STATE:
      sasResult = SAS_RESULT_ERROR_WRONG_STATE;
      break;
    case SD_RESULT_SHUTDOWN:
      sasResult = SAS_RESULT_ERROR_SHUTDOWN;
      break;
    case SD_RESULT_WRONG_VERSION:
      sasResult = SAS_RESULT_ERROR_WRONG_VERSION;
      break;

    default:
      sasResult=SAS_RESULT_ERROR_UNSPECIFIED;
      break;
  }
  return sasResult;
}

GCFEvent::TResult MACScheduler::initial_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      m_propertySet->setValue(MS_PROPNAME_STATUS,GCFPVInteger(SAS_RESULT_NO_ERROR));
      
      // connect to startdaemon clients
      GCF::ParameterSet* pParamSet = GCF::ParameterSet::instance();
      try
      {
        int sds = pParamSet->getInt(MS_CONFIG_PREFIX + string("numberOfVIStartDaemons"));
        for(int i=0;i<sds;i++)
        {
          char ccuName[20];
          sprintf(ccuName,"CCU%d",i+1);
          
          string startDaemonHostName = pParamSet->getString(MS_CONFIG_PREFIX + string(ccuName) + string(".startDaemonHost"));
          string startDaemonPortName = pParamSet->getString(MS_CONFIG_PREFIX + string(ccuName) + string(".startDaemonPort"));
          string startDaemonTaskName = pParamSet->getString(MS_CONFIG_PREFIX + string(ccuName) + string(".startDaemonTask"));

          TRemotePortPtr startDaemonPort(new TRemotePort(*this,startDaemonTaskName,GCFPortInterface::SAP,STARTDAEMON_PROTOCOL));
          TPeerAddr peerAddr;
          peerAddr.taskname = startDaemonTaskName;
          peerAddr.portname = startDaemonPortName;
          startDaemonPort->setAddr(peerAddr);
          startDaemonPort->open();
          m_VISDclientPorts[startDaemonHostName] = startDaemonPort;
        }
      }
      catch(Exception& e)
      {
        LOG_FATAL("Unable to read parameter");
      }
                  
      TRAN(MACScheduler::idle_state);
      
      break;
    }
  
    default:
      LOG_DEBUG(formatString("MACScheduler(%s)::initial_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }    
  return status;
}

GCFEvent::TResult MACScheduler::idle_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      // open server ports
      m_SASserverPort.open();
      m_VIparentPort.open();
      break;
    }
  
    case F_ACCEPT_REQ:
    {
      if(port.getProtocol() == SAS_PROTOCOL)
      {
        TTCPPortPtr server(new GCFTCPPort);
        server->init(*this, m_SASserverPortName, GCFPortInterface::SPP, SAS_PROTOCOL);
        m_SASserverPort.accept(*(server.get()));
        m_SASclientPorts.push_back(server);
      }
      else if(port.getProtocol() == LOGICALDEVICE_PROTOCOL)
      {
        TRemotePortPtr client(new TRemotePort);
        client->init(*this, m_VIparentPortName, GCFPortInterface::SPP, LOGICALDEVICE_PROTOCOL);
        m_VIparentPort.accept(*(client.get()));
        m_VIclientPorts.push_back(client);
      }
      break;
    }

    case F_CONNECTED:
      break;

    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
    
    case F_TIMER:
    {
      GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
      // no need to check for timerID because only one timer is used
      port.cancelTimer(timerEvent.id);
      port.open();
      break;
    }

    case SAS_SCHEDULE:
    case SAS_CANCELSCHEDULE:
    case SAS_UPDATESCHEDULE:
      _handleSASprotocol(event,port);
      break;

    case LOGICALDEVICE_CONNECT:
    {
      LOGICALDEVICEConnectEvent connectEvent(event);
      TRemotePortPtr portPtr(static_cast<TRemotePort*>(&port));
      m_connectedVIclientPorts[connectEvent.nodeId] = portPtr;
      
      LOGICALDEVICEConnectedEvent connectedEvent;
      connectedEvent.result = LD_RESULT_NO_ERROR;
      port.send(connectedEvent);
      break;
    }
    
    case LOGICALDEVICE_SCHEDULED:
    {
      LOGICALDEVICEScheduledEvent scheduledEvent(event);
      SASResponseEvent sasResponseEvent;
      sasResponseEvent.result = _LDtoSASresult(scheduledEvent.result);

      string viName = _getVInameFromPort(port);
      sasResponseEvent.VIrootID = viName;

      TStringTCPportMap::iterator it = m_VItoSASportMap.find(viName);
      if(it != m_VItoSASportMap.end())
      {
        it->second->send(sasResponseEvent);
      }
      
      m_propertySet->setValue(MS_PROPNAME_STATUS,GCFPVInteger(sasResponseEvent.result));
      break;
    }
      
    case STARTDAEMON_SCHEDULED:
    {
      STARTDAEMONScheduledEvent scheduledEvent(event);
      SASResponseEvent sasResponseEvent;
      sasResponseEvent.result = _SDtoSASresult(scheduledEvent.result);

      string viName = scheduledEvent.VIrootID;
      sasResponseEvent.VIrootID = viName;

      TStringTCPportMap::iterator it = m_VItoSASportMap.find(viName);
      if(it != m_VItoSASportMap.end())
      {
        it->second->send(sasResponseEvent);
      }
      
      m_propertySet->setValue(MS_PROPNAME_STATUS,GCFPVInteger(sasResponseEvent.result));
      break;
    }
      
    case LOGICALDEVICE_SCHEDULECANCELLED:
    {
      LOGICALDEVICESchedulecancelledEvent schedulecancelledEvent(event);
      SASResponseEvent sasResponseEvent;
      sasResponseEvent.result = _LDtoSASresult(schedulecancelledEvent.result);
      
      string viName = _getVInameFromPort(port);
      sasResponseEvent.VIrootID = viName;

      TStringTCPportMap::iterator it = m_VItoSASportMap.find(viName);
      if(it != m_VItoSASportMap.end())
      {
        it->second->send(sasResponseEvent);
      }
      
      m_propertySet->setValue(MS_PROPNAME_STATUS,GCFPVInteger(sasResponseEvent.result));
      
      break;
    }
      
    default:
      LOG_DEBUG(formatString("MACScheduler(%s)::idle_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void MACScheduler::_handleSASprotocol(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());

  switch (event.signal)
  {
    case SAS_SCHEDULE:
    {
      // schedule event received from SAS
      SASScheduleEvent sasScheduleEvent(event);
      _schedule(sasScheduleEvent.VIrootID, &port);
      break;
    }
    case SAS_CANCELSCHEDULE:
    {
      // schedule event received from SAS
      SASCancelscheduleEvent sasCancelScheduleEvent(event);
      _cancelSchedule(sasCancelScheduleEvent.VIrootID);
      break;
    }
    
    case SAS_UPDATESCHEDULE:
    {
      // schedule event received from SAS
      SASUpdatescheduleEvent sasUpdateScheduleEvent(event);
      _updateSchedule(sasUpdateScheduleEvent.VIrootID,&port);
      break;
    }
    
    default:
      break;
  }
}

void MACScheduler::_convertRelativeTimes(boost::shared_ptr<ACC::APS::ParameterSet> ps)
{
  try
  {
    _convertRelativeTimesChild("",ps);
  }
  catch(Exception& e)
  {
  }
}

void MACScheduler::_convertRelativeTimesChild(string child, boost::shared_ptr<ACC::APS::ParameterSet> ps)
{
  string keyPrefix("");
  if(child.length() > 0)
  {
    keyPrefix=child + string(".");
  }
  string claimKey(keyPrefix + string("claimTime"));
  string prepareKey(keyPrefix + string("prepareTime"));
  string startKey(keyPrefix + string("startTime"));
  string stopKey(keyPrefix + string("stopTime"));
  string childsKey(keyPrefix + string("childs"));
  
  time_t claimTime   = APLUtilities::decodeTimeString(ps->getString(claimKey));
  time_t prepareTime = APLUtilities::decodeTimeString(ps->getString(prepareKey));
  time_t startTime   = APLUtilities::decodeTimeString(ps->getString(startKey));
  time_t stopTime    = APLUtilities::decodeTimeString(ps->getString(stopKey));

  ACC::APS::KVpair kvPairClaimTime(claimKey,claimTime);
  ps->replace(kvPairClaimTime);
  ACC::APS::KVpair kvPairPrepareTime(prepareKey,prepareTime);
  ps->replace(kvPairPrepareTime);
  ACC::APS::KVpair kvPairStartTime(startKey,startTime);
  ps->replace(kvPairStartTime);
  ACC::APS::KVpair kvPairStopTime(stopKey,stopTime);
  ps->replace(kvPairStopTime);

  // propagate into the child keys
  vector<string> childKeys;
  childKeys = ps->getStringVector(childsKey);
  vector<string>::iterator chIt;
  for(chIt=childKeys.begin(); chIt!=childKeys.end();++chIt)
  {
    try
    {
      _convertRelativeTimesChild(*chIt,ps);
    }
    catch(Exception& e)
    {
    }
  }
}

bool MACScheduler::_allocateBeamlets(const string& VIrootID, boost::shared_ptr<ACC::APS::ParameterSet> ps, const string& prefix)
{
  LOG_DEBUG(formatString("Allocating beamlets for VI:%s",VIrootID.c_str()));
  bool allocationOk(false);
  
  try
  {
    m_beamletAllocator.logAllocation();
  
    vector<string> childKeys;
    vector<string> stations;
    vector<int16> subbandsVector;
    map<string,string> station2vtKeyMap;

    boost::shared_ptr<ACC::APS::ParameterSet> psVI(new ACC::APS::ParameterSet(ps->makeSubset(prefix + string("."))));

    childKeys         = psVI->getStringVector("childs");
    subbandsVector    = psVI->getInt16Vector("subbands");
    time_t startTime  = psVI->getInt32("claimTime");
    time_t stopTime   = psVI->getInt32("stopTime");
    
    for(vector<string>::iterator childsIt=childKeys.begin();childsIt!=childKeys.end();++childsIt)
    {
      string ldTypeString = psVI->getString(*childsIt + ".logicalDeviceType");
      TLogicalDeviceTypes ldType = APLUtilities::convertLogicalDeviceType(ldTypeString);
      if(ldType == LDTYPE_VIRTUALTELESCOPE)
      {
        string station = psVI->getString(*childsIt + ".remoteSystem");
        stations.push_back(station);
        station2vtKeyMap[station] = *childsIt;
      }
    }
    
    BeamletAllocator::TStationBeamletAllocation allocation;
    allocationOk = m_beamletAllocator.allocateBeamlets(
      VIrootID,
      stations,
      startTime,
      stopTime,
      subbandsVector,
      allocation);
      
    // do something with it
    if(allocationOk)
    {
      m_beamletAllocator.logAllocation(true);
    
      BeamletAllocator::TStationBeamletAllocation::iterator allocIt;
      for(allocIt = allocation.begin(); allocIt != allocation.end(); ++allocIt)
      {
        map<string,string>::iterator keyIt = station2vtKeyMap.find(allocIt->first);
        if(keyIt != station2vtKeyMap.end())
        {
          LOG_DEBUG(formatString("writing beamlet allocation on station %s to %s",allocIt->first.c_str(),keyIt->second.c_str()));
          string beamlets;
          APLUtilities::vector2String(allocIt->second,beamlets,',');
          beamlets = string("[") + beamlets + string("]");
          ps->replace(prefix + string(".") + keyIt->second + string(".beamlets"), beamlets);
        }
      }
    }
  }
  catch(Exception& e)
  {
    LOG_FATAL(formatString("Error allocating beamlets: %s",e.message().c_str()));
  }

  return allocationOk;
}

void MACScheduler::_schedule(const string& VIrootID, GCFPortInterface* port)
{  
  string shareLocation = _getShareLocation();
  try
  {
    // read the parameterset from the database:
#ifndef ACC_CONFIGURATIONMGR_UNAVAILABLE
    boost::shared_ptr<ACC::APS::ParameterSet> ps(m_configurationManager->getPS(VIrootID, "latest");
#else // ACC_CONFIGURATIONMGR_UNAVAILABLE
    LOG_FATAL("TODO: Use ACC::ConfigurationMgr to access OTDB database");
    // When the ACC::ConfigurationMgr can be used, then the following code is obsolete:
    boost::shared_ptr<ACC::APS::ParameterSet> ps(new ACC::APS::ParameterSet(shareLocation + string("source/") + VIrootID)); // assume VIrootID is a file
    // End of soon to be obsolete code
#endif // ACC_CONFIGURATIONMGR_UNAVAILABLE

    // replace the parent port (assigned by the ServiceBroker)
    unsigned int parentPort = m_VIparentPort.getPortNumber();
    ACC::APS::KVpair kvPair(string("parentPort"),(int)parentPort);
    ps->replace(kvPair);
    
    // get some parameters and write it to the allocated CCU
    string allocatedCCU = ps->getString("allocatedCCU");
    string viName = ps->getString("name");
    string parameterFileName = viName + string(".param");
    
    // make all relative times absolute
    _convertRelativeTimes(ps);

    string ldTypeString = ps->getString("logicalDeviceType");
    TLogicalDeviceTypes ldTypeRoot = APLUtilities::convertLogicalDeviceType(ldTypeString);
    
    bool beamletsAllocated = true;
    
    // find the subbands allocations in VI sections
    vector<string> childKeys;
    childKeys         = ps->getStringVector("childs");
    for(vector<string>::iterator childsIt=childKeys.begin();beamletsAllocated && childsIt!=childKeys.end();++childsIt)
    {
      string ldTypeString = ps->getString(*childsIt + ".logicalDeviceType");
      TLogicalDeviceTypes ldType = APLUtilities::convertLogicalDeviceType(ldTypeString);
      if(ldType == LDTYPE_VIRTUALINSTRUMENT)
      {
	//        boost::shared_ptr<ACC::APS::ParameterSet> psVI(new ACC::APS::ParameterSet(ps->makeSubset(*childsIt + string("."))));
        // allocate beamlets for VI's
        beamletsAllocated = _allocateBeamlets(VIrootID, ps, *childsIt);
        if(!beamletsAllocated)
        {
          SASResponseEvent sasResponseEvent;
          sasResponseEvent.result = SAS_RESULT_ERROR_BEAMLET_ALLOCATION_FAILED;
          sasResponseEvent.VIrootID = VIrootID;
  
          if(port != 0)
          {
            port->send(sasResponseEvent);      
          }
          m_propertySet->setValue(MS_PROPNAME_STATUS,GCFPVInteger(sasResponseEvent.result));
        }
      }
    }
    if(beamletsAllocated)
    {
      string tempFileName = APLUtilities::getTempFileName();
      ps->writeFile(tempFileName);
      APLUtilities::remoteCopy(tempFileName,allocatedCCU,shareLocation+parameterFileName);
      remove(tempFileName.c_str());

      if(port != 0)
      {
        // add the VI to the VI-SASport map
        if(_isSASclientPort(*port))
        {
          m_VItoSASportMap[viName] = *_getSASclientPort(*port);
        }
      }
      // send the schedule event to the VI-StartDaemon on the CCU
      STARTDAEMONScheduleEvent sdScheduleEvent;
      sdScheduleEvent.logicalDeviceType = ldTypeRoot;
      sdScheduleEvent.taskName = viName;
      sdScheduleEvent.fileName = parameterFileName;
    
      TStringRemotePortMap::iterator it = m_VISDclientPorts.find(allocatedCCU);
      if(it != m_VISDclientPorts.end())
      {
        it->second->send(sdScheduleEvent);
      }
      else
      {
        SASResponseEvent sasResponseEvent;
        sasResponseEvent.result = SAS_RESULT_ERROR_VI_NOT_FOUND;
        sasResponseEvent.VIrootID = VIrootID;
        
        if(port != 0)
        {
          port->send(sasResponseEvent);      
        }
        m_propertySet->setValue(MS_PROPNAME_STATUS,GCFPVInteger(sasResponseEvent.result));
      }
    }        
  }
  catch(Exception& e)
  {
    LOG_FATAL(formatString("Error reading schedule parameters: %s",e.message().c_str()));
    SASResponseEvent sasResponseEvent;
    sasResponseEvent.result = SAS_RESULT_ERROR_UNSPECIFIED;
    sasResponseEvent.VIrootID = VIrootID;
    
    if(port != 0)
    {
      port->send(sasResponseEvent);
    }
    m_propertySet->setValue(MS_PROPNAME_STATUS,GCFPVInteger(sasResponseEvent.result));
  }
}

void MACScheduler::_updateSchedule(const string& VIrootID, GCFPortInterface* port)
{
  string shareLocation = _getShareLocation();
  
  // search the port of the VI
  try
  {
    // read the parameterset from the database:
#ifndef ACC_CONFIGURATIONMGR_UNAVAILABLE
    boost::shared_ptr<ACC::APS::ParameterSet> ps(m_configurationManager->getPS(VIrootID, "latest");
#else // ACC_CONFIGURATIONMGR_UNAVAILABLE
    LOG_FATAL("TODO: Use ACC::ConfigurationMgr to access OTDB database");
    // When the ACC::ConfigurationMgr can be used, then the following code is obsolete:
    boost::shared_ptr<ACC::APS::ParameterSet> ps(new ACC::APS::ParameterSet(shareLocation + string("source/") + VIrootID)); // assume VIrootID is a file
    // End of soon to be obsolete code
#endif // ACC_CONFIGURATIONMGR_UNAVAILABLE
    
    // replace the parent port (assigned by the ServiceBroker)
    unsigned int parentPort = m_VIparentPort.getPortNumber();
    ACC::APS::KVpair kvPair(string("parentPort"),(int)parentPort);
    ps->replace(kvPair);
    
    string allocatedCCU = ps->getString("allocatedCCU");
    string viName = ps->getString("name");
    string parameterFileName = viName + string(".param");
  
    // make all relative times absolute
    _convertRelativeTimes(ps);
  
    string tempFileName = APLUtilities::getTempFileName();
    ps->writeFile(tempFileName);
    APLUtilities::remoteCopy(tempFileName,allocatedCCU,shareLocation+parameterFileName);
    remove(tempFileName.c_str());
    
    // send a SCHEDULE message
    TStringRemotePortMap::iterator it = m_connectedVIclientPorts.find(viName);
    if(it != m_connectedVIclientPorts.end())
    {
      LOGICALDEVICEScheduleEvent scheduleEvent;
      scheduleEvent.fileName = parameterFileName;
  
      it->second->send(scheduleEvent);
    }
    else
    {
      SASResponseEvent sasResponseEvent;
      sasResponseEvent.result = SAS_RESULT_ERROR_VI_NOT_FOUND;
      sasResponseEvent.VIrootID = VIrootID;
      
      if(port != 0)
      {
        port->send(sasResponseEvent);
      }
      m_propertySet->setValue(MS_PROPNAME_STATUS,GCFPVInteger(sasResponseEvent.result));
    }        
  }
  catch(Exception& e)
  {
    LOG_FATAL(formatString("Error reading schedule parameters: %s",e.message().c_str()));
    SASResponseEvent sasResponseEvent;
    sasResponseEvent.result = SAS_RESULT_ERROR_UNSPECIFIED;
    sasResponseEvent.VIrootID = VIrootID;
  
    if(port != 0)
    {
      port->send(sasResponseEvent);      
    }
    m_propertySet->setValue(MS_PROPNAME_STATUS,GCFPVInteger(sasResponseEvent.result));
  }
}

void MACScheduler::_cancelSchedule(const string& VIrootID, GCFPortInterface* port)
{
  string shareLocation = _getShareLocation();
  
  // search the port of the VI
  try
  {
    // read the parameterset from the database:
#ifndef ACC_CONFIGURATIONMGR_UNAVAILABLE
    boost::shared_ptr<ACC::APS::ParameterSet> ps(m_configurationManager->getPS(VIrootID, "latest");
#else // ACC_CONFIGURATIONMGR_UNAVAILABLE
    LOG_FATAL("TODO: Use ACC::ConfigurationMgr to access OTDB database");
    // When the ACC::ConfigurationMgr can be used, then the following code is obsolete:
    boost::shared_ptr<ACC::APS::ParameterSet> ps(new ACC::APS::ParameterSet(shareLocation + string("source/") + VIrootID)); // assume VIrootID is a file
    // End of soon to be obsolete code
#endif // ACC_CONFIGURATIONMGR_UNAVAILABLE
    
    string viName = ps->getString("name");
    
    // send a CANCELSCHEDULE message
    TStringRemotePortMap::iterator it = m_connectedVIclientPorts.find(viName);
    if(it != m_connectedVIclientPorts.end())
    {
      LOGICALDEVICECancelscheduleEvent cancelScheduleEvent;
      it->second->send(cancelScheduleEvent);
    }
    else
    {
      SASResponseEvent sasResponseEvent;
      sasResponseEvent.result = SAS_RESULT_ERROR_VI_NOT_FOUND;
      sasResponseEvent.VIrootID = VIrootID;

      if(port != 0)
      {
        port->send(sasResponseEvent);      
      }
      m_propertySet->setValue(MS_PROPNAME_STATUS,GCFPVInteger(sasResponseEvent.result));
    }
    
    m_beamletAllocator.deallocateBeamlets(VIrootID);
  }
  catch(Exception& e)
  {
    LOG_FATAL(formatString("Error reading schedule parameters: %s",e.message().c_str()));
    SASResponseEvent sasResponseEvent;
    sasResponseEvent.result = SAS_RESULT_ERROR_UNSPECIFIED;
    sasResponseEvent.VIrootID = VIrootID;

    if(port != 0)
    {
      port->send(sasResponseEvent);
    }
    m_propertySet->setValue(MS_PROPNAME_STATUS,GCFPVInteger(sasResponseEvent.result));
  }
}

};
};
