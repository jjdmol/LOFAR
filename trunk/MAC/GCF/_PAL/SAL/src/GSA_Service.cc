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
    LOG_ERROR(LOFAR::formatString (
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
void GSAService::handleHotLink(const DpMsgAnswer& answer, const GSAWaitForAnswer& wait)
{
  CharString pvssDPEConfigName;
  string DPEConfigName;
  string dpName;
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
        if (answer.isAnswerOn() == DP_MSG_DP_REQ)
        {
          LOG_INFO(LOFAR::formatString (
              "DP %s was deleted successfully", 
              wait.getDpName().c_str()));   
          dpDeleted(wait.getDpName());
          handled = true;
        }
        else
        {
          LOG_FATAL(LOFAR::formatString (
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
            LOG_INFO(LOFAR::formatString (
                "DPE %s was subscribed successfully", 
                dpName.c_str()));   
            dpeSubscribed(dpName);
            break;
          case DP_MSG_REQ_NEW_DP:
            LOG_INFO(LOFAR::formatString (
                "DP %s was created successfully", 
                dpName.c_str()));   
            dpCreated(dpName);
            break;
            break;
          case DP_MSG_SIMPLE_REQUEST:
          {
            varPtr = pAnItem->getValuePtr();
            if (varPtr)      // could be NULL !!
            {
              if (convertPVSSToMAC(*varPtr, pAnItem->getDpIdentifier(), 
                                        &pPropertyValue) != SA_NO_ERROR)
              {
                LOG_ERROR(LOFAR::formatString (
                    "Could not convert PVSS DP (type %s) to MAC property (%s)", 
                    (const char*)pvssTypeName, dpName.c_str()));   
              }
              else
              {
                LOG_DEBUG(LOFAR::formatString (
                    "Value of '%s' has get", 
                    dpName.c_str()));   
                dpeValueGet(dpName, *pPropertyValue);
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
      {
        LOG_DEBUG(LOFAR::formatString (
            "Error (%s) in answer on: %d",
            (const char*) pError->getErrorText(),
            answer.isAnswerOn()));
      }
    }
  }
  if (!handled)
  {
    LOG_DEBUG(LOFAR::formatString (
        "Answer on: %d is not handled",
        answer.isAnswerOn()));   
  }  
}

// Handle incoming hotlinks.
// This function is called from our hotlink object
void GSAService::handleHotLink(const DpHLGroup& group)
{
  CharString pvssDPEConfigName;
  string DPEConfigName;
  string dpName;
  GCFPValue* pPropertyValue(0);
  ErrClass* pErr(0);
  
  if ((pErr = group.getErrorPtr()) != 0)
  {
    LOG_INFO(LOFAR::formatString (
            "PVSS Error: (%s) in hotlink",
            (const char*) pErr->getErrorText()));
  }
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
        LOG_FATAL(LOFAR::formatString (
            "PVSS: Could not convert dpIdentifier '%d'", 
            item->getDpIdentifier().getDp()));   
      }
      else if (convertPVSSToMAC(*varPtr, item->getDpIdentifier(), &pPropertyValue) != SA_NO_ERROR)
      {
        DPEConfigName = pvssDPEConfigName;
        convDpConfigToProp(DPEConfigName, dpName);        
        LOG_ERROR(LOFAR::formatString (
            "Could not convert PVSS DP to MAC property (%s)", 
            dpName.c_str()));   
      }
      else
      {
        DPEConfigName = pvssDPEConfigName;
        convDpConfigToProp(DPEConfigName, dpName);        
        LOG_DEBUG(LOFAR::formatString (
            "Value of '%s' has changed", 
            dpName.c_str()));   
        dpeValueChanged(dpName, *pPropertyValue);
      }
      if (pPropertyValue)
        delete pPropertyValue; // constructed by convertPVSSToMAC method
    }
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

  LOG_INFO(LOFAR::formatString (
      "Create DP '%s'", 
      dpName.c_str()));

  if ((result = GSASCADAHandler::instance()->isOperational()) == SA_SCADA_NOT_AVAILABLE)
  {
    LOG_FATAL(LOFAR::formatString (
        "Unable to create DP: '%s'", 
        dpName.c_str()));    
  }
  else if (dpeExists(dpName))
  {
    LOG_WARN(LOFAR::formatString (
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

    LOG_FATAL(LOFAR::formatString (
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

    LOG_ERROR(LOFAR::formatString (
        "PVSS: Unable to create DP: '%s'", 
        dpName.c_str()));

    result = SA_CREATEPROP_FAILED;
  }
  else
  {
    LOG_DEBUG(LOFAR::formatString (
        "Creation of DP '%s' was requested successful", 
        dpName.c_str()));
  }
  return result;
}

TSAResult GSAService::dpDelete(const string& dpName)
{
  TSAResult result(SA_NO_ERROR);  
  DpIdentifier dpId;
  GSAWaitForAnswer *pWFA = new GSAWaitForAnswer(*this);
  pWFA->setDpName(dpName);

  LOG_INFO(LOFAR::formatString (
      "Delete DP '%s'", 
      dpName.c_str()));
  
  if ((result = GSASCADAHandler::instance()->isOperational()) != SA_NO_ERROR)
  {
    LOG_FATAL(LOFAR::formatString (
        "Unable to delete DP: '%s'", 
        dpName.c_str()));
  }
  else if (!dpeExists(dpName))
  {
    LOG_WARN(LOFAR::formatString (
        "DP '%s' does not exists", 
        dpName.c_str()));    
    result = SA_PROP_DOES_NOT_EXIST;
  }
  else if ((result = getDpId(dpName, dpId)) != SA_NO_ERROR)
  {
    LOG_ERROR(LOFAR::formatString (
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

    LOG_ERROR(LOFAR::formatString (
        "PVSS: Unable to delete DP: '%s'", 
        dpName.c_str()));

    result = SA_DELETEPROP_FAILED;
  }
  else
  {
    LOG_DEBUG(LOFAR::formatString (
        "Deletion of DP '%s' was requested successful", 
        dpName.c_str()));
  }

  return result;
}

TSAResult GSAService::dpeSubscribe(const string& propName)
{
  TSAResult result(SA_NO_ERROR);
  
  DpIdentifier dpId;
  string pvssDpName;

  convPropToDpConfig(propName, pvssDpName, true);

  LOG_INFO(LOFAR::formatString (
      "Subscribe on property '%s'", 
      propName.c_str()));
  
  if ((result = GSASCADAHandler::instance()->isOperational()) != SA_NO_ERROR)
  {
    LOG_FATAL(LOFAR::formatString (
        "Unable to subscribe on property: '%s'", 
        propName.c_str()));
  }
  else if (!dpeExists(propName))
  {
    LOG_WARN(LOFAR::formatString (
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
  
      LOG_ERROR(LOFAR::formatString (
          "PVSS: Unable to subscribe on property: '%s'", 
          propName.c_str()));
  
      result = SA_SUBSCRIBEPROP_FAILED;
    }
    else
    {
      LOG_DEBUG(LOFAR::formatString (
          "Subscription on property '%s' was requested successful", 
          propName.c_str()));
    }
  }
  else
  {
    
    LOG_ERROR(LOFAR::formatString (
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

  LOG_INFO(LOFAR::formatString (
      "Unsubscribe from property '%s'", 
      propName.c_str()));
  
  if ((result = GSASCADAHandler::instance()->isOperational()) != SA_NO_ERROR)
  {
    LOG_FATAL(LOFAR::formatString (
        "Unable to unsubscribe from property: '%s'", 
        propName.c_str()));
  }
  else if (!dpeExists(propName))
  {
    LOG_WARN(LOFAR::formatString (
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

      LOG_ERROR(LOFAR::formatString (
          "PVSS: Unable to unsubscribe from property: '%s'", 
          propName.c_str()));

      result = SA_UNSUBSCRIBEPROP_FAILED;
    }
    else
    {
      LOG_DEBUG(LOFAR::formatString (
          "Unsubscription from property '%s' was requested successful", 
          propName.c_str()));
    }
  }
  else
  {
    LOG_ERROR(LOFAR::formatString (
        "Unable to unsubscribe from property: '%s'", 
        propName.c_str()));
  }
  
  return result;
}

TSAResult GSAService::dpeGet(const string& propName)
{
  TSAResult result(SA_NO_ERROR);
  DpIdentifier dpId;
  string pvssDpName;

  convPropToDpConfig(propName, pvssDpName, true);

  LOG_INFO(LOFAR::formatString (
      "Request value of property '%s'", 
      propName.c_str()));
  
  if ((result = GSASCADAHandler::instance()->isOperational()) != SA_NO_ERROR)
  {
    LOG_FATAL(LOFAR::formatString (
        "Unable to request of property: '%s'", 
        propName.c_str()));
  }
  else if (!dpeExists(propName))
  {
    LOG_WARN(LOFAR::formatString (
        "Property: '%s' does not exists", 
        propName.c_str()));    
    result = SA_PROP_DOES_NOT_EXIST;      
  }
  else if ((result = getDpId(pvssDpName, dpId)) != SA_NO_ERROR)
  {
    LOG_ERROR(LOFAR::formatString (
        "Unable to request value of property: '%s'", 
        propName.c_str()));
  }
  else if (Manager::dpGet(dpId, _pWFA, PVSS_FALSE) == PVSS_FALSE)
  {
    ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
                  ErrClass::ERR_PARAM,        // wrong name: blame others
                  ErrClass::UNEXPECTEDSTATE,  // fits all
                  "GSAService",               // our file name
                  "dpeGet",                      // our function name
                  CharString("Value of datapoint ") + propName.c_str() + 
                  CharString(" could not be requested"));

    LOG_ERROR(LOFAR::formatString (
        "PVSS: Unable to request value of property: '%s'", 
        propName.c_str()));

    result = SA_GETPROP_FAILED;
  }
  else
  {
    LOG_DEBUG(LOFAR::formatString (
        "Value of property '%s' was requested successful", 
        propName.c_str()));
  }
  
  return result;
}

TSAResult GSAService::dpeSet(const string& propName, const GCFPValue& value)
{
  TSAResult result(SA_NO_ERROR);
  DpIdentifier dpId;
  Variable* pVar(0);
  string pvssDpName;

  convPropToDpConfig(propName, pvssDpName, false);

  LOG_INFO(LOFAR::formatString (
      "Set value of property '%s'", 
      propName.c_str()));
  
  if ((result = GSASCADAHandler::instance()->isOperational()) != SA_NO_ERROR)
  {
    LOG_FATAL(LOFAR::formatString (
        "Unable to set value of property: '%s'", 
        propName.c_str()));
  }
  else if (!dpeExists(propName))
  {
    LOG_WARN(LOFAR::formatString (
        "Property: '%s' does not exists", 
        propName.c_str()));    
    result = SA_PROP_DOES_NOT_EXIST;      
  }
  else if ((result = getDpId(pvssDpName, dpId)) != SA_NO_ERROR)
  {
    LOG_ERROR(LOFAR::formatString (
        "Unable to set value of property: '%s'", 
        propName.c_str()));
  }
  else if ((result = convertMACToPVSS(value, &pVar, dpId)) != SA_NO_ERROR)
  {
    LOG_ERROR(LOFAR::formatString (
        "Unable to set value of property: '%s'", 
        propName.c_str()));
  }
  else if (Manager::dpSet(dpId, *pVar) == PVSS_FALSE)
  {
    ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
                  ErrClass::ERR_PARAM,        // wrong name: blame others
                  ErrClass::UNEXPECTEDSTATE,  // fits all
                  "GSAService",               // our file name
                  "dpeSet",                      // our function name
                  CharString("Value of datapoint ") + propName.c_str() + 
                  CharString(" could not be set"));

    LOG_ERROR(LOFAR::formatString (
        "PVSS: Unable to set value of property: '%s'", 
        propName.c_str()));

    result = SA_SETPROP_FAILED;
  }
  else
  {
    LOG_DEBUG(LOFAR::formatString (
        "Property value '%s' is set successful", 
        propName.c_str()));
  }
  if (pVar)
  {
    delete pVar; // constructed by convertMACToPVSS method
  }
  return result;
}

bool GSAService::dpeExists(const string& dpeName)
{
  DpIdentifier dpId;
  if (Manager::getId(dpeName.c_str(), dpId) == PVSS_FALSE)
    return false;
  else
    return true;
}

bool GSAService::typeExists (const string& dpTypeName)
{
  CharString pvssTypeName(dpTypeName.c_str());
  DpTypeId dpTypeId; 
  return (Manager::getTypeId(pvssTypeName, dpTypeId) == PVSS_TRUE);
}

TSAResult GSAService::convertPVSSToMAC(const Variable& variable, 
                                  const DpIdentifier& dpId, 
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
    case TEXT_VAR: *pMacValue = new GCFPVString(((TextVar *)&variable)->getValue()); break;
    //REF!!! case TEXT_VAR: *pMacValue = new GCFPVRef(((TextVar *)&variable)->getValue());
    //case BIT32_VAR: *pMacValue = new GCFPVBit32(((Bit32Var *)&variable)->getValue()); break;
    //case BLOB_VAR: *pMacValue = new GCFPVBlob(((BlobVar *)&variable)->getValue()); break;
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
        case DYNCHAR_VAR: type = LPT_DYNBOOL; break;
        case DYNUINTEGER_VAR: type = LPT_DYNBOOL; break;
        case DYNINTEGER_VAR: type = LPT_DYNBOOL; break;
        case DYNFLOAT_VAR: type = LPT_DYNBOOL; break;
        case DYNTEXT_VAR: type = LPT_DYNBOOL; break;
        default: break;        
      }
        
      const DynVar* pDynVar = static_cast<const DynVar*>(&variable);
      if (pDynVar)
      {
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
            default:
              break;
          }
          arrayTo.push_back(pItemValue);
        }
        *pMacValue = new GCFPVDynArr(type, arrayTo);
      }
      break;
    }
    default:
      result = SA_DPTYPE_UNKNOWN;
      LOG_ERROR(LOFAR::formatString (
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
  DpElementType elTypeId;
  *pVar = 0;
  if (Manager::getTypeContainerPtr()->getElementType(dpId, elTypeId) != DpTypeContOK)
  {
    LOG_FATAL("PVSS: Could not get dpeType" );   
    return SA_DPTYPE_UNKNOWN;
  }
  switch (macValue.getType())
  {
    case LPT_BOOL:
      if (elTypeId == DPELEMENT_BIT) *pVar = new BitVar(((GCFPVBool*)&macValue)->getValue());
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
/*    case LPT_REF:
      if (elTypeId == DPELEMENT_TEXT) *pVar = new TextVar(((GCFPVRef*)&macValue)->getValue());
      break;
    case LPT_BLOB:
      if (elTypeId == DPELEMENT_BLOB) *pVar = new BlobVar(((GCFPVBlob*)&macValue)->getValue());
      break;
    case LPT_DATETIME:
      if (elTypeId == DPELEMENT_TIME) *pVar = new TimeVar(((GCFPVDateTime*)&macValue)->getValue());
      break;
    case LPT_BIT32:
      if (elTypeId == DPELEMENT_BIT32) *pVar = new Bit32Var(((GCFPVBit32 *)&macValue)->getValue());
      break;*/
    default:
      if (macValue.getType() > LPT_DYNARR && 
          macValue.getType() <= LPT_DYNSTRING)
      {        
        Variable* pItemValue;
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
            default:
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
  if (result == SA_NO_ERROR && *pVar == 0)
  {
    CharString valueTypeName;
    getPVSSType(macValue.getType(), valueTypeName);
    LOG_ERROR(LOFAR::formatString (
        "Type mismatch! Property type: %s != Value type: %d (see in DpElemnent.hxx)",         
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

    LOG_DEBUG(LOFAR::formatString (
            "PVSS: Datapoint '%s' missing", 
            dpName.c_str()));   

    result = SA_PROPNAME_MISSING;
  }

  return result;
}

void GSAService::convPropToDpConfig(const string& propName, string& pvssDpName, bool willReadValue)
{
  pvssDpName = propName.c_str();
  if (propName.find('.') < propName.size())
  {
    if (willReadValue) 
      pvssDpName += ":_online.._value";
    else
      pvssDpName += ":_original.._value";
  }
  else
  {
    if (willReadValue) 
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
