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

#include "GSA_Service.h"
#include "GSA_WaitForAnswer.h"
#include "GSA_Defines.h"
#include "GSA_SCADAHandler.h"

#include "GCF_PVBool.h"
#include "GCF_PVChar.h"
#include "GCF_PVInteger.h"
#include "GCF_PVUnsigned.h"
#include "GCF_PVDouble.h"
#include "GCF_PVString.h"
#include "GCF_PVDynArr.h"

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

GSAService::GSAService() : _pWFA(0)
{
  _pWFA = new GSAWaitForAnswer(*this);
  if (GSASCADAHandler::instance()->isOperational() == SA_SCADA_NOT_AVAILABLE)
  {
    LOFAR_LOG_ERROR(SAL_STDOUT_LOGGER, (
        "Error on creating a SCADA service"));
    Manager::exit(-1);
  }
}

GSAService::~GSAService()
{
  if (_pWFA)
    delete _pWFA;
}

// Receive Signals.
// We are interested in SIGINT and SIGTERM. 
void GSAService::handleHotLink(const DpMsgAnswer& answer)
{
  CharString pvssDPEConfigName;
  string DPEConfigName;
  string propName;
  Variable *varPtr;
  bool handled(false);
  GCFPValue* pPropertyValue(0);
  CharString pvssTypeName;
  
  for (AnswerGroup *pGrItem = answer.getFirstGroup();
       pGrItem; pGrItem = answer.getNextGroup())
  {
    for (AnswerItem *pAnItem = pGrItem->getFirstItem(); pAnItem;
         pAnItem = pGrItem->getNextItem())
    {
      if (pAnItem->getDpIdentifier().convertToString(pvssDPEConfigName) == PVSS_FALSE)
      {
        LOFAR_LOG_FATAL(SAL_STDOUT_LOGGER, (
            "PVSS: Could not convert dpIdentifier '%d'", 
            pAnItem->getDpIdentifier().getDp()));   
      }
      else
      { 
        DPEConfigName = pvssDPEConfigName;
        convDpConfigToProp(DPEConfigName, propName);
        handled = true;
        switch (answer.isAnswerOn())
        {
          case DP_MSG_CONNECT:
            propSubscribed(propName);
            break;
          case DP_MSG_DISCONNECT:
            propUnsubscribed(propName);
            break;
          case DP_MSG_REQ_NEW_DP:
            propCreated(propName);
            break;
          case DP_MSG_CMD_NEWDEL_DP:
            propDeleted(propName);
            break;
          case DP_MSG_SIMPLE_REQUEST:
          {
            varPtr = pAnItem->getValuePtr();
            if (varPtr)      // could be NULL !!
            {
              if (Manager::getTypeName(pAnItem->getDpIdentifier().getDpType(), 
                                        pvssTypeName) == PVSS_FALSE)
              {
                LOFAR_LOG_FATAL(SAL_STDOUT_LOGGER, (
                    "PVSS: Could not get dpTypeName '%d'", 
                    pAnItem->getDpIdentifier().getDpType()));   
              }
              else if (convertPVSSToMAC(*varPtr, pvssTypeName, 
                                        &pPropertyValue) != SA_NO_ERROR)
              {
                LOFAR_LOG_ERROR(SAL_STDOUT_LOGGER, (
                    "Could not convert PVSS DP (%d) to MAC property", 
                    (const char*)pvssTypeName));   
              }
              else
              {
                LOFAR_LOG_DEBUG(SAL_STDOUT_LOGGER, (
                    "Value of '%s' has get", 
                    propName.c_str()));   
                propValueGet(propName, *pPropertyValue);
              }
              if (pPropertyValue)
                delete pPropertyValue; // constructed by convertPVSSToMAC method
            }
            break;
          }           
          default:
            handled = false;
            break;        
        }
      }
    }
    if (!handled)
    {
      ErrClass* pError = pGrItem->getError();
      if (pError)
        LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
            "Error (%s) in answer on: %d",
            ErrClass::getErrorText(pError->getErrorId()),
            answer.isAnswerOn()));
    }
  }
  if (!handled)
  {
    LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
        "Answer on: %d is not handled",
        answer.isAnswerOn()));   
  }  
}

