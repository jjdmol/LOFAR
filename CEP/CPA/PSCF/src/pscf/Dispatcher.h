//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C7B7F300041.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C7B7F300041.cm

//## begin module%3C7B7F300041.cp preserve=no
//## end module%3C7B7F300041.cp

//## Module: Dispatcher%3C7B7F300041; Package specification
//## Subsystem: PSCF%3C5A73670223
//## Source file: f:\lofar8\oms\LOFAR\cep\cpa\pscf\src\pscf\Dispatcher.h

#ifndef Dispatcher_h
#define Dispatcher_h 1

//## begin module%3C7B7F300041.additionalIncludes preserve=no
#include "Common.h"
#include "DMI.h"
//## end module%3C7B7F300041.additionalIncludes

//## begin module%3C7B7F300041.includes preserve=yes
#include <signal.h>
//## end module%3C7B7F300041.includes

// PSCFDebugContext
#include "PSCFDebugContext.h"
// Message
#include "Message.h"

class WPQueue;
class WorkProcess;

//## begin module%3C7B7F300041.declarations preserve=no
//## end module%3C7B7F300041.declarations

//## begin module%3C7B7F300041.additionalDeclarations preserve=yes
//## end module%3C7B7F300041.additionalDeclarations


//## begin Dispatcher%3C7B6A3E00A0.preface preserve=yes
//## end Dispatcher%3C7B6A3E00A0.preface

//## Class: Dispatcher%3C7B6A3E00A0
//## Category: PSCF%3BCEC935032A
//## Subsystem: PSCF%3C5A73670223
//## Persistence: Transient
//## Cardinality/Multiplicity: 1..1



//## Uses: <unnamed>%3C7E128D0235;Message { -> }
//## Uses: <unnamed>%3C7E136D0350;WorkProcess { -> F}

class Dispatcher : public PSCFDebugContext  //## Inherits: <unnamed>%3C7FA32C016D
{
  //## begin Dispatcher%3C7B6A3E00A0.initialDeclarations preserve=yes
  //## end Dispatcher%3C7B6A3E00A0.initialDeclarations

  public:
    //## Constructors (specified)
      //## Operation: Dispatcher%3C7CD444039C
      Dispatcher (AtomicID process, AtomicID host, int hz = 100);

    //## Destructor (generated)
      ~Dispatcher();


    //## Other Operations (specified)
      //## Operation: attach%3C7B885A027F
      void attach (WorkProcess *wp);

      //## Operation: start%3C7DFF770140
      void start ();

      //## Operation: stop%3C7E0270027B
      void stop ();

      //## Operation: send%3C7B8867015B
      int send (MessageRef &msg, const MsgAddress &to);

      //## Operation: publish%3C7CAE9B01E8
      int publish (MessageRef &msg, int scope = Message::GLOBAL);

      //## Operation: poll%3C7B888E01CF
      void poll ();

      //## Operation: addTimeout%3C7D28C30061
      int addTimeout (WPQueue *wpq, int ms, int flags, const HIID &id, void* data);

      //## Operation: addInput%3C7D28E3032E
      int addInput (WPQueue *wpq, int fd, int flags);

      //## Operation: addSignal%3C7DFF4A0344
      int addSignal (WPQueue *wpq, int signum);

      //## Operation: removeTimeout%3C7D28F202F3
      bool removeTimeout (WPQueue *wpq, int handle);

      //## Operation: removeInput%3C7D2947002F
      bool removeInput (WPQueue *wpq, int handle);

      //## Operation: removeSignal%3C7DFF57025C
      bool removeSignal (WPQueue *wpq, int signum);

    //## Get and Set Operations for Class Attributes (generated)

      //## Attribute: address%3C7CD390002C
      const MsgAddress& getAddress () const;

    // Additional Public Declarations
      //## begin Dispatcher%3C7B6A3E00A0.public preserve=yes
      AtomicID hostId () const        { return getAddress().host(); }
      AtomicID processId () const     { return getAddress().process(); }
      
      
      // This is a typical debug() method setup. The sdebug()
      // method creates a debug info string at the given level of detail.
      string sdebug ( int detail = 1,const string &prefix = "",
                const char *name = 0 ) const;
      const char * debug ( int detail = 1,const string &prefix = "",
                           const char *name = 0 ) const
      { return Debug::staticBuffer(sdebug(detail,prefix,name)); }
      
      //## end Dispatcher%3C7B6A3E00A0.public
  protected:
    // Additional Protected Declarations
      //## begin Dispatcher%3C7B6A3E00A0.protected preserve=yes
      // flag: dispatcher is running
      bool running;
  
      // flag: repoll required
      bool repoll;
  
      // pointer to static dispatcher object
      static Dispatcher * dispatcher;
      
      // set of outstanding signals
      static sigset_t raisedSignals;
      
      // static signal handler
      static void signalHandler (int signum,siginfo_t *siginfo,void *);
      // heartbeat timer datt
      struct sigaction orig_sigalrm_sa;
      bool sigalrm_handled;
      int heartbeat_hz;
      
      //## end Dispatcher%3C7B6A3E00A0.protected
  private:
    //## Constructors (generated)
      Dispatcher();

      Dispatcher(const Dispatcher &right);

    //## Assignment Operation (generated)
      Dispatcher & operator=(const Dispatcher &right);

    // Additional Private Declarations
      //## begin Dispatcher%3C7B6A3E00A0.private preserve=yes
      //## end Dispatcher%3C7B6A3E00A0.private

  private: //## implementation
    // Data Members for Class Attributes

      //## begin Dispatcher::address%3C7CD390002C.attr preserve=no  public: MsgAddress {U} 
      MsgAddress address;
      //## end Dispatcher::address%3C7CD390002C.attr

    // Data Members for Associations

      //## Association: PSCF::<unnamed>%3C7B73B800E6
      //## Role: Dispatcher::queues%3C7B73B9008E
      //## begin Dispatcher::queues%3C7B73B9008E.role preserve=no  private: WPQueue {1 -> 0..*RFHN}
      map<AtomicID,WPQueue*> queues;
      //## end Dispatcher::queues%3C7B73B9008E.role

    // Additional Implementation Declarations
      //## begin Dispatcher%3C7B6A3E00A0.implementation preserve=yes
      typedef map<AtomicID,WPQueue*>::iterator WPQI;
      typedef map<AtomicID,WPQueue*>::const_iterator CWPQI;
      //## end Dispatcher%3C7B6A3E00A0.implementation
};

//## begin Dispatcher%3C7B6A3E00A0.postscript preserve=yes
//## end Dispatcher%3C7B6A3E00A0.postscript

// Class Dispatcher 

//## Get and Set Operations for Class Attributes (inline)

inline const MsgAddress& Dispatcher::getAddress () const
{
  //## begin Dispatcher::getAddress%3C7CD390002C.get preserve=no
  return address;
  //## end Dispatcher::getAddress%3C7CD390002C.get
}

//## begin module%3C7B7F300041.epilog preserve=yes
//## end module%3C7B7F300041.epilog


#endif
