//#  GPI_PropertySet.cc: 
//#
//#  Copyright (C) 2002-2003
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

#include <GPI_PropertySet.h>
#include <GPI_PMLlightServer.h>
#include <GCF/GCF_PValue.h>
#include <GCF/Utils.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <PI_Protocol.ph>

TPAResult convertPIToPAResult(TPIResult result)
{
  switch (result)
  {
    case PI_WRONG_STATE: return PA_WRONG_STATE;
    case PI_PS_GONE: return PA_PS_GONE;
    case PI_PROP_SET_NOT_EXISTS: return PA_PROP_SET_NOT_EXISTS;
    case PI_PROP_SET_ALLREADY_EXISTS: return PA_PROP_SET_ALLREADY_EXISTS;
    case PI_DPTYPE_UNKNOWN: return PA_DPTYPE_UNKNOWN;
    case PI_INTERNAL_ERROR: return PA_PI_INTERNAL_ERROR;
    case PI_PA_INTERNAL_ERROR: return PA_INTERNAL_ERROR; 
    case PI_NO_ERROR: return PA_NO_ERROR;
    case PI_MISSING_PROPS: return PA_MISSING_PROPS;
    default: return PA_UNKNOWN_ERROR;
  }
}

TPIResult convertPAToPIResult(TPAResult result)
{
  switch (result)
  {
    case PA_WRONG_STATE: return PI_WRONG_STATE;
    case PA_PS_GONE: return PI_PS_GONE;
    case PA_PROP_SET_NOT_EXISTS: return PI_PROP_SET_NOT_EXISTS;
    case PA_PROP_SET_ALLREADY_EXISTS: return PI_PROP_SET_ALLREADY_EXISTS;
    case PA_DPTYPE_UNKNOWN: return PI_DPTYPE_UNKNOWN;
    case PA_INTERNAL_ERROR: return PI_PA_INTERNAL_ERROR;
    case PA_PI_INTERNAL_ERROR: return PI_INTERNAL_ERROR; 
    case PA_NO_ERROR: return PI_NO_ERROR;
    case PA_MISSING_PROPS: return PI_MISSING_PROPS;
    default: return PI_UNKNOWN_ERROR;
  }
}

void GPIPropertySet::propSubscribed(const string& /*propName*/)
{
  _counter--;
  if (_counter == 0)
  {
    PAPropSetLinkedEvent responseOut;
    responseOut.result = convertPIToPAResult(_tmpPIResult);
    responseOut.scope = _scope;
    _state = S_LINKED;
    sendMsgToPA(responseOut);
  }
}

void GPIPropertySet::propValueChanged(const string& propName, const GCFPValue& value)
{
  PIValueChangedEvent indicationOut;
  indicationOut.scopeLength = _scope.length();
  indicationOut.name = propName;
  if (indicationOut.name.find(':') < propName.length())
  {
    indicationOut.name.erase(0, indicationOut.name.find(':') + 1); 
  }
  indicationOut.value._pValue = &value;
  sendMsgToRTC(indicationOut);
}

void GPIPropertySet::enable(PIRegisterScopeEvent& requestIn)
{
  LOG_INFO(formatString(
      "Request to enable prop. set '%s' of type '%s'",
      requestIn.scope.c_str(),
      requestIn.type.c_str()));
  switch (_state)
  {
    case S_DISABLED:
    {
      _state = S_ENABLING;
      PARegisterScopeEvent requestOut;
      requestOut.scope = requestIn.scope;
      requestOut.type = requestIn.type;
      requestOut.seqnr = requestIn.seqnr;
      requestOut.isTemporary = requestIn.isTemporary;
      _scope = requestIn.scope;
      _type = requestIn.type;
      _isTemporary = requestIn.isTemporary;
      _savedSeqnr = requestIn.seqnr;
      sendMsgToPA(requestOut);
      break;
    }
    default:
    {
      wrongState("enable");
      PAScopeRegisteredEvent response;
      response.seqnr = requestIn.seqnr;
      response.result = PA_WRONG_STATE;
      sendMsgToRTC(response);     
      break;
    }
  }
}

void GPIPropertySet::retryEnable()
{
  LOG_INFO(formatString(
      "Retry enable prop. set '%s', because Property Agent was not available yet",
      _scope.c_str()));
  switch (_state)
  {
    case S_ENABLING:
    {
      PARegisterScopeEvent requestOut;
      requestOut.scope = _scope;
      requestOut.type = _type;
      requestOut.seqnr = _savedSeqnr;
      requestOut.isTemporary = _isTemporary;
      sendMsgToPA(requestOut);
      break;
    }
    default:
    {
      wrongState("retryEnable");
      PAScopeRegisteredEvent response;
      response.seqnr = _savedSeqnr;
      response.result = PA_WRONG_STATE;
      sendMsgToRTC(response);     
      break;
    }
  }
}