// Handle incoming hotlinks.
// This function is called from our hotlink object
void GSAService::handleHotLink(const DpHLGroup& group)
{
  CharString pvssDPEConfigName;
  CharString pvssTypeName;
  string DPEConfigName;
  string propName;
  GCFPValue* pPropertyValue(0);

  LOFAR_LOG_DEBUG(SAL_STDOUT_LOGGER, (
    "Receiving hotlink"));   

  // A group consists of pairs of DpIdentifier and values called items.
  // There is exactly one item for all configs we are connected.

  for (DpVCItem *item = group.getFirstItem(); item;
       item = group.getNextItem())
  {
    Variable *varPtr = item->getValuePtr();
    if (varPtr)      // could be NULL !!
    {
      if (item->getDpIdentifier().convertToString(pvssDPEConfigName) == PVSS_FALSE)
      {
        LOFAR_LOG_FATAL(SAL_STDOUT_LOGGER, (
            "PVSS: Could not convert dpIdentifier '%d'", 
            item->getDpIdentifier().getDp()));   
      }
      else if (Manager::getTypeName(item->getDpIdentifier().getDpType(), pvssTypeName) == PVSS_FALSE)
      {
        LOFAR_LOG_FATAL(SAL_STDOUT_LOGGER, (
            "PVSS: Could not get dpTypeName '%d'", 
            item->getDpIdentifier().getDpType()));   
      }
      else if (convertPVSSToMAC(*varPtr, pvssTypeName, &pPropertyValue) != SA_NO_ERROR)
      {
        LOFAR_LOG_ERROR(SAL_STDOUT_LOGGER, (
            "Could not convert PVSS DP (%d) to MAC property", 
            (const char*)pvssTypeName));   
      }
      else
      {
        DPEConfigName = pvssDPEConfigName;
        convDpConfigToProp(DPEConfigName, propName);        
        LOFAR_LOG_DEBUG(SAL_STDOUT_LOGGER, (
            "Value of '%s' has changed", 
            propName.c_str()));   
        propValueChanged(propName, *pPropertyValue);
      }
      if (pPropertyValue)
        delete pPropertyValue; // constructed by convertPVSSToMAC method
    }
  }
}

TSAResult GSAService::createProp(const string& propName, 
                                 GCFPValue::TMACValueType macType)
{
  TSAResult result(SA_NO_ERROR);
  
  DpTypeId dpTypeId;
  LangText dpNameLang(propName.c_str());
  GSAWaitForAnswer *pWFA = new GSAWaitForAnswer(*this);
  CharString pvssTypeName;

  LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
      "Create property '%s'", 
      propName.c_str()));

  if ((result = GSASCADAHandler::instance()->isOperational()) == SA_SCADA_NOT_AVAILABLE)
  {
    LOFAR_LOG_ERROR(SAL_STDOUT_LOGGER, (
        "Unable to create property: '%s'", 
        propName.c_str()));    
  }
  else if (exists(propName))
  {
    LOFAR_LOG_ERROR(SAL_STDOUT_LOGGER, (
        "Property: '%s' already exists", 
        propName.c_str()));
    result = SA_PROP_ALREADY_EXIST;    
  }
  else if (!getPVSSType(macType, pvssTypeName))
  {
    LOFAR_LOG_ERROR(SAL_STDOUT_LOGGER, (
        "Property: '%s' already exists", 
        propName.c_str()));
    result = SA_MACTYPE_UNKNOWN;
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

    LOFAR_LOG_ERROR(SAL_STDOUT_LOGGER, (
        "PVSS: DatapointType '%s' unknown", 
        (const char*) pvssTypeName));

    result = SA_MACTYPE_UNKNOWN;
            
  }
  else if (Manager::dpCreate(dpNameLang, dpTypeId, pWFA) == PVSS_FALSE)
  {
    ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
                  ErrClass::ERR_PARAM,        // wrong name: blame others
                  ErrClass::UNEXPECTEDSTATE,  // fits all
                  "GSAService",              // our file name
                  "createProp",                      // our function name
                  CharString("Datapoint ") + 
                    propName.c_str() + 
                    CharString(" could not be created"));

    LOFAR_LOG_ERROR(SAL_STDOUT_LOGGER, (
        "PVSS: Unable to create property: '%s'", 
        propName.c_str()));

    result = SA_CREATEPROP_FAILED;
  }
  else
  {
    LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
        "Creation of property '%s' was requested successful", 
        propName.c_str()));
  }
  return result;
}

