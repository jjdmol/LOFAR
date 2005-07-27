//#  GSA_Service.cc: 
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

#define LOFARLOGGER_SUBPACKAGE "SAL"

#include "GSA_Service.h"
#include "GSA_Defines.h"
#include "GSA_WaitForAnswer.h"
#include "GSA_SCADAHandler.h"

#include <GCF/GCF_PVBool.h>
#include <GCF/GCF_PVChar.h>
#include <GCF/GCF_PVInteger.h>
#include <GCF/GCF_PVUnsigned.h>
#include <GCF/GCF_PVDouble.h>
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVDynArr.h>
#include <GCF/GCF_PVBlob.h>
#include <GCF/PAL/GCF_PVSSInfo.h>

#include <DpMsgAnswer.hxx>            
#include <DpMsgHotLink.hxx>           
#include <DpHLGroup.hxx>              
#include <DpVCItem.hxx>               
#include <ErrHdl.hxx>                 
#include <ErrClass.hxx>
#include <Manager.hxx>
#include <FloatVar.hxx>
#include <CharVar.hxx>
#include <TextVar.hxx>
#include <IntegerVar.hxx>
#include <UIntegerVar.hxx>
#include <DynVar.hxx>
#include <BlobVar.hxx>
#include <AnyTypeVar.hxx>
#include <DpIdentifierVar.hxx>

