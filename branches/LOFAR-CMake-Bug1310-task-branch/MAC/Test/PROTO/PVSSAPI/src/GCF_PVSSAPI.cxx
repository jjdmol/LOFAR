#include <GCF_PVSSAPI.hxx>
#include <Resources.hxx>
#include <HotLinkWaitForAnswer.hxx>   
#include <StartDpInitSysMsg.hxx>      
#include <DpMsgAnswer.hxx>            
#include <DpMsgHotLink.hxx>           
#include <DpHLGroup.hxx>              
#include <DpVCItem.hxx>               
#include <ErrHdl.hxx>                 
#include <ErrClass.hxx>               
#include <signal.h>

GCFDummyPort GCFPvssApi::_pvssPort("PVSS");

GCFHotlinkWaitForAnswer::GCFHotlinkWaitForAnswer(GCFPvssApi *manager) :
  HotLinkWaitForAnswer(),
  _manager(manager)
{
}

void GCFHotlinkWaitForAnswer::hotLinkCallBack(DpMsgAnswer &answer)
{
  _manager->handleHotLink(answer);  
}


void GCFHotlinkWaitForAnswer::hotLinkCallBack(DpHLGroup &group)
{
  _manager->handleHotLink(group);
}



// -------------------------------------------------------------------------
// Our manager class

// The constructor defines Manager type (API_MAN) and Manager number
GCFPvssApi::GCFPvssApi(GCFTask *task) 
           : Manager(ManagerIdentifier(API_MAN, Resources::getManNum())),
           GCFScadaApi(task),          
           _wait(this)
{
}

// Run our ApiTest manager
void GCFPvssApi::init()
{
	long sec, usec;

  // First connect to Data manager.
  // We want Typecontainer and Identification so we can resolve names
  // This call succeeds or the manager will exit
	connectToData(StartDpInitSysMsg::TYPE_CONTAINER | StartDpInitSysMsg::DP_IDENTIFICATION);

  // While we are in STATE_INIT  we are initialized by the Data manager
  while (getManagerState() == STATE_INIT)
  {
    // Wait max. 1 second in select to receive next message from data.
    // It won't take that long...
    sec = 1;
    usec = 0;
    dispatch(sec, usec);
  }

  // We are now in STATE_ADJUST and can connect to Event manager
  // This call will succeed or the manager will exit
  connectToEvent();

  // We are now in STATE_RUNNING. This is a good time to connect 
  // to our Datapoints
}

PVSSboolean GCFPvssApi::connectToDp(const CharString &dpName, DpIdentifier &dpId)
{
  PVSSboolean result(PVSS_FALSE);

  // Ask the Identification for the DpId of our Datapoints
  if (getDpId(dpName, dpId) == PVSS_TRUE)
  {
    // do the Connect
    if (Manager::dpConnect(dpId, &_wait, PVSS_FALSE) == PVSS_FALSE)
    {
      if (Resources::isDbgFlag(Resources::DBG_API_USR2))
        cerr << "Cannot connect to " << dpName << endl;      
    }
    else
      result = PVSS_TRUE;
  }

  return result;
}

PVSSboolean GCFPvssApi::getDpId(const CharString & dpName, DpIdentifier &dpId)
{
  PVSSboolean result(PVSS_TRUE);

  // Ask the Identification for the DpId of our Datapoints
  if (Manager::getId(dpName, dpId) == PVSS_FALSE)
  {
    // This name was unknown.
    // The parameters are in Bascis/ErrClass.hxx
    ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
                  ErrClass::ERR_PARAM,        // wrong name: blame others
                  ErrClass::UNEXPECTEDSTATE,  // fits all
                  "GCFPvssApi",              // our file name
                  "run",                      // our function name
                  CharString("Datapoint ") + dpName + CharString(" missing"));

    cerr << "Datapoint" << dpName << " missing!!!" << endl;

    result = PVSS_FALSE;
  }

  return result;
}