TSAResult GSAService::deleteProp(const string& propName)
{
  TSAResult result(SA_NO_ERROR);  
  DpIdentifier dpId;
  GSAWaitForAnswer *pWFA = new GSAWaitForAnswer(*this);

  LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
      "Delete property '%s'", 
      propName.c_str()));
  
  if ((result = GSASCADAHandler::instance()->isOperational()) != SA_NO_ERROR)
  {
    LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
        "Unable to delete property: '%s'", 
        propName.c_str()));
  }
  else if (!exists(propName))
  {
    LOFAR_LOG_ERROR(SAL_STDOUT_LOGGER, (
        "Property: '%s' does not exists", 
        propName.c_str()));    
    result = SA_PROP_DOES_NOT_EXIST;
  }
  else if ((result = getDpId(propName, dpId)) != SA_NO_ERROR)
  {
    LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
        "Unable to delete property: '%s'", 
        propName.c_str()));
  }
  else if (Manager::dpDelete(dpId, pWFA) == PVSS_FALSE)
  {
    ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
                  ErrClass::ERR_PARAM,        // wrong name: blame others
                  ErrClass::UNEXPECTEDSTATE,  // fits all
                  "GSAService",               // our file name
                  "deleteProp",               // our function name
                  CharString("Datapoint ") + propName.c_str() + 
                  CharString(" could not be deleted"));

    LOFAR_LOG_ERROR(SAL_STDOUT_LOGGER, (
        "PVSS: Unable to delete property: '%s'", 
        propName.c_str()));

    result = SA_DELETEPROP_FAILED;
  }
  else
  {
    LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
        "Deletion of property '%s' was requested successful", 
        propName.c_str()));
  }

  return result;
}

TSAResult GSAService::subscribe(const string& propName)
{
  TSAResult result(SA_NO_ERROR);
  
  DpIdentifier dpId;
  string pvssDpName;

  convPropToDpConfig(propName, pvssDpName, true);

  LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
      "Subscribe on property '%s'", 
      propName.c_str()));
  
  if ((result = GSASCADAHandler::instance()->isOperational()) != SA_NO_ERROR)
  {
    LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
        "Unable to subscribe on property: '%s'", 
        propName.c_str()));
  }
  else if (!exists(propName))
  {
    LOFAR_LOG_ERROR(SAL_STDOUT_LOGGER, (
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
                    "subscribe",               // our function name
                    CharString("Datapoint ") + propName.c_str() + 
                    CharString(" could not be connected"));
  
      LOFAR_LOG_ERROR(SAL_STDOUT_LOGGER, (
          "PVSS: Unable to subscribe on property: '%s'", 
          propName.c_str()));
  
      result = SA_SUBSCRIBEPROP_FAILED;
    }
    else
    {
      LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
          "Subscription on property '%s' was requested successful", 
          propName.c_str()));
    }
  }
  else
  {
    
    LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
        "Unable to subscribe on property: '%s'", 
        propName.c_str()));
  }    
  return result;
}