namespace LOFAR 
{
 namespace GCF  
 {
  using namespace Common;
  namespace PAL
  {
GSAService::GSAService() : _pWFA(0)
{
  _pWFA  = new GSAWaitForAnswer(*this);
  _pSCADAHandler = GSASCADAHandler::instance();
  assert(_pSCADAHandler);
  if (_pSCADAHandler->isOperational() == SA_SCADA_NOT_AVAILABLE)
  {
    LOG_ERROR(formatString (
        "Error on creating a SCADA service"));
    Manager::exit(-1);
  }
}

GSAService::~GSAService()
{
  if (_pWFA)
    delete _pWFA;
  assert(_pSCADAHandler);
  GSASCADAHandler::release();
  _pSCADAHandler = 0;
}

// Receive Signals.
// We are interested in SIGINT and SIGTERM. 
void GSAService::handleHotLink(const DpMsgAnswer& answer, const GSAWaitForAnswer& wait)
{
  CharString pvssDPEConfigName;
  string DPEConfigName;
  string dpName;
  Variable *pVar;
  bool handled(false);
  GCFPValue* pPropertyValue(0);
  CharString pvssTypeName;
  
  TimeVar ts(answer.getOriginTime());
  
  GCFPVSSInfo::_lastTimestamp.tv_sec = ts.getSeconds();
  GCFPVSSInfo::_lastTimestamp.tv_usec = ts.getMilli() * 1000;

  for (AnswerGroup *pGrItem = answer.getFirstGroup();
       pGrItem; pGrItem = answer.getNextGroup())
  {
    for (AnswerItem *pAnItem = pGrItem->getFirstItem(); pAnItem;
         pAnItem = pGrItem->getNextItem())
    {
      GCFPVSSInfo::_lastSysNr = pAnItem->getDpIdentifier().getSystem();
      if (pAnItem->getDpIdentifier().convertToString(pvssDPEConfigName) == PVSS_FALSE)
      {
        if (answer.isAnswerOn() == DP_MSG_DP_REQ)
        {
          LOG_INFO(formatString (
              "DP %s was deleted successful", 
              wait.getDpName().c_str()));   
          dpDeleted(wait.getDpName());
          handled = true;
        }
        else
        {
          LOG_FATAL(formatString (
              "PVSS: Could not convert dpIdentifier '%d'", 
              pAnItem->getDpIdentifier().getDp()));   
        }
      }
      else
      { 
        DPEConfigName = pvssDPEConfigName;
        convDpConfigToProp(DPEConfigName, dpName);
        handled = true;
        switch (answer.isAnswerOn())
        {
          case DP_MSG_CONNECT:
            LOG_INFO(formatString (
                "DPE %s was subscribed successful", 
                dpName.c_str()));   
            dpeSubscribed(dpName);
            break;

          case DP_MSG_REQ_NEW_DP:
            LOG_INFO(formatString (
                "DP %s was created successful", 
                dpName.c_str()));   
            dpCreated(dpName);
            break;

          case DP_MSG_SIMPLE_REQUEST:
            pVar = pAnItem->getValuePtr();
            if (pVar)      // could be NULL !!
            {
              if (convertPVSSToMAC(*pVar, &pPropertyValue) != SA_NO_ERROR)
              {
                LOG_ERROR(formatString (
                    "Could not convert PVSS DP (type %s) to MAC property (%s)", 
                    (const char*)pvssTypeName, dpName.c_str()));   
              }
              else
              {
                LOG_DEBUG(formatString (
                    "Value of '%s' has get", 
                    dpName.c_str()));   
                dpeValueGet(dpName, *pPropertyValue);
              }
              if (pPropertyValue)
                delete pPropertyValue; // constructed by convertPVSSToMAC method
            }
            break;
            
          case DP_MSG_FILTER_CONNECT:
            pVar = pAnItem->getValuePtr();
            if (pVar)      // could be NULL !!
            {
              if (pVar->isA() == UINTEGER_VAR)
              {
                LOG_DEBUG(formatString (
                    "Query subscription is performed successful (with queryid %d)", 
                    ((UIntegerVar *)pVar)->getValue()));   
                dpQuerySubscribed(((UIntegerVar *)pVar)->getValue());
              }
            }
            break;
            
          default:
            handled = false;
            break;        
        }
      }
      GCFPVSSInfo::_lastSysNr = 0;      
    }
    if (!handled)
    {
      if (pGrItem->wasOk() == PVSS_TRUE)
      {
        if (answer.isAnswerOn() == DP_MSG_COMPLEX_VC)
        {
          // this must be the answer on a dpSet(Wait) 
          dpeValueSet(wait.getDpName());
          handled = true;
        }
      }
      else
      {
        LOG_DEBUG(formatString (
            "Error (%s) in answer on: %d",
            (const char*) pGrItem->getError()->getErrorText(),
            answer.isAnswerOn()));
      }
    }
    
  }
  if (!handled)
  { 
    MsgType mt = answer.isAnswerOn();
    LOG_DEBUG(formatString (
        "Answer on: %d is not handled",
        mt));   
  }  
  GCFPVSSInfo::_lastTimestamp.tv_sec = 0;
  GCFPVSSInfo::_lastTimestamp.tv_usec = 0;
}

// Handle incoming hotlinks.
// This function is called from our hotlink object
void GSAService::handleHotLink(const DpHLGroup& group, const GSAWaitForAnswer& wait)
{
  ErrClass* pErr(0);
  TimeVar ts(group.getOriginTime());
      
  if ((pErr = group.getErrorPtr()) != 0)
  {
    // The only error, which can occur here means always that the subscriptions 
    // to the DPE is lost
    dpeSubscriptionLost(wait.getDpName());
  }
  // A group consists of pairs of DpIdentifier and values called items.
  // There is exactly one item for all configs we are connected.

  if (group.getIdentifier() == 0)
  {    
    GCFPVSSInfo::_lastTimestamp.tv_sec = ts.getSeconds();
    GCFPVSSInfo::_lastTimestamp.tv_usec = ts.getMilli() * 1000;
    GCFPVSSInfo::_lastManNum = group.getOriginManager().getManNum();
    GCFPVSSInfo::_lastManType = group.getOriginManager().getManType();
    GCFPVSSInfo::_lastSysNr = group.getOriginManager().getSystem();
    // normal subscription (with dpeSubscribe)
    for (DpVCItem *pItem = group.getFirstItem(); pItem;
         pItem = group.getNextItem())
    {
      Variable *pVar = pItem->getValuePtr();
      if (pVar)      // could be NULL !!
      {        
        convAndForwardValueChange(pItem->getDpIdentifier(), *pVar);
      }
    }
  }
  else
  {
    // hotlink as result of query subscriptions
    // first item contains the query id received in the answer of the query
    // request
    DpVCItem* pItem = group.getFirstItem();
    assert(pItem);
    Variable* pVar = pItem->getValuePtr();
    assert(pVar);
    assert(pVar->isA() == UINTEGER_VAR);
    assert(((UIntegerVar*)pVar)->getValue() == group.getIdentifier());
    
    // second (and last) item contains the changed property and its new value
    // it is received in a dyndynanytype variable with the following construction:
    // Note that indexing starts in this case at 1!!!
    // [ 
    //   1: [ ( this first item describes always the structure of the remaining items,
    //          its like a header of a table with columns)
    //        1: this header of the first column is empty because it points 
    //           always to the DP name
    //        2: header of the second column, it points to the result value for
    //           the first "SELECT" field in the query
    //        3: header of the third column for the second field
    //        4: ....
    //      ]
    //   2: [ 
    //        1: DP ID of the changed value (used to get the DP name),
    //        2: value for the first "SELECT" field in the query 
    //        3: value for the second "SELECT" field in the query 
    //        4: ...
    //      ]
    // ]
    pItem = group.getNextItem();
    assert(pItem);
    pVar = pItem->getValuePtr();
    assert(pVar);
    assert(pVar->isA() == DYNDYNANYTYPE_VAR);
    
    DpIdentifierVar* pDpId = (DpIdentifierVar*)((AnyTypeVar*)((DynVar&)*((DynVar&)*pVar)[2])[1])->getVar();
    ts = *(TimeVar*)((AnyTypeVar*)((DynVar&)*((DynVar&)*pVar)[2])[3])->getVar();
        
    GCFPVSSInfo::_lastTimestamp.tv_sec = ts.getSeconds();
    GCFPVSSInfo::_lastTimestamp.tv_usec = ts.getMilli() * 1000;

    ManagerIdentifier manId;
    IntegerVar manIdInt = *(IntegerVar*)((AnyTypeVar*)((DynVar&)*((DynVar&)*pVar)[2])[4])->getVar();
    manId.convFromInt(manIdInt.getValue());
    GCFPVSSInfo::_lastManNum = manId.getManNum();
    GCFPVSSInfo::_lastManType = manId.getManType();
    GCFPVSSInfo::_lastSysNr = manId.getSystem();
    pVar = ((AnyTypeVar*)((DynVar&)*((DynVar&)*pVar)[2])[2])->getVar();

    convAndForwardValueChange(pDpId->getValue(), *pVar);
  }
  
  GCFPVSSInfo::_lastTimestamp.tv_sec = 0;
  GCFPVSSInfo::_lastTimestamp.tv_usec = 0;
  GCFPVSSInfo::_lastManNum = 0;
  GCFPVSSInfo::_lastManType = 0;
  GCFPVSSInfo::_lastSysNr = 0;
}

void GSAService::convAndForwardValueChange(const DpIdentifier& dpId, const Variable& pvssVar)
{
  static CharString pvssDPEConfigName = "";
  static string DPEConfigName = "";
  static string dpName = "";
  static GCFPValue* pPropertyValue = 0;
  
  if (dpId.convertToString(pvssDPEConfigName) == PVSS_FALSE)
  {
    LOG_FATAL(formatString (
        "PVSS: Could not convert dpIdentifier '%d'", 
        dpId.getDp()));   
  }
  else 
  {
    DPEConfigName = pvssDPEConfigName;
    convDpConfigToProp(DPEConfigName, dpName);        
    if (convertPVSSToMAC(pvssVar, &pPropertyValue) != SA_NO_ERROR)
    {
      LOG_ERROR(formatString (
          "Could not convert PVSS DP to MAC property (%s)", 
          dpName.c_str()));   
    }
    else
    {
      LOG_DEBUG(formatString (
          "Value of '%s' has been changed", 
          dpName.c_str()));   
      dpeValueChanged(dpName, *pPropertyValue);
    }
    if (pPropertyValue)
      delete pPropertyValue; // constructed by convertPVSSToMAC method    
  }
}

TSAResult GSAService::dpCreate(const string& dpName, 
                               const string& typeName)
{
  TSAResult result(SA_NO_ERROR);
  
  DpTypeId dpTypeId;
  LangText dpNameLang(dpName.c_str());
  GSAWaitForAnswer *pWFA = new GSAWaitForAnswer(*this);
  CharString pvssTypeName(typeName.c_str());

  LOG_INFO(formatString (
      "Create DP '%s'", 
      dpName.c_str()));
  
  assert(_pSCADAHandler);
  if ((result = _pSCADAHandler->isOperational()) == SA_SCADA_NOT_AVAILABLE)
  {
    LOG_FATAL(formatString (
        "Unable to create DP: '%s'", 
        dpName.c_str()));    
  }
  else if (GCFPVSSInfo::propExists(dpName))
  {
    LOG_WARN(formatString (
        "DP '%s' already exists", 
        dpName.c_str()));
    result = SA_DP_ALREADY_EXISTS;    
  }
  else if (Manager::getTypeId(pvssTypeName, dpTypeId) == PVSS_FALSE)
  {
    ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
                  ErrClass::ERR_PARAM,        // wrong name: blame others
                  ErrClass::UNEXPECTEDSTATE,  // fits all
                  "GSAService",              // our file name
                  "createProp",                      // our function name
                  CharString("DatapointType ") + 
                    pvssTypeName + 
                    CharString(" missing"));

    LOG_FATAL(formatString (
        "PVSS: DatapointType '%s' unknown", 
        (const char*) pvssTypeName));

    result = SA_DPTYPE_UNKNOWN;
            
  }
  else if (Manager::dpCreate(dpNameLang, dpTypeId, pWFA) == PVSS_FALSE)
  {
    ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
                  ErrClass::ERR_PARAM,        // wrong name: blame others
                  ErrClass::UNEXPECTEDSTATE,  // fits all
                  "GSAService",               // our file name
                  "dpCreate",                 // our function name
                  CharString("Datapoint ") + 
                    dpName.c_str() + 
                    CharString(" could not be created"));