void GPIPropertySet::enabled(TPAResult result)
{
  PIScopeRegisteredEvent responseOut;
  responseOut.seqnr = _savedSeqnr;
  switch (_state)
  {
    case S_ENABLING:
    {
      responseOut.result = convertPAToPIResult(result);
      if (result == PA_NO_ERROR)
      {    
        _state = S_ENABLED;
      }
      else
      {    
        _state = S_DISABLED;
      }
      break;
    }
    default:
    {
      wrongState("enabled");
      responseOut.result = PI_WRONG_STATE;
      break;
    }
  }
  sendMsgToRTC(responseOut);
}

void GPIPropertySet::disable(PIUnregisterScopeEvent& requestIn)
{
  LOG_INFO(formatString(
      "Request to disable prop. set '%s' of type '%s'",
      _scope.c_str(),
      _type.c_str()));
  switch (_state)
  {
    case S_LINKED:
      for (list<string>::iterator iter = _propsSubscribed.begin(); 
           iter != _propsSubscribed.end(); ++iter)
      {
        string fullName;
        assert(_scope.length() > 0);
        fullName = _scope + GCF_PROP_NAME_SEP + *iter;
        if (GCFPVSSInfo::propExists(fullName))
        {
          unsubscribeProp(fullName);          
        }
      }
      break;    
    case S_LINKING:
      assert(_counter > 0);
      _state = S_DELAYED_DISABLING;
      _savedSeqnr = requestIn.seqnr;
      break;
    case S_UNLINKING:
    case S_ENABLING:
    case S_ENABLED:
    {
      _state = S_DISABLING;
      PAUnregisterScopeEvent requestOut;
      requestOut.scope = _scope;
      requestOut.seqnr = _savedSeqnr;
      sendMsgToPA(requestOut);      
      break;
    }
    default:
    {
      wrongState("disable");
      PIScopeUnregisteredEvent erResponse;
      erResponse.seqnr = _savedSeqnr;
      erResponse.result = PI_WRONG_STATE;
      sendMsgToRTC(erResponse);     
      break;
    }
  }
}

void GPIPropertySet::disabled(TPAResult result)
{
  PIScopeUnregisteredEvent responseOut;
  responseOut.seqnr = _savedSeqnr;
  switch (_state)
  {
    case S_DISABLING:
    {
      responseOut.result = convertPAToPIResult(result);
      _state = S_DISABLED;
      break;
    }
    default:
    {
      wrongState("disabled");
      responseOut.result = PI_WRONG_STATE;
      break;
    }
  }
  sendMsgToRTC(responseOut);
}

void GPIPropertySet::linkPropSet(PALinkPropSetEvent& requestIn)
{
  PAPropSetLinkedEvent erResponse;
  erResponse.scope = _scope;
  switch (_state)
  {
    case S_ENABLED:
    {
      PILinkPropSetEvent requestOut;
      requestOut.scope = requestIn.scope;
      _state = S_LINKING;
      sendMsgToRTC(requestOut);
      break;
    }
    case S_DISABLED:
    case S_DISABLING:
      LOG_DEBUG(formatString ( 
          "Property set with scope %s is deleting in the meanwhile", 
          _scope.c_str()));
      erResponse.result = PA_PS_GONE;   
      sendMsgToPA(erResponse);
      break;
    default:
    {
      wrongState("linkProperties");
      erResponse.result = PA_WRONG_STATE;
      sendMsgToPA(erResponse);
      break;
    }
  }
}

bool GPIPropertySet::propSetLinkedInRTC(PIPropSetLinkedEvent& responseIn)
{
  if (_state == S_LINKING)
  {    
    assert(_counter == 0);
    if (responseIn.result != PI_PS_GONE)
    {
      Utils::convStringToList(_propsSubscribed, responseIn.propList);
      _tmpPIResult = responseIn.result;
      return trySubscribing();
    }
    else
    {
      propSetLinkedInPI(PA_PS_GONE);
    }
  }
  else
  {
    wrongState("propSetLinked");
    propSetLinkedInPI(PA_WRONG_STATE);
  }
  return true;
}

void GPIPropertySet::propSetLinkedInPI(TPAResult result)
{
  PAPropSetLinkedEvent response;
  response.scope = _scope;
  response.result = result;
  sendMsgToPA(response);
}