TSAResult GSAService::unsubscribe(const string& propName)
{
  TSAResult result(SA_NO_ERROR);
  DpIdentifier dpId;
  string pvssDpName;

  convPropToDpConfig(propName, pvssDpName, true);

  LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
      "Unsubscribe from property '%s'", 
      propName.c_str()));
  
  if ((result = GSASCADAHandler::instance()->isOperational()) != SA_NO_ERROR)
  {
    LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
        "Unable to unsubscribe from property: '%s'", 
        propName.c_str()));
  }
  else if (!exists(propName))
  {
    LOFAR_LOG_ERROR(SAL_STDOUT_LOGGER, (
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
                    "unsubscribe",               // our function name
                    CharString("Datapoint ") + propName.c_str() + 
                    CharString(" could not be disconnected"));

      LOFAR_LOG_ERROR(SAL_STDOUT_LOGGER, (
          "PVSS: Unable to unsubscribe from property: '%s'", 
          propName.c_str()));

      result = SA_UNSUBSCRIBEPROP_FAILED;
    }
    else
    {
      LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
          "Unsubscription from property '%s' was requested successful", 
          propName.c_str()));
    }
  }
  else
  {
    LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
        "Unable to unsubscribe from property: '%s'", 
        propName.c_str()));
  }
  
  return result;
}

TSAResult GSAService::get(const string& propName)
{
  TSAResult result(SA_NO_ERROR);
  DpIdentifier dpId;
  string pvssDpName;

  convPropToDpConfig(propName, pvssDpName, true);

  LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
      "Request value of property '%s'", 
      propName.c_str()));
  
  if ((result = GSASCADAHandler::instance()->isOperational()) != SA_NO_ERROR)
  {
    LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
        "Unable to request of property: '%s'", 
        propName.c_str()));
    result = SA_PROP_DOES_NOT_EXIST;      
  }
  else if (!exists(propName))
  {
    LOFAR_LOG_ERROR(SAL_STDOUT_LOGGER, (
        "Property: '%s' does not exists", 
        propName.c_str()));    
  }
  else if ((result = getDpId(pvssDpName, dpId)) != SA_NO_ERROR)
  {
    LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
        "Unable to request value of property: '%s'", 
        propName.c_str()));
  }
  else if (Manager::dpGet(dpId, _pWFA, PVSS_FALSE) == PVSS_FALSE)
  {
    ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
                  ErrClass::ERR_PARAM,        // wrong name: blame others
                  ErrClass::UNEXPECTEDSTATE,  // fits all
                  "GSAService",               // our file name
                  "get",                      // our function name
                  CharString("Value of datapoint ") + propName.c_str() + 
                  CharString(" could not be requested"));

    LOFAR_LOG_ERROR(SAL_STDOUT_LOGGER, (
        "PVSS: Unable to request value of property: '%s'", 
        propName.c_str()));

    result = SA_GETPROP_FAILED;
  }
  else
  {
    LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
        "Value of property '%s' was requested successful", 
        propName.c_str()));
  }
  
  return result;
}

TSAResult GSAService::set(const string& propName, const GCFPValue& value)
{
  TSAResult result(SA_NO_ERROR);
  DpIdentifier dpId;
  Variable* pVar(0);
  string pvssDpName;

  convPropToDpConfig(propName, pvssDpName, false);

  LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
      "Set value of property '%s'", 
      propName.c_str()));
  
  if ((result = GSASCADAHandler::instance()->isOperational()) != SA_NO_ERROR)
  {
    LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
        "Unable to set value of property: '%s'", 
        propName.c_str()));
  }
  else if (!exists(propName))
  {
    LOFAR_LOG_ERROR(SAL_STDOUT_LOGGER, (
        "Property: '%s' does not exists", 
        propName.c_str()));    
    result = SA_PROP_DOES_NOT_EXIST;      
  }
  else if ((result = getDpId(pvssDpName, dpId)) != SA_NO_ERROR)
  {
    LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
        "Unable to set value of property: '%s'", 
        propName.c_str()));
  }
  else if ((result = convertMACToPVSS(value, &pVar)) != SA_NO_ERROR)
  {
    LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
        "Unable to set value of property: '%s'", 
        propName.c_str()));
  }
  else if (Manager::dpSet(dpId, *pVar) == PVSS_FALSE)
  {
    ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
                  ErrClass::ERR_PARAM,        // wrong name: blame others
                  ErrClass::UNEXPECTEDSTATE,  // fits all
                  "GSAService",               // our file name
                  "set",                      // our function name
                  CharString("Value of datapoint ") + propName.c_str() + 
                  CharString(" could not be set"));

    LOFAR_LOG_ERROR(SAL_STDOUT_LOGGER, (
        "PVSS: Unable to set value of property: '%s'", 
        propName.c_str()));

    result = SA_SETPROP_FAILED;
  }
  else
  {
    LOFAR_LOG_TRACE(SAL_STDOUT_LOGGER, (
        "Property value '%s' is set successful", 
        propName.c_str()));
  }
  if (pVar)
  {
    delete pVar; // constructed by convertMACToPVSS method
  }
  return result;
}

