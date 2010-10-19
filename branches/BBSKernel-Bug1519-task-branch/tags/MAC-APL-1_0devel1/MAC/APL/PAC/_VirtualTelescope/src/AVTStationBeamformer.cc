//#  AVTStationBeamformer.cc: Implementation of the Virtual Telescope task
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

#include <stdio.h>
#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_PVUnsigned.h>
#include <boost/shared_ptr.hpp>
#include "../../../APLCommon/src/APL_Defines.h"
#include "AVTDefines.h"
#include "AVTStationBeamformer.h"
#include "LogicalDevice_Protocol.ph"
#include "AVTUtilities.h"
#include "ABS_Protocol.ph"

const string bsScope("BeamServer");

AVTStationBeamformer::AVTStationBeamformer(string& taskName, 
                                           const TPropertySet& primaryPropertySet,
                                           const string& APCName,
                                           const string& APCScope,
                                           string& beamServerPortName) :
  AVTLogicalDevice(taskName,primaryPropertySet,APCName,APCScope),
  m_beamServer(*this, beamServerPortName, GCFPortInterface::SAP, ABS_PROTOCOL),
  m_beamServerConnected(false),
  m_numAPCsLoaded(0),
  m_maxAPCs(1),
  m_startTime(0),
  m_stopTime(0),
  m_frequency(0.0),
  m_subbands(),
  m_APCBeamServerStatistics(string("ApcAplBeamServerStatistics"),bsScope,&m_APCAnswer),
  m_directionType(0),
  m_directionAngle1(0.0),
  m_directionAngle2(0.0),
  m_beamID(-1)
{
  registerProtocol(ABS_PROTOCOL, ABS_PROTOCOL_signalnames);
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::AVTStationBeamformer",getName().c_str()));
}


AVTStationBeamformer::~AVTStationBeamformer()
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::~AVTStationBeamformer",getName().c_str()));
}

int AVTStationBeamformer::convertDirection(const string type) const
{
  int direction=2;
  if(type==string(DIRECTIONTYPE_J2000))
  {
    direction=1;
  }
  else if(type==string(DIRECTIONTYPE_LMN))
  {
    direction=3;
  }
  return direction;
}

// increment 1 only!!!
// the LogicalDeviceServer needs a beamserver port to send ABS_WGSETTINGS messages to 
// At the moment, the BeamServer does not allow more than one connection.
GCFPort& AVTStationBeamformer::getBeamServerPort()
{
  return m_beamServer;
}

bool AVTStationBeamformer::_isBeamServerPort(GCFPortInterface& port)
{
  return (&port == &m_beamServer); // comparing two pointers. yuck?
}

void AVTStationBeamformer::concreteDisconnected(GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concreteDisconnected",getName().c_str()));
  // go to initial state only if the connection with the beamformer is lost.
  if(_isBeamServerPort(port))
  {
    m_beamServerConnected=false;
    TRAN(AVTLogicalDevice::initial_state);
  }
}

