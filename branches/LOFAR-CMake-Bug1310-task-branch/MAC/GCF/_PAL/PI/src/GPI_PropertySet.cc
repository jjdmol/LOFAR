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

#include <lofar_config.h>

#include <GPI_PropertySet.h>
#include <GPI_PMLlightServer.h>
#include <GCF/GCF_PValue.h>
#include <GCF/Utils.h>
#include <GCF/PAL/GCF_PVSSInfo.h>

namespace LOFAR 
{
 namespace GCF 
 {
using namespace Common;
  namespace PAL
  {
TPIResult convertPAToPIResult(TPAResult result);
TPAResult convertPIToPAResult(TPIResult result);

GPIPropertySet::~GPIPropertySet() 
{ 
  unsubscribeAllProps();
}

void GPIPropertySet::propSubscribed(const string& /*propName*/)
{
  ASSERT(_state == S_LINKING || _state == S_DELAYED_DISABLING);
  
  _counter--;
  LOG_DEBUG(formatString("%d subscriptions left", _counter));
  if (_counter == 0)
  {
    TState oldState(_state);
    propSetLinkedInPI(convertPIToPAResult(_tmpPIResult));
    _state = S_LINKED;
    if (oldState == S_DELAYED_DISABLING)
    {
      PIUnregisterScopeEvent dummy;
      disable(dummy);
    }
  }
}

void GPIPropertySet::propValueChanged(const string& propName, const GCFPValue& value)
{
  if (_state == S_LINKED)
  {
    PIValueChangedEvent indicationOut;
    indicationOut.scopeLength = _scope.length();
    indicationOut.name = propName;
    if (indicationOut.name.find(':') < propName.length())
    {
      indicationOut.name.erase(0, indicationOut.name.find(':') + 1); 
    }
    indicationOut.value._pValue = &value;
    sendMsgToClient(indicationOut);
  }
}

void GPIPropertySet::enable(const PIRegisterScopeEvent& requestIn)
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
      requestOut.category = requestIn.category;
      _scope = requestIn.scope;
      _type = requestIn.type;
      _category = requestIn.category;
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
      sendMsgToClient(response);     
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
      requestOut.category = _category;
      sendMsgToPA(requestOut);
      break;
    }
    default:
    {
      wrongState("retryEnable");
      PAScopeRegisteredEvent response;
      response.seqnr = _savedSeqnr;
      response.result = PA_WRONG_STATE;
      sendMsgToClient(response);     
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
      _state = (result == PA_NO_ERROR ? S_ENABLED : S_DISABLED);
      break;
    }
    default:
    {
      wrongState("enabled");
      responseOut.result = PI_WRONG_STATE;
      break;
    }
  }
  sendMsgToClient(responseOut);
  _savedSeqnr = 0;
}

void GPIPropertySet::disable(const PIUnregisterScopeEvent& requestIn)
{
  LOG_INFO(formatString(
      "Request to disable prop. set '%s' of type '%s'",
      _scope.c_str(),
      _type.c_str()));
  switch (_state)
  {
    case S_LINKED:
      unsubscribeAllProps();
      // intentional fall through
    case S_ENABLED:
      if (_savedSeqnr == 0)
      {
        _savedSeqnr = requestIn.seqnr;
      }     
      disabling(_savedSeqnr);
      _savedSeqnr = 0;
      break;

    case S_LINKING:
      LOG_DEBUG(formatString("%d subscriptions left", _counter));
      ASSERT(_counter > 0);
      // intentional fall through
    case S_UNLINKING:
    case S_LINKING_IN_CLIENT:
      _state = S_DELAYED_DISABLING;
      _savedSeqnr = requestIn.seqnr;
      break;
      
    default:
    {
      wrongState("disable");
      PIScopeUnregisteredEvent errResponse;
      errResponse.seqnr = requestIn.seqnr;
      errResponse.result = PI_WRONG_STATE;
      sendMsgToClient(errResponse);
      _savedSeqnr = 0;
      break;
    }
  }
}

void GPIPropertySet::disabling(unsigned int seqnr)
{
  _state = S_DISABLING;
  PAUnregisterScopeEvent requestOut;
  requestOut.scope = _scope;
  requestOut.seqnr = seqnr;
  sendMsgToPA(requestOut);      
}