bool GSAService::exists(const string& propName)
{
  DpIdentifier dpId;
  if (Manager::getId(propName.c_str(), dpId) == PVSS_FALSE)
    return false;
  else
    return true;
}

TSAResult GSAService::convertPVSSToMAC(const Variable& variable, 
                                  const CharString& typeName, 
                                  GCFPValue** pMacValue) const
{
  TSAResult result(SA_NO_ERROR);
  if (typeName == "BOOL_VAL")
  {
    if (variable.isA() == BIT_VAR)
      *pMacValue = new GCFPVBool(((BitVar *)&variable)->getValue());
    else
      result = SA_VARIABLE_WRONG_TYPE;
  }
  else if (typeName == "CHAR_VAL")
  {
    if (variable.isA() == CHAR_VAR)
      *pMacValue = new GCFPVChar(((CharVar *)&variable)->getValue());
    else
      result = SA_VARIABLE_WRONG_TYPE;
  }
  else if (typeName == "UNSIGNED_VAL")
  {
    if (variable.isA() == UINTEGER_VAR)
      *pMacValue = new GCFPVUnsigned(((UIntegerVar *)&variable)->getValue());
    else
      result = SA_VARIABLE_WRONG_TYPE;
  }
  else if (typeName == "INTEGER_VAL")
  {
    if (variable.isA() == INTEGER_VAR)
      *pMacValue = new GCFPVInteger(((IntegerVar *)&variable)->getValue());
    else
      result = SA_VARIABLE_WRONG_TYPE;
  }
  else if (typeName == "FLOAT_VAL")
  {
    if (variable.isA() == FLOAT_VAR)
      *pMacValue = new GCFPVDouble(((FloatVar *)&variable)->getValue());
    else
      result = SA_VARIABLE_WRONG_TYPE;     
  }
  else if (typeName == "STRING_VAL")
  {
    if (variable.isA() == TEXT_VAR)
      *pMacValue = new GCFPVString(((TextVar *)&variable)->getValue());
    else
      result = SA_VARIABLE_WRONG_TYPE;     
  }
  else if (typeName.ncmp("DYN", 3))
  {
    const DynVar* pDynVar = static_cast<const DynVar*>(&variable);
    if (pDynVar)
    {
      GCFPValueArray arrayTo;
      GCFPValue* pItemValue(0);
      GCFPValue::TMACValueType type(GCFPValue::NO_VAL);
      // the type for the new FPValue must be determined 
      // separate, because the array could be empty
      switch (DynVar::getItemType(pDynVar->isA()))
      {
        case BIT_VAR:
          type = GCFPValue::DYNBOOL_VAL;
          break;
        case CHAR_VAR:
          type = GCFPValue::DYNCHAR_VAL;
          break;
        case INTEGER_VAR:
          type = GCFPValue::DYNINTEGER_VAL;
          break;
        case UINTEGER_VAR:
          type = GCFPValue::DYNUNSIGNED_VAL;
          break;
        case FLOAT_VAR:
          type = GCFPValue::DYNDOUBLE_VAL;
          break;
        case TEXT_VAR:
          type = GCFPValue::DYNSTRING_VAL;
          break;
      }
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
        }
        arrayTo.push_back(pItemValue);
      }
      *pMacValue = new GCFPVDynArr(type, arrayTo);
    }
  }