    LOG_ERROR(formatString (
        "PVSS: Unable to create DP: '%s'", 
        dpName.c_str()));

    result = SA_CREATEPROP_FAILED;
  }
  else
  {
    LOG_DEBUG(formatString (
        "Creation of DP '%s' was requested successful", 
        dpName.c_str()));
  }

  if (result != SA_NO_ERROR)
  {
    // default the PVSS API is configured to delete this object
    // but if there is an error occured PVSS API will not do this
    delete pWFA;
  }
  return result;
}

TSAResult GSAService::dpDelete(const string& dpName)
{
  TSAResult result(SA_NO_ERROR);  
  DpIdentifier dpId;
  GSAWaitForAnswer *pWFA = new GSAWaitForAnswer(*this);
  pWFA->setDpName(dpName);

  LOG_INFO(formatString (
      "Delete DP '%s'", 
      dpName.c_str()));
  
  assert(_pSCADAHandler);
  if ((result = _pSCADAHandler->isOperational()) != SA_NO_ERROR)
  {
    LOG_FATAL(formatString (
        "Unable to delete DP: '%s'", 
        dpName.c_str()));
  }
  else if (!GCFPVSSInfo::propExists(dpName))
  {
    LOG_WARN(formatString (
        "DP '%s' does not exists", 
        dpName.c_str()));    
    result = SA_PROP_DOES_NOT_EXIST;
  }
  else if ((result = getDpId(dpName, dpId)) != SA_NO_ERROR)
  {
    LOG_ERROR(formatString (
        "Unable to delete DP: '%s'", 
        dpName.c_str()));
  }
  else if (Manager::dpDelete(dpId, pWFA) == PVSS_FALSE)
  {
    ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
                  ErrClass::ERR_PARAM,        // wrong name: blame others
                  ErrClass::UNEXPECTEDSTATE,  // fits all
                  "GSAService",               // our file name
                  "dpDelete",               // our function name
                  CharString("Datapoint ") + dpName.c_str() + 
                  CharString(" could not be deleted"));

    LOG_ERROR(formatString (
        "PVSS: Unable to delete DP: '%s'", 
        dpName.c_str()));

    result = SA_DELETEPROP_FAILED;
  }
  else
  {
    LOG_DEBUG(formatString (
        "Deletion of DP '%s' was requested successful", 
        dpName.c_str()));
  }