bool GPIPropertySet::trySubscribing()
{  
  bool successful(true);
  switch (_state)
  {
    case S_LINKING:
    {
      TPAResult result(PA_NO_ERROR);
      for(list<string>::iterator iter = _propsSubscribed.begin(); 
          iter != _propsSubscribed.end(); ++iter)
      {
        string fullName;
        assert(_scope.length() > 0);
        fullName = _scope + GCF_PROP_NAME_SEP + *iter;
        if (GCFPVSSInfo::propExists(fullName))
        {   
          if (subscribeProp(fullName) == GCF_NO_ERROR)
          {
            _counter++;
          }
          else
          {
            result = PA_PI_INTERNAL_ERROR;
          }
        }
        else 
        {
          _missing++;
        }
      }      
      if (_counter == 0)
      {
        if (_missing == _propsSubscribed.size())
        {
          // propset is not yet known in this application, retry it with a 
          // 0 timer
          _missing = 0;
          successful = false;
          break;
        }
        // no more asyncronous link responses will be expected and 
        // no more properties needed to be linked 
        // so we can return a response to the controller
        _state = S_LINKED;
        if (_missing > 0)
        {         
          propSetLinkedInPI((result == PA_NO_ERROR ? PA_MISSING_PROPS : result));
        }
        else
        {        
          propSetLinkedInPI(result);
        }
      }
      break;
    }
    case S_DELAYED_DISABLING:
    {
      propSetLinkedInPI(PA_PS_GONE);
      _state = S_ENABLED;
      PIUnregisterScopeEvent dummy;
      disable(dummy);
      break;
    }
    case S_DISABLED:
    case S_DISABLING:
      propSetLinkedInPI(PA_PS_GONE);
      break;
    default:
      wrongState("trySubscribing");
      propSetLinkedInPI(PA_WRONG_STATE);
      break;
  }
  return successful;
}

void GPIPropertySet::unlinkPropSet(PAUnlinkPropSetEvent& requestIn)
{
  switch (_state)
  {
    case S_LINKED:
    {    
      _state = S_UNLINKING;
      TPAResult result(PA_NO_ERROR);
      for (list<string>::iterator iter = _propsSubscribed.begin(); 
           iter != _propsSubscribed.end(); ++iter)
      {
        string fullName;        
        if (_scope.length() > 0)
        {
          fullName = _scope + GCF_PROP_NAME_SEP + *iter;
        }
        else
        {
          fullName = *iter;
        }
        if (GCFPVSSInfo::propExists(fullName))
        {
          if (unsubscribeProp(fullName) != GCF_NO_ERROR)
          {
            result = PA_PI_INTERNAL_ERROR;
          }
        }
      }
      if (result == PA_NO_ERROR)
      {
        PIUnlinkPropSetEvent requestOut;
        requestOut.scope = requestIn.scope;
  
        assert(_counter == 0);
  
        sendMsgToRTC(requestOut);
      }
      else
      {
        propSetUnlinkedInPI(result);
      }
      break;  
    }
    case S_DISABLED:
    case S_DISABLING:
      LOG_DEBUG(formatString ( 
          "Property set with scope %s is deleting in the meanwhile", 
          _scope.c_str()));
      propSetUnlinkedInPI(PA_PS_GONE);
      break;
    default:
      wrongState("unlinkPropSet");
      propSetUnlinkedInPI(PA_WRONG_STATE);
      break;
  }
}

void GPIPropertySet::propSetUnlinkedInRTC(PIPropSetUnlinkedEvent& responseIn)
{
  if (_state == S_UNLINKING)
  {
    PAPropSetUnlinkedEvent responseOut;    
    responseOut.result = convertPIToPAResult(responseIn.result);
    responseOut.scope = responseIn.scope;
    _state = S_ENABLED;
    sendMsgToPA(responseOut);
  }
  else
  {
    wrongState("propSetUnlinked");
    propSetLinkedInPI(PA_WRONG_STATE);
  }
}

void GPIPropertySet::propSetUnlinkedInPI(TPAResult result)
{
  PAPropSetUnlinkedEvent response;
  response.scope = _scope;
  response.result = result;
  sendMsgToPA(response);
}

void GPIPropertySet::sendMsgToPA(GCFEvent& msg)
{
  if (_pls.getPAPort().isConnected())
  {
    _pls.getPAPort().send(msg);
  }
}

void GPIPropertySet::sendMsgToRTC(GCFEvent& msg)
{
  if (_pls.getPort().isConnected())
  {
    _pls.getPort().send(msg);
  }
}

void GPIPropertySet::wrongState(const char* request)
{
  const char* stateString[] =
  {
    "DISABLED",
    "DISABLING",
    "ENABLING",
    "ENABLED",
    "LINKING",
    "LINKED",
    "UNLINKING",
    "DELAYED DISABLING"
  };
  LOG_WARN(formatString ( 
        "Could not perform '%s' on property set '%s'. Wrong state: %s",
        request,
        getScope().c_str(),
        stateString[_state]));  
}