/*  else if (typeName == "BIT32_VAL")
  {
    *pMacValue = new GCFPVBit32(((Bit32Var *)&variable)->getValue());
  }
  else if (typeName == "REF_VAL")
  {
    *pMacValue = new GCFPVRef(((TextVar *)&variable)->getValue());
  }
  else if (typeName == "BLOB_VAL")
  {
    *pMacValue = new GCFPVBlob(((BlobVar *)&variable)->getValue());
  }
  else if (typeName == "DATETIME_VAL")
  {
    *pMacValue = new GCFPVDateTime(((TimeVar *)&variable)->getValue());
  }*/
  else 
  {
    result = SA_DPTYPE_UNKNOWN;
  }
  return result;
}

TSAResult GSAService::convertMACToPVSS(const GCFPValue& macValue, 
                                  Variable** pVar) const                                  
{
  TSAResult result(SA_NO_ERROR);
  switch (macValue.getType())
  {
    case GCFPValue::BOOL_VAL:
      *pVar = new BitVar(((GCFPVBool*)&macValue)->getValue());
      break;
    case GCFPValue::CHAR_VAL:
      *pVar = new CharVar(((GCFPVChar*)&macValue)->getValue());
      break;
    case GCFPValue::UNSIGNED_VAL:
      *pVar = new UIntegerVar(((GCFPVUnsigned*)&macValue)->getValue());
      break;
    case GCFPValue::INTEGER_VAL:
      *pVar = new IntegerVar(((GCFPVInteger*)&macValue)->getValue());
      break;
    case GCFPValue::DOUBLE_VAL:
      *pVar = new FloatVar(((GCFPVDouble*)&macValue)->getValue());
      break;
    case GCFPValue::STRING_VAL:
      *pVar = new TextVar(((GCFPVString*)&macValue)->getValue().c_str());
      break;
/*    case GCFPValue::REF_VAL:
      *pVar = new TextVar(((GCFPVRef*)&macValue)->getValue());
      break;
    case GCFPValue::BLOB_VAL:
      *pVar = new BlobVar(((GCFPVBlob*)&macValue)->getValue());
      break;
    case GCFPValue::DATETIME_VAL:
      *pVar = new TimeVar(((GCFPVDateTime*)&macValue)->getValue());
      break;
    case GCFPValue::BIT32_VAL:
      *pVar = new Bit32Var(((GCFPVBit32 *)&macValue)->getValue());
      break;*/
    default:
      if (macValue.getType() > GCFPValue::DYNARR_VAL && 
          macValue.getType() <= GCFPValue::DYNSTRING_VAL)
      {        
        Variable* pItemValue;
        VariableType type(NOTYPE_VAR);
        // the type for the new FPValue must be determined 
        // separat, because the array could be empty
        switch (macValue.getType())
        {
          case GCFPValue::DYNBOOL_VAL:
            type = BIT_VAR;
            break;
          case GCFPValue::DYNCHAR_VAL:
            type = CHAR_VAR;
            break;
          case GCFPValue::DYNINTEGER_VAL:
            type = INTEGER_VAR;
            break;
          case GCFPValue::DYNUNSIGNED_VAL:
            type = UINTEGER_VAR;
            break;
          case GCFPValue::DYNDOUBLE_VAL:
            type = FLOAT_VAR;
            break;
          case GCFPValue::DYNSTRING_VAL:
            type = TEXT_VAR;
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
            case GCFPValue::BOOL_VAL:
              pItemValue  = new BitVar(((GCFPVBool*)pValue)->getValue());
              break;
            case GCFPValue::CHAR_VAL:
              pItemValue  = new CharVar(((GCFPVChar*)pValue)->getValue());
              break;
            case GCFPValue::INTEGER_VAL:
              pItemValue  = new IntegerVar(((GCFPVInteger*)pValue)->getValue());
              break;
            case GCFPValue::UNSIGNED_VAL:
              pItemValue  = new UIntegerVar(((GCFPVUnsigned*)pValue)->getValue());
              break;
            case GCFPValue::DOUBLE_VAL:
              pItemValue  = new FloatVar(((GCFPVDouble*)pValue)->getValue());
              break;
            case GCFPValue::STRING_VAL:
              pItemValue  = new TextVar(((GCFPVString*)pValue)->getValue().c_str());
              break;
          }
          if (pItemValue)
            ((DynVar *)(*pVar))->append(*pItemValue);
        }
      }
      else
        result = SA_MACTYPE_UNKNOWN;
      break;
  }  
  
  return result;
}