  if (result != SA_NO_ERROR)
  {
    // default the PVSS API is configured to delete this object
    // but if there is an error occured PVSS API will not do this
    delete pWFA;
  }

  return result;
}

TSAResult GSAService::dpeSubscribe(const string& propName)
{
  TSAResult result(SA_NO_ERROR);
  
  DpIdentifier dpId;
  string pvssDpName;
  _pWFA->setDpName(propName);
  
  convPropToDpConfig(propName, pvssDpName, true);

  LOG_INFO(formatString (
      "Subscribe on property '%s'", 
      propName.c_str()));
  
  assert(_pSCADAHandler);
  if ((result = _pSCADAHandler->isOperational()) != SA_NO_ERROR)
  {
    LOG_FATAL(formatString (
        "Unable to subscribe on property: '%s'", 
        propName.c_str()));
  }
  else if (!GCFPVSSInfo::propExists(propName))
  {
    LOG_WARN(formatString (
        "Property: '%s' does not exists", 
        propName.c_str()));    
    result = SA_PROP_DOES_NOT_EXIST;      
  }
  else if ((result = getDpId(pvssDpName, dpId)) == SA_NO_ERROR)
  {
    DpIdentList dpIdList;

    dpIdList.append(dpId);
    if (Manager::dpConnect(dpIdList, _pWFA, PVSS_FALSE) == PVSS_FALSE)
    {
      ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
                    ErrClass::ERR_PARAM,        // wrong name: blame others
                    ErrClass::UNEXPECTEDSTATE,  // fits all
                    "GSAService",               // our file name
                    "dpeSubscribe",               // our function name
                    CharString("Datapoint ") + propName.c_str() + 
                    CharString(" could not be connected"));
  
      LOG_ERROR(formatString (
          "PVSS: Unable to subscribe on property: '%s'", 
          propName.c_str()));
  
      result = SA_SUBSCRIBEPROP_FAILED;
    }
    else
    {
      LOG_DEBUG(formatString (
          "Subscription on property '%s' was requested successful", 
          propName.c_str()));
    }
  }
  else
  {
    
    LOG_ERROR(formatString (
        "Unable to subscribe on property: '%s'", 
        propName.c_str()));
  }    
  return result;
}

TSAResult GSAService::dpeUnsubscribe(const string& propName)
{
  TSAResult result(SA_NO_ERROR);
  DpIdentifier dpId;
  string pvssDpName;

  convPropToDpConfig(propName, pvssDpName, true);

  LOG_INFO(formatString (
      "Unsubscribe from property '%s'", 
      propName.c_str()));
  
  assert(_pSCADAHandler);
  if ((result = _pSCADAHandler->isOperational()) != SA_NO_ERROR)
  {
    LOG_FATAL(formatString (
        "Unable to unsubscribe from property: '%s'", 
        propName.c_str()));
  }
  else if (!GCFPVSSInfo::propExists(propName))
  {
    LOG_WARN(formatString (
        "Property: '%s' does not exists", 
        propName.c_str()));    
    result = SA_PROP_DOES_NOT_EXIST;      
  }
  else if ((result = getDpId(pvssDpName, dpId)) == SA_NO_ERROR)
  {
    DpIdentList dpIdList;

    dpIdList.append(dpId);

    if (Manager::dpDisconnect(dpIdList, _pWFA) == PVSS_FALSE)
    {
      ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
                    ErrClass::ERR_PARAM,        // wrong name: blame others
                    ErrClass::UNEXPECTEDSTATE,  // fits all
                    "GSAService",               // our file name
                    "dpeUnsubscribe",               // our function name
                    CharString("Datapoint ") + propName.c_str() + 
                    CharString(" could not be disconnected"));

      LOG_ERROR(formatString (
          "PVSS: Unable to unsubscribe from property: '%s'", 
          propName.c_str()));

      result = SA_UNSUBSCRIBEPROP_FAILED;
    }
    else
    {
      LOG_DEBUG(formatString (
          "Unsubscription from property '%s' was requested successful", 
          propName.c_str()));
    }
  }
  else
  {
    LOG_ERROR(formatString (
        "Unable to unsubscribe from property: '%s'", 
        propName.c_str()));
  }
  
  return result;
}

TSAResult GSAService::dpeGet(const string& dpeName)
{
  TSAResult result(SA_NO_ERROR);
  DpIdentifier dpId;
  string pvssDpName;
  GSAWaitForAnswer *pWFA = new GSAWaitForAnswer(*this);
  pWFA->setDpName(dpeName);

  convPropToDpConfig(dpeName, pvssDpName, true);

  LOG_INFO(formatString (
      "Request value of property '%s'", 
      dpeName.c_str()));
  
  assert(_pSCADAHandler);
  if ((result = _pSCADAHandler->isOperational()) != SA_NO_ERROR)
  {
    LOG_FATAL(formatString (
        "Unable to request of property: '%s'", 
        dpeName.c_str()));
  }
  else if (!GCFPVSSInfo::propExists(dpeName))
  {
    LOG_WARN(formatString (
        "Property: '%s' does not exists", 
        dpeName.c_str()));    
    result = SA_PROP_DOES_NOT_EXIST;      
  }
  else if ((result = getDpId(pvssDpName, dpId)) != SA_NO_ERROR)
  {
    LOG_ERROR(formatString (
        "Unable to request value of property: '%s'", 
        dpeName.c_str()));
  }
  else if (Manager::dpGet(dpId, pWFA) == PVSS_FALSE)
  {
    ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
                  ErrClass::ERR_PARAM,        // wrong name: blame others
                  ErrClass::UNEXPECTEDSTATE,  // fits all
                  "GSAService",               // our file name
                  "dpeGet",                      // our function name
                  CharString("Value of datapoint ") + dpeName.c_str() + 
                  CharString(" could not be requested"));

    LOG_ERROR(formatString (
        "PVSS: Unable to request value of property: '%s'", 
        dpeName.c_str()));

    result = SA_GETPROP_FAILED;
  }
  else
  {
    LOG_DEBUG(formatString (
        "Value of property '%s' was requested successful", 
        dpeName.c_str()));
  }
  
  if (result != SA_NO_ERROR)
  {
    // default the PVSS API is configured to delete this object
    // but if there is an error occured PVSS API will not do this
    delete pWFA;
  }
  return result;
}