// Receive Signals.
// We are interested in SIGINT and SIGTERM. 
void GCFPvssApi::handleHotLink(const DpMsgAnswer &answer)
{
  if (Resources::isDbgFlag(Resources::DBG_API_USR2))
    cerr << "Is answer on: " << answer.isAnswerOn() << endl;
  switch (answer.isAnswerOn())
  {
    case DP_MSG_CONNECT:
      for (AnswerGroup *pGrItem = answer.getFirstGroup();
           pGrItem; pGrItem = answer.getNextGroup())
      {
        for (AnswerItem *pAnItem = pGrItem->getFirstItem(); pAnItem;
             pAnItem = pGrItem->getNextItem())
        {
          GCFAnswerEvent msg(F_DPCONNECTED_SIG);
          if (Resources::isDbgFlag(Resources::DBG_API_USR2))
          {
            CharString dpName;
            pAnItem->getDpIdentifier().convertToString(dpName);
            cerr << "Connected: " << dpName << endl;
          }

          msg._pMsg = pAnItem;
          _pFsm->dispatch(msg, _pvssPort);
        }
      }
      break;      

    case DP_MSG_REQ_NEW_DP:
      for (AnswerGroup *pGrItem = answer.getFirstGroup();
           pGrItem; pGrItem = answer.getNextGroup())
      {
        for (AnswerItem *pAnItem = pGrItem->getFirstItem(); pAnItem;
             pAnItem = pGrItem->getNextItem())
        {
          GCFAnswerEvent msg(F_DPCREATED_SIG);
          if (Resources::isDbgFlag(Resources::DBG_API_USR2))
          {
            CharString dpName;
            pAnItem->getDpIdentifier().convertToString(dpName);
            cerr << "Created: " << dpName << endl;
          }
          msg._pMsg = pAnItem;
          _pFsm->dispatch(msg, _pvssPort);
        }
      }
      break;
  }
}

// Handle incoming hotlinks.
// This function is called from our hotlink object
void GCFPvssApi::handleHotLink(const DpHLGroup &group)
{
  // Print Debug information
  if (Resources::isDbgFlag(Resources::DBG_API_USR1))
	  cerr << "Receiving HotLink" << endl;

  // A group consists of pairs of DpIdentifier and values called items.
  // There is exactly one item for all configs we are connected.

  for (DpVCItem *item = group.getFirstItem(); item;
       item = group.getNextItem())
	{
    GCFVChangeMsgEvent msg;
    msg._pMsg = item;
    _pFsm->dispatch(msg, _pvssPort);
  }
}

PVSSboolean GCFPvssApi::createDp(const CharString &typeName, const CharString &dpName, DpIdentifier *pDpId)
{
  DpTypeId dpTypeId;
  LangText dpNameLang(dpName);
  PVSSboolean result(PVSS_FALSE);

    // This is more complex than it looks like, due to async. datapoint creation
  if (getTypeId(typeName, dpTypeId) == PVSS_FALSE)
  {
    // This name was unknown.
    // The parameters are in Bascis/ErrClass.hxx
    ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
                  ErrClass::ERR_PARAM,        // wrong name: blame others
                  ErrClass::UNEXPECTEDSTATE,  // fits all
                  "GCFPvssApi",              // our file name
                  "createDp",                      // our function name
                  CharString("DatapointType ") + typeName + CharString(" missing"));

    if (Resources::isDbgFlag(Resources::DBG_API_USR2))
      cerr << "DatapointType " + typeName << " missing!!!" << endl;

  }
  else if (dpCreate(dpNameLang, dpTypeId, &_wait) == PVSS_FALSE)
  {
    // This name was unknown.
    // The parameters are in Bascis/ErrClass.hxx
    ErrHdl::error(ErrClass::PRIO_SEVERE,      // It is a severe error
                  ErrClass::ERR_PARAM,        // wrong name: blame others
                  ErrClass::UNEXPECTEDSTATE,  // fits all
                  "GCFPvssApi",              // our file name
                  "createDp",                      // our function name
                  CharString("Datapoint ") + dpName + CharString(" could not be created"));

    if (Resources::isDbgFlag(Resources::DBG_API_USR2))
      cerr << "Datapoint " << dpName << " could not be created!!!" << endl;    
  }
  else
  {
    if (pDpId != NULL)
    {
      CharString dpNameForGet;
      dpNameForGet.format("%s.:_original.._value", (const char *)dpName);
      result = getDpId(dpNameForGet, *pDpId);
    }
    else
      result = PVSS_TRUE;
      
    if (Resources::getSaveLastValueBit() == 1)
    {
      CharString userBitDp;
      DpIdentifier dpId;
      BitVar bitVar(PVSS_TRUE);
      
      userBitDp.format("%s.:_original.._userbit1", (const char *)dpName);
      getDpId(userBitDp, dpId);
      dpSet(dpId, bitVar);
    }
    
    if (result == PVSS_TRUE && Resources::isDbgFlag(Resources::DBG_API_USR2))
      cerr << "Datapoint " << dpName << " is created successfuly!!!" << endl;
  }
  return result;
}

void GCFPvssApi::workProc()
{
  if (getManagerState() == STATE_RUNNING)
  {
   	long sec(0), usec(0);        // No wait

    sec = 0;
    dispatch(sec, usec);
  }
  else
  {
    init();
  }
}

