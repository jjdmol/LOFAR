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

#include <PIClient.h>
#include <GCF/PALlight/CEPPropertySet.h>
#include <GCF/Utils.h>
#include <GCF/ParameterSet.h>
#include <Transport/TH_Socket.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
using LOFAR::BlobIStream;
using LOFAR::BlobOStream;

namespace LOFAR {
 namespace GCF {
  namespace CEPPMLlight {
    

PIClient* PIClient::_pInstance = 0;

void logResult(TPIResult result, CEPPropertySet& propSet);

PIClient::PIClient() : 
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
    assert(_pInstance->mayDeleted());
  }
  if (!temporary) _pInstance->use();
  return _pInstance;
}

void PIClient::release()
{
  assert(_pInstance);
  assert(!_pInstance->mayDeleted());
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
      ParameterSet::instance()->adoptFile("PropertyInterface.conf");
      TH_Socket TCPto(ParameterSet::instance()->getString(PARAM_PI_HOST), 
                      "",
                      ParameterSet::instance()->getInt(PARAM_PI_PORT),
                      false,
                      false);
    
      TH_Socket TCPfrom("", 
                        ParameterSet::instance()->getString(PARAM_PI_HOST), 
                        ParameterSet::instance()->getInt(PARAM_PI_PORT),
                        true,
                        false);
      _dhPIClient.setID(1);
      // this dummy dataholder represents the remote server (Property Interface)
      DH_PIProtocol dhServer("dummy");
      dhServer.setID(2);
      
      LOG_DEBUG("Setup connection");
    
      // initilizes a virtual connection with the Property Interface
      _dhPIClient.connectBidirectional(dhServer, TCPto, TCPfrom, false);
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

    
  do
  {
    retry = false;
    try
    {
      LOG_DEBUG("Try to connect");
      // real synchronous connect with the Property Interface
      if (_dhPIClient.init())
      {    
        LOG_DEBUG("Connected to PropertyInterface of MAC");
      }
      else
      {
        retry = true;
        continue;
      }
  
      // the main loop of the thread
      while (true)
      {
        processOutstandingActions();
        
        if (_dhPIClient.read())
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
              assert(0);
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
 
      _propSetMutex.lock();  
      for (TMyPropertySets::iterator iter = _myPropertySets.begin();
           iter != _myPropertySets.end(); ++iter)
      {
        iter->second->scopeUnregistered(false);
      }
      TMyPropertySets tempMyPropertySets(_myPropertySets);
      _myPropertySets.clear();

      for (TMyPropertySets::iterator iter = tempMyPropertySets.begin();
           iter != tempMyPropertySets.end(); ++iter)
      {
        iter->second->enable();
      }

      _propSetMutex.unlock();
      sleep(10);
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
      seqToDelete.push_back(iter->first);
    }
  }
  for (list<uint16>::iterator iter = seqToDelete.begin();
       iter != seqToDelete.end(); ++iter)
  {
    _startedSequences.erase(*iter);
  }
  for (TBufferedActions::iterator iter = _bufferedActions.begin();
       iter != _bufferedActions.end(); ++iter)
  {
    if (iter->pPropSet == &propSet)
    {
      iter = _bufferedActions.erase(iter);
      if (iter != _bufferedActions.end()) break;
    }    
  }
  _bufferMutex.unlock();
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
      
    BlobOStream extraData(_extraDataBuf);
    extraData.clear();
    _extraDataBuf.clear();
    extraData.putStart(0);
    extraData << propSet.getScope();
    extraData << propSet.getType();
    extraData << (char) propSet.getCategory();  
    
    bufferAction(DH_PIProtocol::REGISTER_SCOPE, &propSet);
  }

  return succeed;
}

void PIClient::unregisterScope(CEPPropertySet& propSet)
{
  BlobOStream extraData(_extraDataBuf);
  extraData.clear();
  _extraDataBuf.clear();
  extraData.putStart(0);
  extraData << propSet.getScope();
  
  // will be called from user thread context
  bufferAction(DH_PIProtocol::UNREGISTER_SCOPE, &propSet);

  _propSetMutex.lock();

  _myPropertySets.erase(propSet.getScope());

  _propSetMutex.unlock();
}

void PIClient::propertiesLinked(const string& scope, TPIResult result)
{
  BlobOStream extraData(_extraDataBuf);
  extraData.clear();
  _extraDataBuf.clear();
  extraData.putStart(0);
  extraData << scope;
  extraData << (uint16) result;
  
  // will be called in piClient thread context
  bufferAction(DH_PIProtocol::PROPSET_LINKED, 0);
}

void PIClient::propertiesUnlinked(const string& scope, TPIResult result)
{
  // will be called in piClient thread context
  BlobOStream extraData(_extraDataBuf);
  extraData.clear();
  _extraDataBuf.clear();
  extraData.putStart(0);
  extraData << scope;
  extraData << (uint16) result;
  bufferAction(DH_PIProtocol::PROPSET_UNLINKED, 0);  
}

void PIClient::valueSet(const string& propName, const GCFPValue& value)
{
  // will be called from user thread context
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

  bufferAction(DH_PIProtocol::VALUE_SET, 0);  
}

void PIClient::scopeRegistered()
{
  // will be called in piClient thread context
  CEPPropertySet* pPropertySet = _startedSequences[_dhPIClient.getSeqNr()];
  _startedSequences.erase(_dhPIClient.getSeqNr());
    
  assert(pPropertySet);

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

void PIClient::scopeUnregistered()
{
  // will be called in piClient thread context
  CEPPropertySet* pPropertySet = _startedSequences[_dhPIClient.getSeqNr()];
  
  _startedSequences.erase(_dhPIClient.getSeqNr());
  
  assert(pPropertySet);
  BlobIStream& blob = _dhPIClient.getExtraBlob();
  uint16 result;
  blob >> result;

  _propSetMutex.lock();

  logResult((TPIResult) result, *pPropertySet);
  pPropertySet->scopeUnregistered((result == PI_NO_ERROR));

  _propSetMutex.unlock();
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
                            CEPPropertySet* pPropSet)
{
  // will be called in different thread context
  TAction action;
  action.eventID = event;
  action.pPropSet = pPropSet;
  //action.sent = false;
  
  uint32 neededSize = _extraDataBuf.size() - sizeof(BlobHeader);
  // _extraDataBuf is set in the calling method
  action.extraData = new char[neededSize];
  memcpy(action.extraData, _extraDataBuf.getBuffer() + sizeof(BlobHeader), neededSize);
  action.extraDataSize = neededSize;

  _bufferMutex.lock();

  _bufferedActions.push_back(action); 
  
  _bufferMutex.unlock();
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
  _bufferMutex.lock();
  bool sent(true);
  CEPPropertySet* pPropSet;
  for (TBufferedActions::iterator iter = _bufferedActions.begin();
       iter != _bufferedActions.end() && sent; ++iter)
  {
    _dhPIClient.setEventID(iter->eventID);
    BlobOStream& blob = _dhPIClient.createExtraBlob();
    switch (iter->eventID)
    {
      case DH_PIProtocol::REGISTER_SCOPE:
      case DH_PIProtocol::UNREGISTER_SCOPE:
        pPropSet = iter->pPropSet;
        assert(pPropSet);

        _dhPIClient.setSeqNr(startSequence(*pPropSet));

      default:
        break;
    }
    blob.put(iter->extraData, iter->extraDataSize);
    
    sent = false;
    try
    {
      sent = _dhPIClient.write();
    }
    catch (std::exception& e)
    {
      LOG_ERROR(formatString (
          "Exception: %s", 
          e.what()));
    }
    if (sent)
    {
      delete [] iter->extraData;
      _bufferedActions.erase(iter);
      iter--;
    }
  }  
  _bufferMutex.unlock();  
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