TSAResult GSAService::dpeSet(const string& dpeName, 
                             const GCFPValue& value,
                             double timestamp, 
                             bool wantAnswer)
{
  TSAResult result(SA_NO_ERROR);
  DpIdentifier dpId;
  Variable* pVar(0);
  string pvssDpName;

  convPropToDpConfig(dpeName, pvssDpName, false);

  LOG_INFO(formatString (
      "Set value of property '%s'", 
      dpeName.c_str()));
  
  assert(_pSCADAHandler);
  if ((result = _pSCADAHandler->isOperational()) != SA_NO_ERROR)
  {
    LOG_FATAL(formatString (
        "Unable to set value of property: '%s'", 
        dpeName.c_str()));
  }
  else if (!GCFPVSSInfo::propExists(dpeName))
  {
    LOG_WARN(formatString (
        "Property: '%s' does not exists", 
        dpeName.c_str()));    
    result = SA_PROP_DOES_NOT_EXIST;      
  }
  else if ((result = getDpId(pvssDpName, dpId)) != SA_NO_ERROR)
  {
    LOG_ERROR(formatString (
        "Unable to set value of property: '%s'", 
        dpeName.c_str()));
  }
  else if ((result = convertMACToPVSS(value, &pVar, dpId)) != SA_NO_ERROR)
  {
    LOG_ERROR(formatString (
        "Unable to set value of property: '%s'", 
        dpeName.c_str()));
  }
  else 
  {
    GSAWaitForAnswer* pWFA(0);
    if (wantAnswer)
    {
      GSAWaitForAnswer *pWFA = new GSAWaitForAnswer(*this); // will be deleted by PVSS API
      pWFA->setDpName(dpeName);
    }
    PVSSboolean retVal, retValTS(PVSS_TRUE);
    
    retVal = Manager::dpSet(dpId, *pVar, pWFA);      
    if (timestamp > 0.0)
    {
      TimeVar ts;
      ts.setSeconds((time_t)timestamp);
      ts.setMilli((time_t)(timestamp - ((time_t) timestamp)) * 1000);
      string::size_type attrStart = pvssDpName.find(".._value");
      if (attrStart < pvssDpName.length() && attrStart > 0)
      {
        pvssDpName.replace(attrStart, string::npos, ".._stime");           
        if ((result = getDpId(pvssDpName, dpId)) != SA_NO_ERROR)
        {        
          retValTS = Manager::dpSet(dpId, ts);
        }
      }
    }
    if (retVal == PVSS_FALSE || retValTS == PVSS_FALSE)
    {
      ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
                    ErrClass::ERR_PARAM,        // wrong name: blame others
                    ErrClass::UNEXPECTEDSTATE,  // fits all
                    "GSAService",               // our file name
                    "dpeSet()",                      // our function name
                    CharString("Value of datapoint ") + dpeName.c_str() + 
                    CharString(" could not be set"));
  
      LOG_ERROR(formatString (
          "PVSS: Unable to set value of property: '%s' (with 'answer')",
          dpeName.c_str()));
  
      result = SA_SETPROP_FAILED;
      // default the PVSS API is configured to delete this object
      // but if there is an error occured PVSS API will not do this
      delete pWFA;
    }
    else
    {
      if (wantAnswer)
      {
        LOG_DEBUG(formatString (
            "Setting the value of property '%s' is requested successful", 
            dpeName.c_str()));
      }
      else
      {
        LOG_DEBUG(formatString (
            "Property value '%s' is set successful", 
            dpeName.c_str()));
      }
    }
  }
  if (pVar)
  {
    delete pVar; // constructed by convertMACToPVSS method
  }
  return result;
}

TSAResult GSAService::dpQuerySubscribeSingle(const string& queryWhere, const string& queryFrom)
{
  TSAResult result(SA_NO_ERROR);
  const char queryFormat[] = "SELECT '_online.._value', '_online.._stime','_online.._manager' FROM %s WHERE %s";

  CharString query;
  query.format(queryFormat, queryWhere.c_str(), queryFrom.c_str());

  LOG_INFO(formatString (
      "Subscription on queried properties '%s'", 
      (const char*)query));
  
  assert(_pSCADAHandler);
  if ((result = _pSCADAHandler->isOperational()) != SA_NO_ERROR)
  {
    LOG_FATAL(formatString (
        "Unable to query subscriptions: '%s'", 
        (const char*) query));
  }
  else if (!Manager::dpQueryConnectSingle(query, PVSS_FALSE, _pWFA, PVSS_FALSE))
  {
    ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
                  ErrClass::ERR_PARAM,        // wrong name: blame others
                  ErrClass::UNEXPECTEDSTATE,  // fits all
                  "GSAService",               // our file name
                  "dpQuerySubscribeSingle()", // our function name
                  CharString("Query subscription (") + query + 
                  CharString(") could not be requested!"));

    LOG_ERROR(formatString (
        "PVSS: Unable to perform dpQueryConnectSingle: '%s'",
        (const char*) query));

    result = SA_QUERY_SUBSC_FAILED;    
  }
  else
  {
    LOG_DEBUG(formatString (
        "Query subscription (%s) was requested succesful", 
        (const char*) query));
  }
  return result;
}