GCFEvent::TResult AVTStationBeamformer::concrete_initial_state(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concrete_initial_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
      // open all ports
      m_beamServer.open();
      break;

    case F_CONNECTED_SIG:
    {
      // go to operational only if there is a connection with the beam server.
      if(_isBeamServerPort(port))
      {
        m_beamServerConnected=true;
        if(isAPCLoaded())
        {
          TRAN(AVTLogicalDevice::idle_state);
        }
        else
        {
          m_beamServer.setTimer(2.0); // try again
        }
      }
      break;
    }

    case F_DISCONNECTED_SIG:
      if(_isBeamServerPort(port))
      {
        m_beamServer.setTimer(2.0); // try again
      }
      break;

    case F_TIMER_SIG:
      if(_isBeamServerPort(port))
      {
        if(m_beamServerConnected)
        {
          if(isAPCLoaded())
          {
            TRAN(AVTLogicalDevice::idle_state);
          }
          else
          {
            m_beamServer.setTimer(2.0); // try again
          }
        }
        else
        {
          m_beamServer.open(); // try again
        }
      }
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concrete_initial_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTStationBeamformer::concrete_claiming_state(GCFEvent& event, GCFPortInterface& /*port*/, bool& stateFinished)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concrete_claiming_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case LOGICALDEVICE_CLAIMED:
      stateFinished=true;
      break;
      
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concrete_claiming_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult AVTStationBeamformer::concrete_preparing_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished, bool& error)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concrete_preparing_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;
  stateFinished=true;
  error=false;

  switch (event.signal)
  {
    case LOGICALDEVICE_PREPARED:
      stateFinished=true;
      break;
      
    case ABS_BEAMALLOC_ACK: 
      // prepared event is received from the beam server
      if(_isBeamServerPort(port))
      {
        // check the beam ID and status of the ACK message
        ABSBeamalloc_AckEvent& ackEvent=static_cast<ABSBeamalloc_AckEvent&>(event);
        if(ackEvent.status==0)
        {
          m_beamID=ackEvent.handle;
          
          // load APC's for the subbands of the beam;
          m_APCBeamServerStatistics.load(false); // no defaults
          SubbandAPCMapT::iterator subbandIterator=m_subbands.begin();
          while(subbandIterator!=m_subbands.end())
          {
            subbandIterator->second->load(false); // no defaults          
            ++subbandIterator;
          }

          // point the new beam
          time_t time_arg(0);
          ABSBeampointtoEvent beamPointToEvent(m_beamID,time_arg,m_directionType,m_directionAngle1,m_directionAngle2);
          m_beamServer.send(beamPointToEvent);
        }
        else
        {
          error=true;
        }
        stateFinished=true;
      }
      break;
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concrete_preparing_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTStationBeamformer::concrete_releasing_state(GCFEvent& event, GCFPortInterface& /*port*/, bool& stateFinished)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concrete_releasing_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case ABS_BEAMFREE_ACK:
    {
      // check the beam ID and status of the ACK message
      ABSBeamfree_AckEvent& ackEvent=static_cast<ABSBeamfree_AckEvent&>(event);
      if(ackEvent.handle==m_beamID)
      {
        stateFinished=true;
      }
      break;
    }
        
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concrete_releasing_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void AVTStationBeamformer::handlePropertySetAnswer(GCFEvent& answer)
{
  switch(answer.signal)
  {
    case F_MYPLOADED_SIG:
    {
      // property set loaded, now load apc
      m_APC.load(false);
      break;
    }
    
    case F_VCHANGEMSG_SIG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&answer);
      assert(pPropAnswer);
      if ((pPropAnswer->pValue->getType() == GCFPValue::LPT_STRING) &&
          (strstr(pPropAnswer->pPropName, PROPERTYNAME_COMMAND) != 0))
      {
        // command received
        string commandString(((GCFPVString*)pPropAnswer->pValue)->getValue());
        vector<string> parameters;
        string command;
        AVTUtilities::decodeCommand(commandString,command,parameters);
        
        // PREPARE <starttime>,<stoptime>,<frequency>,<subbands>,<directiontype>,<directionangle1>,<directionangle2>
        if(command==string(LD_COMMAND_PREPARE))
        {
          if(parameters.size()==7)
          {
            // send prepare message:
            char prepareParameters[700];
            AVTUtilities::encodeParameters(parameters,prepareParameters,700);
            
            // send prepare to myself using a dummyport
            GCFDummyPort dummyPort(this,string("BF_command_dummy"),LOGICALDEVICE_PROTOCOL);
            LOGICALDEVICEPrepareEvent prepareEvent(prepareParameters);
            dispatch(prepareEvent,dummyPort); // dummyport
          }
        }
        // SUSPEND
        else if(command==string(LD_COMMAND_SUSPEND))
        {
          // send prepare to myself using a dummyport
          GCFDummyPort dummyPort(this,string("BF_command_dummy"),LOGICALDEVICE_PROTOCOL);
          GCFEvent e(LOGICALDEVICE_SUSPEND);
          dispatch(e,dummyPort); // dummyport
        }
        // RESUME
        else if(command==string(LD_COMMAND_RESUME))
        {
          // send prepare to myself using a dummyport
          GCFDummyPort dummyPort(this,string("BF_command_dummy"),LOGICALDEVICE_PROTOCOL);
          GCFEvent e(LOGICALDEVICE_RESUME);
          dispatch(e,dummyPort); // dummyport
        }
      }
      else if ((pPropAnswer->pValue->getType() == GCFPValue::LPT_STRING) &&
               (strstr(pPropAnswer->pPropName, PROPERTYNAME_DIRECTIONTYPE) != 0))
      {
        // directionType received
        string typeString(((GCFPVString*)pPropAnswer->pValue)->getValue());
        int newType=convertDirection(typeString);
        if(newType!=m_directionType)
        {
          m_directionType=newType;
          // send new direction
          time_t time_arg(0);
          ABSBeampointtoEvent beamPointToEvent(m_beamID,time_arg,m_directionType,m_directionAngle1,m_directionAngle2);
          m_beamServer.send(beamPointToEvent);
        }
      }      
      else if ((pPropAnswer->pValue->getType() == GCFPValue::LPT_DOUBLE) &&
               (strstr(pPropAnswer->pPropName, PROPERTYNAME_DIRECTIONANGLE1) != 0))
      {
        // directionAngle1 received
        double angle1(((GCFPVDouble*)pPropAnswer->pValue)->getValue());
        if(angle1!=m_directionAngle1)
        {
          m_directionAngle1 = angle1;
          // send new direction
          time_t time_arg(0);
          ABSBeampointtoEvent beamPointToEvent(m_beamID,time_arg,m_directionType,m_directionAngle1,m_directionAngle2);
          m_beamServer.send(beamPointToEvent);
        }
      }      
      else if ((pPropAnswer->pValue->getType() == GCFPValue::LPT_DOUBLE) &&
               (strstr(pPropAnswer->pPropName, PROPERTYNAME_DIRECTIONANGLE2) != 0))
      {
        // directionAngle2 received
        double angle2(((GCFPVDouble*)pPropAnswer->pValue)->getValue());
        if(angle2!=m_directionAngle2)
        {
          m_directionAngle2 = angle2;
          // send new direction
          time_t time_arg(0);
          ABSBeampointtoEvent beamPointToEvent(m_beamID,time_arg,m_directionType,m_directionAngle1,m_directionAngle2);
          m_beamServer.send(beamPointToEvent);
        }
      }      
      break;
    }  

    default:
      break;
  }  
}

