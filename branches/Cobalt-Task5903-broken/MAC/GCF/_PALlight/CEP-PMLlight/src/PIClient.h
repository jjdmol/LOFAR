//#  PIClient.h: singleton class; bridge between CEP application 
//#                    and PropertyInterface
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

#ifndef PICLIENT_H
#define PICLIENT_H

#include "PICEPDefines.h"
#include <Common/lofar_map.h>
#include <Common/lofar_list.h>
#include <GCF/Thread.h>    
#include <GCF/Mutex.h>
#include <GCF/Protocols/DH_PIProtocol.h>
#include <GCF/Protocols/PI_Protocol.ph> // for the TPIResult enumeration

// This singleton class forms the bridge between the CEP-PMLlight API classes 
// and the PA via the PI. It is a hidden object in each CEP application, which 
// wants to be monitorable via the MAC subsystem of LOFAR. It will be created at 
// the moment a service of PI will be requested by the Application (like enable 
// property set). 
// Opposite to the (comparible) Controller classes in the PML and PMLlight packages, 
// this class will mainly run in a seperate thread (only started for this purpose).
// The communication with the PI will be performed with 
// the DH_PIProtocol class (from the PI package) and the TH_Socket class of 
// the LCS/Transport package.

namespace LOFAR 
{
class TH_Socket;
class CSConnection;

 namespace GCF 
 {
  namespace Common
  {
class GCFPValue;
  }
  namespace CEPPMLlight {
    
class CEPPropertySet;

class PIClient
{
  public:
    virtual ~PIClient ();
    // realizes the singleton pattern
    // <group>
    static PIClient* instance(bool temporary = false);
    static void release();
    // </group>

  public: // member functions
    // called by CEPPropertySet
    // <group>
    bool registerScope (CEPPropertySet& propSet);
    void unregisterScope (CEPPropertySet& propSet, bool stillEnabling);
       
    void propertiesLinked (const string& scope, 
                           TPIResult result);
    void propertiesUnlinked (const string& scope, 
                            TPIResult result);
    void valueSet(CEPPropertySet& propSet, 
                  const string& propName, 
                  const Common::GCFPValue& value);
    void deletePropSet(const CEPPropertySet& propSet);
    // </group>
    
  private:
    PIClient ();

  private: // helper methods
    void bufferAction (DH_PIProtocol::TEventID event, 
                       CEPPropertySet* pPropSet,
                       uint16 waitForSeqNr = 0);
                       
    void processOutstandingActions();

    uint16 startSequence(CEPPropertySet& propSet);
    
    static void* piClientThread (void *arg);
    void run();
    
    // part of the singelton pattern
    // <group>
    // @return true if no other objects use this singleton anymore otherwise false
    bool mayDeleted() { return (_usecount == 0);}
    // increments the uscount
    void use() { _usecount++;}
    // decrements the uscount
    void leave() { _usecount--;}
    // </group>
    
    // translated responses/request of the PI
    // <group>
    void scopeRegistered();
    void scopeUnregistered();
    void linkPropSet();
    void unlinkPropSet();
    // </group>
    
  private: // data members     
    // the protocol
    DH_PIProtocol   _dhPIClient;
    CSConnection*   _pReadConn;
    CSConnection*   _pWriteConn;
    
    typedef map<string /* scope */, CEPPropertySet*>  TMyPropertySets;
    TMyPropertySets _myPropertySets;

  public:
    typedef struct
    {
      CEPPropertySet* pPropSet;
      DH_PIProtocol::TEventID eventID;
      char* extraData;
      uint32 extraDataSize;
      uint16 waitForSeqNr;
    } TAction;
  private: // admistrative members
    typedef list<TAction>  TBufferedActions;
    TBufferedActions _bufferedActions;    
    TBufferedActions _bufferedValues;    

    typedef map<uint16 /*seqnr*/, CEPPropertySet*>  TStartedSequences;
    TStartedSequences _startedSequences;    

    GCF::Thread::ThrID _thread;
    GCF::Thread::Mutex _bufferMutex;
    GCF::Thread::Mutex _propSetMutex;
    
    static PIClient* _pInstance;
    // count the usage of this singleton
    unsigned int _usecount;
    
    char*         _valueBuf;
    unsigned int  _upperboundValueBuf;
    BlobOBufChar  _extraDataBuf;
};
  } // namespace CEPPMLlight
 } // namespace GCF
} // namespace LOFAR
#endif