TSAResult GSAService::dpQueryUnsubscribe(uint32 queryId)
{
  TSAResult result(SA_NO_ERROR);
  LOG_INFO(formatString (
      "Unsubscription from queried properties '%d'", 
      queryId));

  assert(_pSCADAHandler);
  if ((result = _pSCADAHandler->isOperational()) != SA_NO_ERROR)
  {
    LOG_FATAL(formatString (
        "Unable to unsubscribe: '%d'", 
        queryId));
  }
  else if (!Manager::dpQueryDisconnect(queryId, _pWFA, PVSS_FALSE))
  {
    ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
                  ErrClass::ERR_PARAM,        // wrong name: blame others
                  ErrClass::UNEXPECTEDSTATE,  // fits all
                  "GSAService",               // our file name
                  "dpQuerySubscribeSingle()",                      // our function name
                  CharString("Query unsubscription (") + queryId + 
                  CharString(") could not be requested!"));

    LOG_ERROR(formatString (
        "PVSS: Unable to perform dpQueryDisconnect: '%d'",
        queryId));

    result = SA_QUERY_UNSUBSC_FAILED;    
  }
  else
  {
    LOG_DEBUG(formatString (
        "Unsubscription of queried properties (%d) was requested succesful", 
        queryId));
  }
  return result;
}

TSAResult GSAService::convertPVSSToMAC(const Variable& variable, 
                                       GCFPValue** pMacValue) const
{
  TSAResult result(SA_NO_ERROR);
  *pMacValue = 0;
  switch (variable.isA())
  {
    case BIT_VAR: *pMacValue = new GCFPVBool(((BitVar *)&variable)->getValue()); break;
    case CHAR_VAR: *pMacValue = new GCFPVChar(((CharVar *)&variable)->getValue()); break;
    case UINTEGER_VAR: *pMacValue = new GCFPVUnsigned(((UIntegerVar *)&variable)->getValue()); break;
    case INTEGER_VAR: *pMacValue = new GCFPVInteger(((IntegerVar *)&variable)->getValue()); break;
    case FLOAT_VAR: *pMacValue = new GCFPVDouble(((FloatVar *)&variable)->getValue()); break;
    case TEXT_VAR: *pMacValue = new GCFPVString(((TextVar*)&variable)->getValue()); 
      break;
    case BLOB_VAR: 
    {
      const Blob* pBlob = &((BlobVar *)&variable)->getValue();
      *pMacValue = new GCFPVBlob((unsigned char*) pBlob->getData(), pBlob->getLen()); // no memcpy !!!
      break;
    }
    //case BIT32_VAR: *pMacValue = new GCFPVBit32(((Bit32Var *)&variable)->getValue()); break;
    //case TIME_VAR: *pMacValue = new GCFPVDateTime(((TimeVar *)&variable)->getValue()); break;
    case DYNBIT_VAR: 
    case DYNCHAR_VAR: 
    case DYNUINTEGER_VAR: 
    case DYNINTEGER_VAR: 
    case DYNFLOAT_VAR:
    case DYNTEXT_VAR:
    {
      TMACValueType type(NO_LPT);
      switch (variable.isA())
      {
        case DYNBIT_VAR: type = LPT_DYNBOOL; break;
        case DYNCHAR_VAR: type = LPT_DYNCHAR; break;
        case DYNUINTEGER_VAR: type = LPT_DYNUNSIGNED; break;
        case DYNINTEGER_VAR: type = LPT_DYNINTEGER; break;
        case DYNFLOAT_VAR: type = LPT_DYNDOUBLE; break;
        case DYNTEXT_VAR: type = LPT_DYNSTRING; break;
        case DYNBLOB_VAR: type = LPT_DYNBLOB; break;
        default: break;        
      }
        
      if (type != NO_LPT)
      {
        const DynVar* pDynVar = (const DynVar*) (&variable);
        GCFPValueArray arrayTo;
        GCFPValue* pItemValue(0);
        for (Variable* pVar = pDynVar->getFirst();
             pVar; pVar = pDynVar->getNext())
        {
          switch (pVar->isA())
          {
            case BIT_VAR:
              pItemValue = new GCFPVBool(((BitVar*)pVar)->getValue());
              break;
            case CHAR_VAR:
              pItemValue = new GCFPVChar(((CharVar*)pVar)->getValue());
              break;
            case INTEGER_VAR:
              pItemValue = new GCFPVInteger(((IntegerVar*)pVar)->getValue());
              break;
            case UINTEGER_VAR:
              pItemValue = new GCFPVUnsigned(((UIntegerVar*)pVar)->getValue());
              break;
            case FLOAT_VAR:
              pItemValue = new GCFPVDouble(((FloatVar*)pVar)->getValue());
              break;
            case TEXT_VAR:
              pItemValue = new GCFPVString(((TextVar*)pVar)->getValue()); 
              break;
            case BLOB_VAR: 
            {
              const Blob* pBlob = &((BlobVar *)&variable)->getValue();
              pItemValue = new GCFPVBlob((unsigned char*)pBlob->getData(), pBlob->getLen()); // no memcpy !!!
              break;
            }
            default:
              break;
          }
          arrayTo.push_back(pItemValue);
        }
        *pMacValue = new GCFPVDynArr(type, arrayTo);
        for (GCFPValueArray::iterator iter = arrayTo.begin();
             iter != arrayTo.end(); ++iter)
        {
          delete *iter;  
        }      
      }
      break;
    }
    default:
      result = SA_DPTYPE_UNKNOWN;
      LOG_ERROR(formatString (
          "DPE type not supported (yet)! (%d) see in Variable.hxx", 
          variable.isA()));
      break;
  }
  return result;
}