void GPIPropertySet::disabled(const PAScopeUnregisteredEvent& responseIn)
{
  PIScopeUnregisteredEvent responseOut;
  responseOut.seqnr = responseIn.seqnr;
  switch (_state)
  {
    case S_DISABLING:
    {
      
      responseOut.result = convertPAToPIResult(responseIn.result);
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
  sendMsgToClient(responseOut);
}

void GPIPropertySet::linkPropSet(const PALinkPropSetEvent& requestIn)
{
  PAPropSetLinkedEvent errResponse;
  errResponse.scope = _scope;
  switch (_state)
  {
    case S_ENABLED:
    {
      PILinkPropSetEvent requestOut;
      requestOut.scope = requestIn.scope;
      _state = S_LINKING_IN_CLIENT;
      sendMsgToClient(requestOut);
      break;
    }
    case S_DISABLED:
    case S_DISABLING:
      LOG_DEBUG(formatString ( 
          "Property set with scope %s is deleting in the meanwhile", 
          _scope.c_str()));
      errResponse.result = PA_PS_GONE;   
      sendMsgToPA(errResponse);
      break;
      
    default:
      wrongState("linkProperties");
      errResponse.result = PA_WRONG_STATE;
      sendMsgToPA(errResponse);
      break;
  }
}

void GPIPropertySet::linkCEPPropSet(const PALinkPropSetEvent& requestIn)
{
  PAPropSetLinkedEvent errResponse;
  errResponse.scope = _scope;
  switch (_state)
  {
    case S_ENABLED:
    {
      TPAResult result(PA_NO_ERROR);
      // write __pa_PiLinkPS datapoint
      setPropValue(PI_LINKPS, requestIn.scope, false);

      _state = S_LINKED;
      propSetLinkedInPI(result);
      break;
    }
    case S_DISABLED:
    case S_DISABLING:
      LOG_DEBUG(formatString ( 
          "Property set with scope %s is deleting in the meanwhile", 
          _scope.c_str()));
      errResponse.result = PA_PS_GONE;   
      sendMsgToPA(errResponse);
      break;
      
    default:
      wrongState("linkProperties");
      errResponse.result = PA_WRONG_STATE;
      sendMsgToPA(errResponse);
      break;
  }
}

bool GPIPropertySet::propSetLinkedInClient(const PIPropSetLinkedEvent& responseIn)
{
  switch (_state)
  {
    case S_LINKING_IN_CLIENT:
      ASSERT(_counter == 0);
      _state = S_LINKING;
      if (responseIn.result != PI_PS_GONE)
      {
        Common::convStringToList(_propsSubscribed, responseIn.propList);
        _tmpPIResult = responseIn.result;
        return trySubscribing();
      }
      else
      {
        propSetLinkedInPI(PA_PS_GONE);
      }
      break;

    case S_DELAYED_DISABLING:
      propSetLinkedInPI(convertPIToPAResult(responseIn.result));
      disabling(_savedSeqnr);
      _savedSeqnr = 0;
      break;
      
    default:
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
        ASSERT(_scope.length() > 0);
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
        if (_missing == _propsSubscribed.size() && _propsSubscribed.size() > 0)
        {
          // propset is not yet known in this application, retry it with a 
          // null timer in the server
          _missing = 0;
          successful = false;
          break;
        }
        // no more asyncronous link responses will be expected and 
        // no more properties needed to be linked 
        // so we can return a response to the controller
        _state = S_LINKED;
        if (_missing > 0 && result == PA_NO_ERROR)
        {         
          propSetLinkedInPI(PA_MISSING_PROPS);
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
      propSetLinkedInPI(PA_NO_ERROR);
      _state = S_LINKED;
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

void GPIPropertySet::unlinkPropSet(const PAUnlinkPropSetEvent& requestIn)
{
  switch (_state)
  {
    case S_LINKED:
    {    
      _state = S_UNLINKING;
      TPAResult result = unsubscribeAllProps();
      if (result == PA_NO_ERROR)
      {
        PIUnlinkPropSetEvent requestOut;
        requestOut.scope = requestIn.scope;
  
        ASSERT(_counter == 0);
  
        sendMsgToClient(requestOut);
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
          "Property set with scope %s is deleted in the meanwhile", 
          _scope.c_str()));
      propSetUnlinkedInPI(PA_PS_GONE);
      break;
      
    default:
      wrongState("unlinkPropSet");
      propSetUnlinkedInPI(PA_WRONG_STATE);
      break;
  }
}

void GPIPropertySet::unlinkCEPPropSet(const PAUnlinkPropSetEvent& requestIn)
{
  switch (_state)
  {
    case S_LINKED:
    {    
      _state = S_ENABLED;
      TPAResult result = unsubscribeAllProps();
      if (result == PA_NO_ERROR)
      {
        // write __pa_PiUnlinkPS datapoint
        setPropValue(PI_UNLINKPS, requestIn.scope, false);

        propSetUnlinkedInPI(result);
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
          "Property set with scope %s is deleted in the meanwhile", 
          _scope.c_str()));
      propSetUnlinkedInPI(PA_PS_GONE);
      break;
      
    default:
      wrongState("unlinkPropSet");
      propSetUnlinkedInPI(PA_WRONG_STATE);
      break;
  }
}

void GPIPropertySet::propSetUnlinkedInClient(const PIPropSetUnlinkedEvent& responseIn)
{
  switch (_state)
  {
    case S_UNLINKING:
    {
      PAPropSetUnlinkedEvent responseOut;    
      responseOut.result = convertPIToPAResult(responseIn.result);
      responseOut.scope = responseIn.scope;
      _state = S_ENABLED;
      sendMsgToPA(responseOut);
      break;
    }
    case S_DELAYED_DISABLING:
    {
      propSetUnlinkedInPI(convertPIToPAResult(responseIn.result));
      _state = S_ENABLED;
      PIUnregisterScopeEvent dummy;
      disable(dummy);
      break;
    }
    default:
      wrongState("propSetUnlinked");
      propSetLinkedInPI(PA_WRONG_STATE);
      break;
  }
}

void GPIPropertySet::propSetUnlinkedInPI(TPAResult result)
{
  PAPropSetUnlinkedEvent response;
  response.scope = _scope;
  response.result = result;
  sendMsgToPA(response);
}


TPAResult GPIPropertySet::unsubscribeAllProps()
{
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
  _propsSubscribed.clear();
  return result;
}

void GPIPropertySet::wrongState(const char* request)
{
  const char* stateString[] =
  {
    "DISABLED",
    "DISABLING",
    "DELAYED DISABLING",
    "ENABLING",
    "ENABLED",
    "LINKING",
    "LINKING IN CLIENT"
    "LINKED",
    "UNLINKING"
  };
  LOG_WARN(formatString ( 
        "Could not perform '%s' on property set '%s'. Wrong state: %s",
        request,
        getScope().c_str(),
        stateString[_state]));  
}

TPAResult convertPIToPAResult(TPIResult result)
{
  switch (result)
  {
    case PI_WRONG_STATE:              return PA_WRONG_STATE;
    case PI_PS_GONE:                  return PA_PS_GONE;
    case PI_PROP_SET_NOT_EXISTS:      return PA_PROP_SET_NOT_EXISTS;
    case PI_PROP_SET_ALREADY_EXISTS:  return PA_PROP_SET_ALREADY_EXISTS;
    case PI_DPTYPE_UNKNOWN:           return PA_DPTYPE_UNKNOWN;
    case PI_INTERNAL_ERROR:           return PA_PI_INTERNAL_ERROR;
    case PI_PA_INTERNAL_ERROR:        return PA_INTERNAL_ERROR; 
    case PI_NO_ERROR:                 return PA_NO_ERROR;
    case PI_MISSING_PROPS:            return PA_MISSING_PROPS;
    default:                          return PA_UNKNOWN_ERROR;
  }
}

TPIResult convertPAToPIResult(TPAResult result)
{
  switch (result)
  {
    case PA_WRONG_STATE:              return PI_WRONG_STATE;
    case PA_PS_GONE:                  return PI_PS_GONE;
    case PA_PROP_SET_NOT_EXISTS:      return PI_PROP_SET_NOT_EXISTS;
    case PA_PROP_SET_ALREADY_EXISTS:  return PI_PROP_SET_ALREADY_EXISTS;
    case PA_DPTYPE_UNKNOWN:           return PI_DPTYPE_UNKNOWN;
    case PA_INTERNAL_ERROR:           return PI_PA_INTERNAL_ERROR;
    case PA_PI_INTERNAL_ERROR:        return PI_INTERNAL_ERROR; 
    case PA_NO_ERROR:                 return PI_NO_ERROR;
    case PA_MISSING_PROPS:            return PI_MISSING_PROPS;
    default:                          return PI_UNKNOWN_ERROR;
  }
}
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