bool GSAService::getPVSSType(GCFPValue::TMACValueType macType, 
                             CharString& pvssTypeName) const
{
  switch (macType)
  {
    case GCFPValue::BOOL_VAL:
      pvssTypeName = "BOOL_VAL";
      break;
    case GCFPValue::CHAR_VAL:
      pvssTypeName = "CHAR_VAL";
      break;
    case GCFPValue::UNSIGNED_VAL:
      pvssTypeName = "UNSIGNED_VAL";
      break;
    case GCFPValue::INTEGER_VAL:
      pvssTypeName = "INTEGER_VAL";
      break;
    case GCFPValue::DOUBLE_VAL:
      pvssTypeName = "DOUBLE_VAL";
      break;
    case GCFPValue::STRING_VAL:
      pvssTypeName = "STRING_VAL";
      break;
    case GCFPValue::DYNBOOL_VAL:
      pvssTypeName = "DYNBOOL_VAL";
      break;
    case GCFPValue::DYNCHAR_VAL:
      pvssTypeName = "DYNCHAR_VAL";
      break;
    case GCFPValue::DYNUNSIGNED_VAL:
      pvssTypeName = "DYNUNSIGNED_VAL";
      break;
    case GCFPValue::DYNINTEGER_VAL:
      pvssTypeName = "DYNINTEGER_VAL";
      break;
    case GCFPValue::DYNDOUBLE_VAL:
      pvssTypeName = "DYNDOUBLE_VAL";
      break;
    case GCFPValue::DYNSTRING_VAL:
      pvssTypeName = "DYNSTRING_VAL";
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

    LOFAR_LOG_ERROR(SAL_STDOUT_LOGGER, (
            "PVSS: Datapoint '%s' missing", 
            dpName.c_str()));   

    result = SA_PROPNAME_MISSING;
  }

  return result;
}

void GSAService::convPropToDpConfig(const string& propName, string& pvssDpName, bool read)
{
  pvssDpName = propName.c_str();
  if (propName.find('.') < propName.size())
  {
    if (read) 
      pvssDpName += ":_online.._value";
    else
      pvssDpName += ":_original.._value";
  }
  else
  {
    if (read) 
      pvssDpName += ".:_online.._value";
    else
      pvssDpName += ".:_original.._value";
  }
}

void GSAService::convDpConfigToProp(const string& pvssDPEConfigName, string& propName)
{
  size_t doublePointPos = pvssDPEConfigName.find(':');
  size_t dotPos = pvssDPEConfigName.find('.');
  size_t nrOfCharsToCopy(pvssDPEConfigName.size());
  size_t startPosToCopy(0);  
  if (doublePointPos < pvssDPEConfigName.size() || 
      dotPos < pvssDPEConfigName.size())
  {
    size_t secondDoublePointPos(pvssDPEConfigName.size());

    if (doublePointPos < pvssDPEConfigName.size())
    {
      secondDoublePointPos = pvssDPEConfigName.find(':', doublePointPos + 1);

      if (doublePointPos < dotPos)
      {
        startPosToCopy = doublePointPos + 1; 
      }
    }
    if (secondDoublePointPos < pvssDPEConfigName.size())
    {
      nrOfCharsToCopy = secondDoublePointPos - startPosToCopy;
      if ((secondDoublePointPos - 1) == dotPos)
        nrOfCharsToCopy--;
    }
    else 
    {
      nrOfCharsToCopy = pvssDPEConfigName.size() - startPosToCopy;
      if ((pvssDPEConfigName.size() - 1) == dotPos)
        nrOfCharsToCopy--;
    }
  }
  propName.assign(pvssDPEConfigName, startPosToCopy, nrOfCharsToCopy); 
}