TSAResult GSAService::convertMACToPVSS(const GCFPValue& macValue, 
                                  Variable** pVar, 
                                  const DpIdentifier& dpId) const                                  
{
  TSAResult result(SA_NO_ERROR);
  DpElementType elTypeId(DPELEMENT_NOELEMENT);
  *pVar = 0;
  PVSSboolean res = Manager::getElementType(dpId, elTypeId);
  if (res == PVSS_FALSE)
  {
    LOG_FATAL("PVSS: Could not get dpeType");   
    return SA_DPTYPE_UNKNOWN;
  }
  switch (macValue.getType())
  {
    case LPT_BOOL:
      if (elTypeId == DPELEMENT_BIT)*pVar = new BitVar(((GCFPVBool*)&macValue)->getValue());
      break;
    case LPT_CHAR:
      if (elTypeId == DPELEMENT_CHAR) *pVar = new CharVar(((GCFPVChar*)&macValue)->getValue());
      break;
    case LPT_UNSIGNED:
      if (elTypeId == DPELEMENT_UINT) *pVar = new UIntegerVar(((GCFPVUnsigned*)&macValue)->getValue());
      break;
    case LPT_INTEGER:
      if (elTypeId == DPELEMENT_INT) *pVar = new IntegerVar(((GCFPVInteger*)&macValue)->getValue());
      break;
    case LPT_DOUBLE:
      if (elTypeId == DPELEMENT_FLOAT) *pVar = new FloatVar(((GCFPVDouble*)&macValue)->getValue());
      break;
    case LPT_STRING:
      if (elTypeId == DPELEMENT_TEXT) *pVar = new TextVar(((GCFPVString*)&macValue)->getValue().c_str());
      break;
    case LPT_BLOB:
      if (elTypeId == DPELEMENT_BLOB)
      {
        *pVar = new BlobVar(((GCFPVBlob*)&macValue)->getValue(), ((GCFPVBlob*)&macValue)->getLen(), PVSS_TRUE);
      }
      break;
/*    case LPT_DATETIME:
      if (elTypeId == DPELEMENT_TIME) *pVar = new TimeVar(((GCFPVDateTime*)&macValue)->getValue());
      break;
    case LPT_BIT32:
      if (elTypeId == DPELEMENT_BIT32) *pVar = new Bit32Var(((GCFPVBit32 *)&macValue)->getValue());
      break;*/
    default:
      if (macValue.getType() > LPT_DYNARR && 
          macValue.getType() < END_DYNLPT)
      {        
        Variable* pItemValue(0);
        VariableType type(NOTYPE_VAR);
        // the type for the new FPValue must be determined 
        // separat, because the array could be empty
        switch (macValue.getType())
        {
          case LPT_DYNBOOL:
            if (elTypeId == DPELEMENT_DYNBIT) type = BIT_VAR;
            break;
          case LPT_DYNCHAR:
            if (elTypeId == DPELEMENT_DYNCHAR) type = CHAR_VAR;
            break;
          case LPT_DYNINTEGER:
            if (elTypeId == DPELEMENT_DYNINT) type = INTEGER_VAR;
            break;
          case LPT_DYNUNSIGNED:
            if (elTypeId == DPELEMENT_DYNUINT) type = UINTEGER_VAR;
            break;
          case LPT_DYNDOUBLE:
            if (elTypeId == DPELEMENT_DYNFLOAT) type = FLOAT_VAR;
            break;
          case LPT_DYNSTRING:
            if (elTypeId == DPELEMENT_DYNTEXT) type = TEXT_VAR;
            break;
          case LPT_DYNBLOB:
            if (elTypeId == DPELEMENT_DYNBLOB) type = BLOB_VAR;
            break;
          case LPT_DYNREF:
            if (elTypeId == DPELEMENT_DYNBLOB) type = TEXT_VAR;
            break;
          default:
            break;          
        }
        if (type == NOTYPE_VAR)
        {
          // type mismatch so stop with converting data
          break;
        }
        *pVar = new DynVar(type);
        GCFPValue* pValue;
        const GCFPValueArray& arrayFrom = ((GCFPVDynArr*)&macValue)->getValue();
        for (GCFPValueArray::const_iterator iter = arrayFrom.begin();
             iter != arrayFrom.end(); ++iter)
        {
          pValue = (*iter);
          switch (pValue->getType())
          {
            case LPT_BOOL:
              pItemValue  = new BitVar(((GCFPVBool*)pValue)->getValue());
              break;
            case LPT_CHAR:
              pItemValue  = new CharVar(((GCFPVChar*)pValue)->getValue());
              break;
            case LPT_INTEGER:
              pItemValue  = new IntegerVar(((GCFPVInteger*)pValue)->getValue());
              break;
            case LPT_UNSIGNED:
              pItemValue  = new UIntegerVar(((GCFPVUnsigned*)pValue)->getValue());
              break;
            case LPT_DOUBLE:
              pItemValue  = new FloatVar(((GCFPVDouble*)pValue)->getValue());
              break;
            case LPT_STRING:
              pItemValue  = new TextVar(((GCFPVString*)pValue)->getValue().c_str());
              break;
            case LPT_BLOB:
              pItemValue = new BlobVar(((GCFPVBlob*)pValue)->getValue(), ((GCFPVBlob*)pValue)->getLen(), PVSS_TRUE);
              break;
            default:
              break;              
          }
          if (pItemValue)
          {
            ((DynVar *)(*pVar))->append(*pItemValue);
            delete pItemValue;
            pItemValue = 0;
          }
        }
      }
      else
        result = SA_MACTYPE_UNKNOWN;
      break;
  }
  if (result == SA_NO_ERROR && *pVar == 0)
  {
    CharString valueTypeName;
    getPVSSType(macValue.getType(), valueTypeName);
    LOG_ERROR(formatString (
        "Type mismatch! Property type: %s != Value type: %d (see in DpElemnentType.hxx)",         
        (const char*) valueTypeName,
        elTypeId));   
    
    result = SA_MACTYPE_MISMATCH;
  }
  
  return result;
}

