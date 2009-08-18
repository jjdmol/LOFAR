/***************************************************************************
                          GPI_Controller.cx  -  description
                             -------------------
    begin                : Fri Jul 11 2003
    copyright            : (C) 2003 by pvss
    email                : pvss@sun.solarsystem
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define DEBUG_SIGNAL
#define DECLARE_SIGNAL_NAMES
#include "GPI_Controller.hxx"
#include "GPI_Resources.hxx"
#include <GPI/GCF_PortManager.hxx>
#include <PVSSAPI/GCF_PVSSAPI.hxx>
#include <PROP/FProperty.h>
#include <PROP/FPValue.h>
#include <PROP/FPByteValue.h>
#include <PROP/FPWordValue.h>
#include <PROP/FPDWordValue.h>
#include <PROP/FPFloatValue.h>
#include <PROP/FPDoubleValue.h>
#include <PROP/FPBoolValue.h>
#include <PROP/FPLongLongValue.h>
#include <BitVar.hxx>
#include <CharVar.hxx>
#include <UIntegerVar.hxx>
#include <IntegerVar.hxx>
#include <FloatVar.hxx>


#include "F_Supervisory_Protocol.ph"

#include <signal.h>
#include <algorithm>

const char* TYPE_TO_STRING[] =
{
  "",
  "BOOL_VAL", // BOOL_VAL
  "UCHAR_VAL", //UCHAR_VAL
  "CHAR_VAL", //CHAR_VAL
  "USHORT_VAL", //USHORT_VAL
  "SHORT_VAL", //SHORT_VAL
  "ULONG_VAL", //ULONG_VAL
  "LONG_VAL", //LONG_VAL
  "ULONGLONG_VAL", //ULONGLONG_VAL
  "LONGLONG_VAL", //LONGLONG_VAL
  "DOUBLE_VAL", //DOUBLE_VAL
  "COMPLEX_DOUBLE_VAL", //COMPLEX_DOUBLE_VAL
  "FLOAT_VAL", //FLOAT_VAL
  "COMPLEX_FLOAT_VAL" //COMPLEX_FLOAT_VAL
};

PVSSboolean GPIController::doExit = PVSS_FALSE;

GPIController::GPIController() :
  GCFTask((State)&GPIController::initial_state, "PIContr"),
  _curSeqNr(0)
{
  _pPortManager = GCFPortManager::getInstance();
  registerProtocol(F_SUPERVISORY_PROTOCOL, F_SUPERVISORY_PROTOCOL_signalnames, 0);
}

GPIController::~GPIController()
{
  delete _pPortManager; // should be done only here
  for( CCLI iter = _sequenceQueue.begin(); iter != _sequenceQueue.end(); iter++ )
  {
    if( iter->seqArg )
    {
      delete iter->seqArg;
    }
  }
  _sequenceQueue.clear();
}

void GPIController::init()
{
  cerr << "GPIManager is started!" << endl;
  _pPortManager->init(this, GPIResources::getServerPort());
}

void GPIController::workProc()
{
  if (GPIController::doExit != PVSS_TRUE)
  {
    _pPortManager->workProc();
  }
  else
  {
    stop();
  }
}

int GPIController::initial_state(GCFEvent& e, GCFPortInterface& p)
{
  int status = GCFEvent::HANDLED;

  F_DEBUG_SIGNAL_I(e, p, "State: GPIController::initial_state");

  switch (e.signal)
  {
  case F_INIT_SIG:
    TRAN(&GPIController::operational_state);
    break;

  case F_ENTRY_SIG:
    break;

  case F_CONNECTED_SIG:
    break;

  case F_DISCONNECTED_SIG:
    break;

  case F_TIMER_SIG:
    break;

  default:
    status = GCFEvent::NOT_HANDLED;
    break;
  }

  return status;
}

int GPIController::operational_state(GCFEvent& e, GCFPortInterface& p)
{
  int status = GCFEvent::HANDLED;
  static DpIdentifier cmdDpID;
  static DpIdentifier tempDpID;
  static GCFPvssApi *pPvssApi = (GCFPvssApi *)_pScadaApi;

  F_DEBUG_SIGNAL_I(e, p, "State: GPIController::operational_state");

  switch (e.signal)
  {
  case F_ENTRY_SIG:
  {
    pPvssApi->connectToDp("CMD.:_online.._value", cmdDpID);
    pPvssApi->connectToDp("BRD1_PING_maxSeqNr.:_online.._value", tempDpID);
    break;
  }
  case F_VCHANGEMSG_SIG:
  {
    GCFVChangeMsgEvent* pEvent = static_cast<GCFVChangeMsgEvent*>(&e);
    DpVCItem* item = pEvent->_pMsg;
    Variable *varPtr = item->getValuePtr();
    if (varPtr)      // could be NULL !!
    {
      if (item->getDpIdentifier() == cmdDpID)
      {
        CharString cmd(((TextVar *)varPtr)->getValue());
        if (Resources::isDbgFlag(Resources::DBG_API_USR1))
          cerr << cmd << endl;
        if (cmd.ncmp("subs", 4) == 0)
        {
          subscribe(((char*)cmd) + 5, true);
        }
        else if (cmd.ncmp("unsubs", 6) == 0)
        {
          subscribe(((char*)cmd) + 7, false);
        }
      }
      else
        setValue(item);
    }
    break;                             

  }  
  case F_SV_SUBSCRIBED:
  {
    SubscribedEvent *pEvent = static_cast<SubscribedEvent*>(&e);
    switch (pEvent->result)
    {
      case NO_ERROR:
      {
        char *seqArg = getSequenceArg(pEvent->seqNr);
        CharString type(TYPE_TO_STRING[pEvent->type]);
        CharString propName(seqArg);
        if (pPvssApi->getDpId(propName + ".:_online.._value", tempDpID) == PVSS_FALSE) 
          pPvssApi->createDp(type, propName);
        deregisterSequence(pEvent->seqNr);
        break;
      }
      case UNSUBSCRIBED:
        deregisterSequence(pEvent->seqNr);
        break;
      default:
        cerr << "Subscribe error: " << pEvent->result << endl;
        break;
    }
    break;
  }
  case F_SV_VALUESET:
  {
    ValuesetEvent *pEvent = static_cast<ValuesetEvent*>(&e);
    switch (pEvent->result)
    {
      case NO_ERROR:
      {
       deregisterSequence(pEvent->seqNr);
       cerr << "Value was set succesfully" << endl;
      }
    }
    break;
  }
  case F_SV_VALUECHANGED:
  {
    valueChanged(e, p);
    break;                                          
  }
  case F_DPCONNECTED_SIG:
    break;
    
  case F_DPCREATED_SIG:
    break;

  case F_CONNECTED_SIG:
    break;

  case F_DISCONNECTED_SIG:
    break;

  case F_TIMER_SIG:
    break;

  default:
    status = GCFEvent::NOT_HANDLED;
    break;
  }

  return status;                                 

}

void GPIController::signalHandler(int sig)
{
  if ( (sig == SIGINT) || (sig == SIGTERM) )
	  GPIController::doExit = PVSS_TRUE;
}                                            
                                             
void GPIController::subscribe(char* arg, bool onOff)
{
  static char buffer[256];
  SubscribeEvent e;
  char* clientName;
  e.onOff = onOff;
  e.seqNr = registerSequence(arg);
  clientName = strtok(arg, "_");
  if (clientName)
  {
    GCFPort *pPort(0);
    sprintf(buffer, "SV%s", clientName);
    pPort = _pPortManager->getPort(buffer);
    if (pPort)
    {
      char* propName;
      propName = strtok(NULL, " ");
      if (propName)
      {
        unsigned short bufLength;
        sprintf(buffer, "%c%s", strlen(propName), propName);
        bufLength = strlen(buffer);
        e.length += bufLength;
        pPort->send(e, buffer, bufLength);
      }
    }
  }
}

unsigned short GPIController::registerSequence(const char *arg)
{
  static unsigned short curSeqNr = 0;
  TSequenceEntry newEntry;
  newEntry.seqNr = ++curSeqNr;
  newEntry.seqArg = strdup(arg); // will be freed by deregisterSequence method

  _sequenceQueue.insert(newEntry);
  return curSeqNr;
}                                                


char* GPIController::getSequenceArg(unsigned short seqNr)
{
  TSequenceEntry entry;
  entry.seqNr = seqNr;
  CCLI iter = _sequenceQueue.find(entry);

  if ( iter != _sequenceQueue.end())
  {
    return iter->seqArg;
  }
  return 0;
}

void GPIController::deregisterSequence(unsigned short seqNr)
{
  TSequenceEntry entry;
  entry.seqNr = seqNr;

  CCLI iter = _sequenceQueue.find(entry);
  if ( iter != _sequenceQueue.end())
  {
    delete iter->seqArg;
    _sequenceQueue.erase(iter);
  }
}

void GPIController::valueChanged(GCFEvent& e, GCFPortInterface& p)
{
  DpIdentifier dpID;  
  GCFPvssApi *pPvssApi = (GCFPvssApi *)_pScadaApi;
  char *buffer = ((char*)&e) + sizeof(GCFEvent);
  unsigned char propNameLength(0);
  unsigned char taskNameLength(0);
  unsigned char bytesRead(0);
  static char totalPropName[512];
  static char propName[256];
  static char taskName[256];
  CharString tempTotPropName;

  memcpy((void *) &propNameLength, buffer, sizeof(unsigned char));
  buffer += sizeof(unsigned char);
  memcpy(propName, buffer, propNameLength);
  propName[propNameLength] = 0;
  buffer += propNameLength;
  FProperty newProperty(propName, (FPValue::ValueType) buffer[0]);
  bytesRead = newProperty.unpack(buffer);
  newProperty.syncValues();
  buffer += bytesRead;

  memcpy((void *) &taskNameLength, buffer, sizeof(unsigned char));
  buffer += sizeof(unsigned char);
  memcpy(taskName, buffer, taskNameLength);
  taskName[taskNameLength] = 0;

  sprintf(totalPropName, "%s_%s_%s.:_original.._value", (p.getName() + 2), taskName, propName);
  tempTotPropName = totalPropName;
  if (pPvssApi->getDpId(tempTotPropName, dpID) == PVSS_FALSE) return;

  Variable *pVar(0);
  
  const FPValue *pVal = newProperty.getValue();
  
  switch (pVal->getType())
  {
    case FPValue::BOOL_VAL:
      pVar = new BitVar(((FPBoolValue *)pVal)->getValue());
      break;
    case FPValue::UCHAR_VAL: 
      pVar = new CharVar(((FPUCharValue *)pVal)->getValue());
      break;
    case FPValue::CHAR_VAL:
      pVar = new CharVar(((FPCharValue *)pVal)->getValue());
      break;
    case FPValue::USHORT_VAL:
      pVar = new UIntegerVar(((FPUShortValue *)pVal)->getValue());
      break;
    case FPValue::SHORT_VAL:
      pVar = new IntegerVar(((FPShortValue *)pVal)->getValue());
      break;
    case FPValue::ULONG_VAL:
      pVar = new UIntegerVar(((FPULongValue *)pVal)->getValue());
      break;
    case FPValue::LONG_VAL:
      pVar = new IntegerVar(((FPLongValue *)pVal)->getValue());
      break;
    case FPValue::ULONGLONG_VAL:
      pVar = new UIntegerVar(((FPULongLongValue *)pVal)->getValue());
      break;
    case FPValue::LONGLONG_VAL:
      pVar = new IntegerVar(((FPLongLongValue *)pVal)->getValue());
      break;
    case FPValue::DOUBLE_VAL:
      pVar = new FloatVar(((FPFloatValue *)pVal)->getValue());
      break;
    case FPValue::FLOAT_VAL:
      pVar = new FloatVar(((FPFloatValue *)pVal)->getValue());
      break;
    case FPValue::COMPLEX_DOUBLE_VAL:
    case FPValue::COMPLEX_FLOAT_VAL:
      cerr << "Error not support this type at this moment" << endl;
      break;
  }
  if (Manager::dpSet(dpID,*pVar) == PVSS_FALSE)
    cerr << "Set value error: " << pVar->formatValue(CharString()) << endl;
  if (pVar)
    delete pVar;
}

void GPIController::setValue(DpVCItem* pVCItem)
{
  static char buffer[256];
  CharString typeName, propName;
  DpIdentifier tempDpId;
  SetvalueEvent e;
  char* sysName(0), *clientName(0), *totPropName(0);
  Variable *pVar = pVCItem->getValuePtr();

  Manager::getTypeName(pVCItem->getDpIdentifier().getDpType(), typeName);
  pVCItem->getDpIdentifier().convertToString(propName);

  totPropName = propName.cutCharPtr();
  sysName = strtok(totPropName, ":"); // ignore systemname for now
  clientName = strtok(NULL, "_");

  if (clientName && pVar)
  {
    GCFPort *pPort(0);
    sprintf(buffer, "SV%s", clientName);
    pPort = _pPortManager->getPort(buffer);
    if (pPort)
    {
      char* propName;
      // we don't have any PVSS struct's for now, so we can ignore the '.'
      propName = strtok(NULL, ".:"); 
      if (propName)
      {        
        unsigned short bufLength;
        FPLongValue* pVal(0);
        FProperty* pSetProperty(0);

        if (typeName == "LONG_VAL")
        {
          pVal = new FPLongValue(((IntegerVar *)pVar)->getValue());
          pSetProperty = new FProperty(propName, FPValue::LONG_VAL);
        }
        pSetProperty->setValue(*pVal);
        bufLength = pSetProperty->pack(buffer);
        e.length += bufLength;
        pPort->send(e, buffer, bufLength);
        delete pVal;
        delete pSetProperty;
      }
    }
  }  
  propName.setCharPtr(totPropName);
}

