//#  PIClient.cc: 
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

#include <PIClient.h>
#include <GCF/PALlight/CEPPropertySet.h>
#include <GCF/Utils.h>
#include <Common/ParameterSet.h>
#include <Transport/TH_Socket.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Transport/CSConnection.h>
using LOFAR::BlobIStream;
using LOFAR::BlobOStream;

namespace LOFAR 
{
 namespace GCF 
 {
using namespace Common;
  namespace CEPPMLlight 
  {
    

PIClient* PIClient::_pInstance = 0;

bool operator == (const PIClient::TAction& a, const PIClient::TAction& b)
{
  return ((a.pPropSet == b.pPropSet) &&
          (a.eventID == b.eventID) &&
          (a.extraData == b.extraData));
}

void logResult(TPIResult result, CEPPropertySet& propSet);

PIClient::PIClient() : 
  _pReadConn(0),
  _pWriteConn(0),
  _usecount(0),
  _valueBuf(0),
  _upperboundValueBuf(0),
  _extraDataBuf(1024, 0)
{
  _thread = Thread::create(piClientThread, this);
}

PIClient::~PIClient () 
{
  _thread.cancel();
  if (_valueBuf) delete [] _valueBuf;
}

PIClient* PIClient::instance(bool temporary)
{
  if (0 == _pInstance)
  {    
    _pInstance = new PIClient();
    ASSERT(_pInstance->mayDeleted());
  }
  if (!temporary) _pInstance->use();
  return _pInstance;
}

void PIClient::release()
{
  ASSERT(_pInstance);
  ASSERT(!_pInstance->mayDeleted());
  _pInstance->leave(); 
  if (_pInstance->mayDeleted())
  {
    delete _pInstance;
    _pInstance = 0;
  }
}

void* PIClient::piClientThread(void* arg)
{
  // The piClient thread
  PIClient* pClient = (PIClient*) arg;
  
  pClient->run();
  
  return 0;
}

void PIClient::run()
{
  // will be called in piClient thread context
  bool retry;

  do
  {    
    retry = false;
    try
    {
      LOG_DEBUG("Setup connection");
      globalParameterSet()->adoptFile("PropertyInterface.conf");

      _dhPIClient.init();
      TH_Socket*  pTH = new TH_Socket(globalParameterSet()->getString(PARAM_PI_HOST), 
                                      globalParameterSet()->getString(PARAM_PI_PORT), 
                                      false);
      LOG_DEBUG("Try to connect");
      pTH->init();

      LOG_DEBUG("Connected to PropertyInterface of MAC");
    
      _pReadConn  = new CSConnection("read",  0, &_dhPIClient, pTH, false);
      _pWriteConn = new CSConnection("write", &_dhPIClient, 0, pTH, false);
      ASSERTSTR(_pReadConn, "Unable to allocate connection for reading");
      ASSERTSTR(_pWriteConn, "Unable to allocate connection for writing");
    }
    catch (std::exception& e)
    {
      LOG_ERROR(formatString (
          "Exception: %s! RETRY after 10s!!!", 
          e.what()));
      retry = true;
      sleep(10);
    }
  } while (retry);

  bool connected(false);  
  do
  {    
    retry = false;
    try
    {
      // the main loop of the thread
      while (true)
      {
        processOutstandingActions();
        
        if (_pReadConn->read() == CSConnection::Finished)
        {
          switch (_dhPIClient.getEventID())
          {
            case DH_PIProtocol::SCOPE_REGISTERED:
              scopeRegistered();
              break; 
            case DH_PIProtocol::SCOPE_UNREGISTERED:
              scopeUnregistered();
              break;
            case DH_PIProtocol::LINK_PROPSET:
              linkPropSet();
              break;
            case DH_PIProtocol::UNLINK_PROPSET:
              unlinkPropSet();
              break;
            default:
              ASSERT(0);
              break;
          }
        }
        usleep(10000);
      }
    } 
    catch (std::exception& e)
    {
      LOG_ERROR(formatString (
          "Exception: %s! RETRY after 10s", 
          e.what()));
      retry = true;
      // TODO: find out this is still necessary
      if (connected)
      {
        _bufferMutex.lock();
        for (TBufferedActions::iterator iter = _bufferedActions.begin();
             iter != _bufferedActions.end(); ++iter)
        {
          delete [] iter->extraData;
        }
        for (TBufferedActions::iterator iter = _bufferedValues.begin();
             iter != _bufferedValues.end(); ++iter)
        {
          delete [] iter->extraData;
        }
        _bufferedActions.clear();
        _bufferedValues.clear();
        _bufferMutex.unlock();
        
        _propSetMutex.lock();  
        TMyPropertySets tempMyPropertySets(_myPropertySets);
        _startedSequences.clear();
        _propSetMutex.unlock();
  
        for (TMyPropertySets::iterator iter = tempMyPropertySets.begin();
             iter != tempMyPropertySets.end(); ++iter)
        {
          iter->second->connectionLost();
        }
        connected = false;  
        sleep(10);
      }
    }
  }
  while (retry);
}

void PIClient::deletePropSet(const CEPPropertySet& propSet)
{
  // will be called from user thread context
  _bufferMutex.lock();
  // cleanup all sequences and buffered actions, which are related to the deleted
  // property set
  list<uint16> seqToDelete;
  for (TStartedSequences::iterator iter = _startedSequences.begin();
       iter != _startedSequences.end(); ++iter)
  {
    if (iter->second == &propSet)
    {
      iter->second = 0;
    }
  }
  for (TBufferedActions::iterator iter = _bufferedActions.begin();
       iter != _bufferedActions.end(); ++iter)
  {
    if (iter->pPropSet == &propSet)
    {
      delete [] iter->extraData;
      iter = _bufferedActions.erase(iter);
      iter--;
    }    
  }
  for (TBufferedActions::iterator iter = _bufferedValues.begin();
       iter != _bufferedValues.end(); ++iter)
  {
    if (iter->pPropSet == &propSet)
    {
      delete [] iter->extraData;
      iter = _bufferedValues.erase(iter);
      iter--;
    }    
  }
  _bufferMutex.unlock();
  _propSetMutex.lock();

  _myPropertySets.erase(propSet.getScope());

  _propSetMutex.unlock();
}

bool PIClient::registerScope(CEPPropertySet& propSet)
{
  // will be called from user thread context
  bool succeed(true);

  _propSetMutex.lock();

  TMyPropertySets::iterator iter = _myPropertySets.find(propSet.getScope());
  if (iter != _myPropertySets.end())
  {
    _propSetMutex.unlock();
    succeed = false;
  }
  else
  {
    _myPropertySets[propSet.getScope()] = &propSet;
    _propSetMutex.unlock();
      
    _bufferMutex.lock();
    BlobOStream extraData(_extraDataBuf);
    extraData.clear();
    _extraDataBuf.clear();
    extraData.putStart(0);
    extraData << propSet.getScope();
    extraData << propSet.getType();
    extraData << (char) propSet.getCategory();  
    
    bufferAction(DH_PIProtocol::REGISTER_SCOPE, &propSet);
    _bufferMutex.unlock();
  }

  return succeed;
}

void PIClient::unregisterScope(CEPPropertySet& propSet, bool stillEnabling)
{
  // will be called from user thread context  
  _bufferMutex.lock();
  BlobOStream extraData(_extraDataBuf);
  extraData.clear();
  _extraDataBuf.clear();
  extraData.putStart(0);
  extraData << propSet.getScope();
  uint16 waitForSeqNr = 0;
  if (stillEnabling)
  {
    
    for (TStartedSequences::iterator iter = _startedSequences.begin();
         iter != _startedSequences.end(); ++iter)
    {
      if (iter->second == &propSet)
      {
        LOG_DEBUG(formatString(
            "Unregister of '%s' will be send delayed (after compl. seq. %d)!",
            propSet.getScope().c_str(),
            iter->first));
        ASSERT(iter->second->_state == CEPPropertySet::S_ENABLING);
        waitForSeqNr = iter->first;
        // response not needed to be forwarded to the prop. set.
        iter->second = 0; 
        break;
      }
    }
    
  }
  bufferAction(DH_PIProtocol::UNREGISTER_SCOPE, &propSet, waitForSeqNr);
  _bufferMutex.unlock();

  _propSetMutex.lock();

  _myPropertySets.erase(propSet.getScope());

  _propSetMutex.unlock();
}

void PIClient::propertiesLinked(const string& scope, TPIResult result)
{
  // will be called in piClient thread context
  _bufferMutex.lock();
  BlobOStream extraData(_extraDataBuf);
  extraData.clear();
  _extraDataBuf.clear();
  extraData.putStart(0);
  extraData << scope;
  extraData << (uint16) result;
    
  bufferAction(DH_PIProtocol::PROPSET_LINKED, 0);
  _bufferMutex.unlock();
}

void PIClient::propertiesUnlinked(const string& scope, TPIResult result)
{
  // will be called in piClient thread context
  _bufferMutex.lock();
  BlobOStream extraData(_extraDataBuf);
  extraData.clear();
  _extraDataBuf.clear();
  extraData.putStart(0);
  extraData << scope;
  extraData << (uint16) result;
  bufferAction(DH_PIProtocol::PROPSET_UNLINKED, 0);  
  _bufferMutex.unlock();  
}

void PIClient::valueSet(CEPPropertySet& propSet, const string& propName, const GCFPValue& value)
{
  // will be called from user thread context
  _bufferMutex.lock();
  BlobOStream extraData(_extraDataBuf);
  extraData.clear();
  _extraDataBuf.clear();
  extraData.putStart(0);

  extraData << propName;

  uint16 valSize(value.getSize());
  extraData << valSize;

  if (_upperboundValueBuf < valSize)
  {
    // enlarge the value buffer
    _upperboundValueBuf = valSize;
    if (_valueBuf) delete [] _valueBuf;
    _valueBuf = new char[valSize];
  }
  value.pack(_valueBuf);
  extraData.put(_valueBuf, valSize);

  bufferAction(DH_PIProtocol::VALUE_SET, &propSet);  
  _bufferMutex.unlock();
}

void PIClient::scopeRegistered()
{
  // will be called in piClient thread context
  CEPPropertySet* pPropertySet = _startedSequences[_dhPIClient.getSeqNr()];
  _startedSequences.erase(_dhPIClient.getSeqNr());
    
  _bufferMutex.lock();
  for (TBufferedActions::iterator iter = _bufferedActions.begin();
       iter != _bufferedActions.end(); ++iter)
  {
    if (iter->waitForSeqNr == _dhPIClient.getSeqNr()) 
    {
      // buffered action, which waits for this response can now send
      iter->waitForSeqNr = 0; 
      break;
    }
  }
  _bufferMutex.unlock();
  if (pPropertySet)
  {
    // unpacks the extra blob
    BlobIStream& blob = _dhPIClient.getExtraBlob();
    uint16 result;
    blob >> result;

    _propSetMutex.lock();
    logResult((TPIResult) result, *pPropertySet);

    if (result != PI_NO_ERROR)
    {
      _myPropertySets.erase(pPropertySet->getScope());      
    }
    pPropertySet->scopeRegistered((result == PI_NO_ERROR));
    _propSetMutex.unlock();
  }
  // else it is deleted in the meanwhile
}

void PIClient::scopeUnregistered()
{
  // will be called in piClient thread context
  CEPPropertySet* pPropertySet = _startedSequences[_dhPIClient.getSeqNr()];
  
  _startedSequences.erase(_dhPIClient.getSeqNr());
  
  if (pPropertySet)
  {
    // unpacks the extra blob
    BlobIStream& blob = _dhPIClient.getExtraBlob();
    uint16 result;
    blob >> result;
  
    _propSetMutex.lock();
  
    logResult((TPIResult) result, *pPropertySet);
    pPropertySet->scopeUnregistered((result == PI_NO_ERROR));
  
    _propSetMutex.unlock();
  }
  // else it is deleted in the meanwhile
}

void PIClient::linkPropSet()
{
  // will be called in piClient thread context
  BlobIStream& blob = _dhPIClient.getExtraBlob();
  string scope;
  blob >> scope;

  LOG_INFO(formatString ( 
    "PA-REQ: Link properties of prop. set '%s'",        
    scope.c_str()));
    
  _propSetMutex.lock();

  CEPPropertySet* pPropertySet = _myPropertySets[scope];

  if (pPropertySet)
  {
    pPropertySet->linkProperties();
    _propSetMutex.unlock();
  }
  else
  {
    _propSetMutex.unlock();
    LOG_DEBUG(formatString ( 
        "Property set with scope %s was deleted in the meanwhile", 
        scope.c_str()));
    propertiesLinked(scope, PI_PS_GONE);
  }
}

void PIClient::unlinkPropSet()
{
  // will be called in piClient thread context
  BlobIStream& blob = _dhPIClient.getExtraBlob();
  string scope;
  blob >> scope;

  LOG_INFO(formatString ( 
    "PA-REQ: Unlink properties of prop. set '%s'",
    scope.c_str()));
    
  _propSetMutex.lock();

  CEPPropertySet* pPropertySet = _myPropertySets[scope];

  if (pPropertySet)
  {
    pPropertySet->unlinkProperties();
    _propSetMutex.unlock();
  }
  else
  {
    _propSetMutex.unlock();
    LOG_DEBUG(formatString ( 
        "Property set with scope %s was deleted in the meanwhile", 
        scope.c_str()));
    propertiesUnlinked(scope, PI_PS_GONE);
  }
}

void PIClient::bufferAction(DH_PIProtocol::TEventID event, 
                            CEPPropertySet* pPropSet,
                            uint16 waitForSeqNr)
{
  // will be called in different thread context
  TAction action;
  action.eventID = event;
  action.pPropSet = pPropSet;
  action.waitForSeqNr = waitForSeqNr;
  
  uint32 neededSize = _extraDataBuf.size() - sizeof(BlobHeader);
  // _extraDataBuf is set in the calling method
  action.extraData = new char[neededSize];
  memcpy(action.extraData, _extraDataBuf.getBuffer() + sizeof(BlobHeader), neededSize);
  action.extraDataSize = neededSize;

  if (event == DH_PIProtocol::VALUE_SET)
  {
    if (_bufferedValues.size() > 1000)
    {
      _bufferedValues.pop_front();
    }
    _bufferedValues.push_back(action);
  }
  else
  {
    _bufferedActions.push_back(action); 
  }
}

uint16 PIClient::startSequence(CEPPropertySet& propSet)
{
  // will be called in piClient thread context
  uint16 seqnr(0); 
  TStartedSequences::const_iterator iter;

  // searches a unique sequence number for a asynchronous message sequence
  do   
  {
    seqnr++; // 0 is reserved for internal use in PA
    iter = _startedSequences.find(seqnr);
  } while (iter != _startedSequences.end());

  _startedSequences[seqnr] = &propSet; 
  return seqnr;
}

void PIClient::processOutstandingActions()
{
  // will be called in piClient thread context
  bool sent(true);
  CEPPropertySet* pPropSet(0);
  TBufferedActions::iterator iter;
  TAction action;
  while (sent)
  {
    _bufferMutex.lock();
    iter = _bufferedActions.begin();
    if (iter == _bufferedActions.end())
    {
      _bufferMutex.unlock();
      break;
    }
    if (iter->waitForSeqNr > 0) 
    {
      _bufferMutex.unlock();
      // pretend this action is sent, because it must be skipped
      sent = true; 
      // skip this action, which waits for a response of another sequence
      continue; 
    }
    _dhPIClient.setEventID(iter->eventID);
    BlobOStream& blob = _dhPIClient.createExtraBlob();
    switch (iter->eventID)
    {
      case DH_PIProtocol::REGISTER_SCOPE:
      case DH_PIProtocol::UNREGISTER_SCOPE:
        pPropSet = iter->pPropSet;
        ASSERT(pPropSet);
        _dhPIClient.setSeqNr(startSequence(*pPropSet));
        LOG_DEBUG(formatString(
            "Send request for '%s' to PI with seq. nr. %d.",
            pPropSet->getScope().c_str(),
            _dhPIClient.getSeqNr()));
        break;
      default:
        break;
    }
    blob.put(iter->extraData, iter->extraDataSize);
    action = *iter;
    _bufferMutex.unlock();  
    
    sent = _pWriteConn->write();
    if (sent)
    {
      // now the action can be removed
      // because the action could be removed between the last unlock and
      // lock of the mutex, we must be carefull 
      // thats why the 'action' is remembered and searched
      // action, if still exists, should still the first in the buffer
      // if 'action' differs the current first action in the buffer no erase
      // is needed anymore
      _bufferMutex.lock();     
      iter = _bufferedActions.begin();
      if (iter != _bufferedActions.end())
      {
        if (*iter == action)
        {
          delete [] action.extraData;
          _bufferedActions.erase(iter);
        }
      }
      _bufferMutex.unlock();
    }
  }
  while (sent)
  {
    // lock again for the value buffer
    _bufferMutex.lock();
    iter = _bufferedValues.begin();
    if (iter == _bufferedValues.end()) 
    {
      _bufferMutex.unlock();
      break;
    }
    if (iter->waitForSeqNr > 0) 
    {
      _bufferMutex.unlock();
      // pretend this action is sent, because it must be skipped
      sent = true; 
      // skip this action, which waits for a response of another sequence
      continue; 
    }
    _dhPIClient.setEventID(iter->eventID);
    BlobOStream& blob = _dhPIClient.createExtraBlob();
    ASSERT(iter->eventID == DH_PIProtocol::VALUE_SET);
    blob.put(iter->extraData, iter->extraDataSize);
    action = *iter;
    _bufferMutex.unlock();
    
    sent = _pWriteConn->write();
    if (sent)
    {
      // see description for the following above
      _bufferMutex.lock();     
      iter = _bufferedValues.begin();
      if (iter != _bufferedValues.end())
      {
        if (*iter == action)
        {
          delete [] action.extraData;
          _bufferedValues.erase(iter);
        }
      }
      _bufferMutex.unlock();
    }
  }  
}

void logResult(TPIResult result, CEPPropertySet& propSet)
{
  switch (result)
  {
    case PI_NO_ERROR:
      // nothing to log
      break;
    case PI_UNKNOWN_ERROR:
      LOG_FATAL("Unknown error");
      break;
    case PI_PS_GONE:
      LOG_ERROR(formatString ( 
          "The property set is gone while perfoming an action on it. (%s:%s)",
          propSet.getType().c_str(), propSet.getScope().c_str()));
      break;
    case PI_MISSING_PROPS:
      LOG_ERROR(formatString ( 
          "One or more loaded properties are not owned by any application. (%s:%s)",
          propSet.getType().c_str(), propSet.getScope().c_str()));
      break;
    case PI_WRONG_STATE:
      LOG_FATAL(formatString ( 
          "The my property set is in a wrong state. (%s:%s)",
          propSet.getType().c_str(), propSet.getScope().c_str()));
      break;
    case PI_PROP_SET_NOT_EXISTS:
      LOG_INFO(formatString ( 
          "Prop. set does not exists. (%s:%s)",
          propSet.getType().c_str(), propSet.getScope().c_str()));
      break;
    case PI_PROP_SET_ALREADY_EXISTS:
      LOG_INFO(formatString ( 
          "Prop. set allready exists. (%s:%s)",
          propSet.getType().c_str(), propSet.getScope().c_str()));
      break;
    case PI_DPTYPE_UNKNOWN:
      LOG_INFO(formatString ( 
          "Specified type not known. (%s:%s)",
          propSet.getType().c_str(), propSet.getScope().c_str()));
      break;
    case PI_INTERNAL_ERROR:
      LOG_FATAL(formatString ( 
          "Internal error in PI. (%s:%s)",
          propSet.getType().c_str(), propSet.getScope().c_str()));
      break;
    case PI_PA_INTERNAL_ERROR:
      LOG_FATAL(formatString ( 
          "Internal error in PA. (%s:%s)",
          propSet.getType().c_str(), propSet.getScope().c_str()));
      break;
    default:
      break;
  }
}
  } // namespace CEPPMLlight
 } // namespace GCF
} // namespace LOFAR