bool GSAService::getPVSSType(TMACValueType macType, 
                             CharString& pvssTypeName) const
{
  switch (macType)
  {
    case LPT_BOOL:
      pvssTypeName = "LPT_BOOL";
      break;
    case LPT_CHAR:
      pvssTypeName = "LPT_CHAR";
      break;
    case LPT_REF:
      pvssTypeName = "LPT_REF";
      break;
    case LPT_UNSIGNED:
      pvssTypeName = "LPT_UNSIGNED";
      break;
    case LPT_INTEGER:
      pvssTypeName = "LPT_INTEGER";
      break;
    case LPT_DOUBLE:
      pvssTypeName = "LPT_DOUBLE";
      break;
    case LPT_STRING:
      pvssTypeName = "LPT_STRING";
      break;
    case LPT_BLOB:
      pvssTypeName = "LPT_BLOB";
      break;
    case LPT_DYNBOOL:
      pvssTypeName = "LPT_DYNBOOL";
      break;
    case LPT_DYNCHAR:
      pvssTypeName = "LPT_DYNCHAR";
      break;
    case LPT_DYNUNSIGNED:
      pvssTypeName = "LPT_DYNUNSIGNED";
      break;
    case LPT_DYNINTEGER:
      pvssTypeName = "LPT_DYNINTEGER";
      break;
    case LPT_DYNDOUBLE:
      pvssTypeName = "LPT_DYNDOUBLE";
      break;
    case LPT_DYNSTRING:
      pvssTypeName = "LPT_DYNSTRING";
      break;
    case LPT_DYNBLOB:
      pvssTypeName = "LPT_DYNBLOB";
      break;
    case LPT_DYNREF:
      pvssTypeName = "LPT_DYNREF";
      break;
    default:
      return false;
  }  
  
  return true;
}

TSAResult GSAService::getDpId(const string& dpName, DpIdentifier& dpId) const
{
  TSAResult result(SA_NO_ERROR);

  CharString pvssDpName(dpName.c_str());
  
  // Ask the Identification for the DpId of our Datapoints
  if (Manager::getId(pvssDpName, dpId) == PVSS_FALSE)
  {
    // This name was unknown.
    // The parameters are in Bascis/ErrClass.hxx
    ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
                  ErrClass::ERR_PARAM,        // wrong name: blame others
                  ErrClass::UNEXPECTEDSTATE,  // fits all
                  "GSAService",              // our file name
                  "getDpId",                      // our function name
                  CharString("Datapoint ") + pvssDpName + CharString(" missing"));

    LOG_DEBUG(formatString (
            "PVSS: Datapoint '%s' missing", 
            dpName.c_str()));   

    result = SA_PROPNAME_MISSING;
  }

  return result;
}

void GSAService::convPropToDpConfig(const string& propName, string& pvssDpName, bool willReadValue)
{
  pvssDpName = propName.c_str();
  if (propName.find('.') >= propName.size())
  {
      pvssDpName += ".";
  }
  if (willReadValue) 
  {
    pvssDpName += ":_online.._value";
  }
  else
  {
    pvssDpName += ":_original.._value";
  }
}

void GSAService::convDpConfigToProp(const string& pvssDPEConfigName, string& propName)
{
  string::size_type doublePointPos = pvssDPEConfigName.find(':');
  string::size_type dotPos = pvssDPEConfigName.find('.');
  string::size_type nrOfCharsToCopy(pvssDPEConfigName.length());
  if (doublePointPos < dotPos)
  {
    doublePointPos = pvssDPEConfigName.find(':', doublePointPos + 1);
  }
  
  if (doublePointPos < pvssDPEConfigName.length())
  {
    nrOfCharsToCopy = doublePointPos;
  }
  else 
  {
    doublePointPos = pvssDPEConfigName.length();
  }
  if ((doublePointPos - 1) == dotPos)
    nrOfCharsToCopy--;
  
  propName.assign(pvssDPEConfigName, 0, nrOfCharsToCopy); 
}
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