void AVTStationBeamformer::handleAPCAnswer(GCFEvent& answer)
{
  switch(answer.signal)
  {
    case F_APCLOADED_SIG:
    {
      // used during startup to check if all necessary APC's have been loaded.
      // not used when loading the statistics APC's
      m_numAPCsLoaded++;
      if(m_numAPCsLoaded==m_maxAPCs)
      {
        apcLoaded();
      }
      break;
    }
    
    default:
      break;
  }  
}

void AVTStationBeamformer::concreteClaim(GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concreteClaim",getName().c_str()));
  // claim my own resources
  
  // send claim message to BeamServer
  
  // if claiming is an async process, then the end of the claiming state
  // is determined in the concrete_claiming_state() method
  // Otherwise, it is done here by calling dispatch
  GCFEvent event(LOGICALDEVICE_CLAIMED);
  dispatch(event,port);
}

void AVTStationBeamformer::concretePrepare(GCFPortInterface& /*port*/,string& parameters)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concretePrepare",getName().c_str()));
  // prepare my own resources
  vector<string> decodedParameters;
  AVTUtilities::decodeParameters(parameters,decodedParameters);
  // parameters: start time,stop time,frequency,subbands,direction type,angle1,angle2
  m_startTime=atoi(decodedParameters[0].c_str());
  m_stopTime=atoi(decodedParameters[1].c_str());
  m_frequency=atof(decodedParameters[2].c_str());
  vector<int> subbandsVector;
  AVTUtilities::decodeSubbandsParameter(decodedParameters[3],subbandsVector);
  
  // create APC's for the subbands. They will be loaded after the ALLOC_ACK has
  // been received
  vector<int>::iterator vectorIterator=subbandsVector.begin();
  while(vectorIterator!=subbandsVector.end())
  {
    char tempScope[100];
    sprintf(tempScope,"%s_power%03d",bsScope.c_str(),*vectorIterator);
    boost::shared_ptr<GCFApc> pSubbandAPC(new GCFApc(string("ApcAplBeamServerSubbandStatistics"),string(tempScope),&m_APCAnswer));
    m_subbands.insert(SubbandAPCMapT::value_type(*vectorIterator,pSubbandAPC));
    ++vectorIterator;
  }
  
  m_directionType=convertDirection(decodedParameters[4]);
  m_directionAngle1=atof(decodedParameters[5].c_str());
  m_directionAngle2=atof(decodedParameters[6].c_str());
  
  m_beamID=0; // TODO
  int spectral_window(0);
  int n_subbands(subbandsVector.size());
  int subbandsArray[ABS::N_BEAMLETS];
  char tempLogStr[1000];
  tempLogStr[0]=0;
  
  memset(subbandsArray,0,sizeof(subbandsArray[0])*ABS::N_BEAMLETS);
  vectorIterator=subbandsVector.begin();
  int arrayIndex(0);
  while(arrayIndex<ABS::N_BEAMLETS && vectorIterator!=subbandsVector.end())
  {
    subbandsArray[arrayIndex++]=*vectorIterator;
    char tempStr[10];
    sprintf(tempStr,"%d ",subbandsArray[arrayIndex-1]);
    strcat(tempLogStr,tempStr);
    ++vectorIterator;
  }
  
  
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::allocate %d subbands: %s",getName().c_str(),n_subbands,tempLogStr));
  ABSBeamallocEvent beamAllocEvent(spectral_window,n_subbands,subbandsArray);
  m_beamServer.send(beamAllocEvent);
}

void AVTStationBeamformer::concreteResume(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concreteResume",getName().c_str()));
  // resume my own resources
  
  // send resume message to BeamFormer
  GCFEvent wgEnableEvent(ABS_WGENABLE);
  m_beamServer.send(wgEnableEvent);
  
}

void AVTStationBeamformer::concreteSuspend(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concreteSuspend",getName().c_str()));
  // suspend my own resources
  
  // send suspend message to BeamFormer
  GCFEvent wgDisableEvent(ABS_WGDISABLE);
  m_beamServer.send(wgDisableEvent);
}

void AVTStationBeamformer::concreteRelease(GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTStationBeamformer(%s)::concreteRelease",getName().c_str()));
  // release my own resources
  
  // send release message to BeamFormer
  ABSBeamfreeEvent beamFreeEvent(m_beamID);
  m_beamServer.send(beamFreeEvent);
  
  // wait for the beamfree_ack event in the releasing state
}
