//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C7B7F3000C3.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C7B7F3000C3.cm

//## begin module%3C7B7F3000C3.cp preserve=no
//## end module%3C7B7F3000C3.cp

//## Module: WorkProcess%3C7B7F3000C3; Package specification
//## Subsystem: PSCF%3C5A73670223
//## Source file: f:\lofar8\oms\LOFAR\cep\cpa\pscf\src\pscf\WorkProcess.h

#ifndef WorkProcess_h
#define WorkProcess_h 1

//## begin module%3C7B7F3000C3.additionalIncludes preserve=no
#include "Common.h"
#include "DMI.h"
//## end module%3C7B7F3000C3.additionalIncludes

//## begin module%3C7B7F3000C3.includes preserve=yes
//## end module%3C7B7F3000C3.includes

// PSCFDebugContext
#include "PSCFDebugContext.h"
// Message
#include "Message.h"

class WPQueue;
class Dispatcher;

//## begin module%3C7B7F3000C3.declarations preserve=no
//## end module%3C7B7F3000C3.declarations

//## begin module%3C7B7F3000C3.additionalDeclarations preserve=yes
//## end module%3C7B7F3000C3.additionalDeclarations


//## begin WorkProcess%3C7B6A3702E5.preface preserve=yes
//## end WorkProcess%3C7B6A3702E5.preface

//## Class: WorkProcess%3C7B6A3702E5; Abstract
//## Category: PSCF%3BCEC935032A
//## Subsystem: PSCF%3C5A73670223
//## Persistence: Transient
//## Cardinality/Multiplicity: n



//## Uses: <unnamed>%3C7E133B016D;Message { -> }

class WorkProcess : public PSCFDebugContext  //## Inherits: <unnamed>%3C7FA31F00CE
{
  //## begin WorkProcess%3C7B6A3702E5.initialDeclarations preserve=yes
  //## end WorkProcess%3C7B6A3702E5.initialDeclarations

  public:
    //## Constructors (generated)
      WorkProcess();

    //## Constructors (specified)
      //## Operation: WorkProcess%3C7CBB10027A
      WorkProcess (AtomicID wpid);

    //## Destructor (generated)
      virtual ~WorkProcess();


    //## Other Operations (specified)
      //## Operation: name%3C83541601B4
      virtual const char * name () const;

      //## Operation: attach%3C7CBAED007B
      void attach (WPQueue* pq);

      //## Operation: init%3C7F882B00E6
      virtual void init ();

      //## Operation: start%3C7E4A99016B
      virtual void start ();

      //## Operation: stop%3C7E4A9C0133
      virtual void stop ();

      //## Operation: isAttached%3C7CBBD101E1
      bool isAttached () const;

      //## Operation: subscribe%3C7CB9B70120
      bool subscribe (const HIID &id);

      //## Operation: unsubscribe%3C7CB9C50365
      bool unsubscribe (const HIID &id);

      //## Operation: addTimeout%3C7D285803B0
      int addTimeout (int ms, int flags = 0, const HIID &id = HIID(), void *data = 0);

      //## Operation: addInput%3C7D2874023E
      int addInput (int fd, int flags);

      //## Operation: addSignal%3C7DFE520239
      int addSignal (int signum);

      //## Operation: removeTimeout%3C7D287F02C6
      bool removeTimeout (int handle);

      //## Operation: removeInput%3C7D28A30141
      bool removeInput (int handle);

      //## Operation: removeSignal%3C7DFE480253
      bool removeSignal (int signum);

      //## Operation: send%3C7CB9E802CF
      int send (MessageRef &msg, MsgAddress to);

      //## Operation: publish%3C7CB9EB01CF
      int publish (MessageRef &msg, int scope = Message::GLOBAL);

      //## Operation: receive%3C7CC0950089
      virtual int receive (MessageRef& mref) = 0;

      //## Operation: timeout%3C7CC2AB02AD
      virtual int timeout (int handle, const HIID &id, void* data);

      //## Operation: input%3C7CC2C40386
      virtual int input (int handle, int fd, int flags);

      //## Operation: signal%3C7DFD240203
      virtual int signal (int signum);

    //## Get and Set Operations for Class Attributes (generated)

      //## Attribute: id%3C7B8EA50093
      AtomicID getId () const;

      //## Attribute: address%3C7CBA880058
      const MsgAddress& getAddress () const;

      //## Attribute: state%3C7E343002FB
      int getState () const;

    //## Get and Set Operations for Associations (generated)

      //## Association: PSCF::<unnamed>%3C7B736603AF
      //## Role: WorkProcess::queue%3C7B736702F3
      const WPQueue * getQueue () const;

      //## Association: PSCF::<unnamed>%3C7E14150352
      //## Role: WorkProcess::dsp%3C7E1416017C
      const Dispatcher * getDsp () const;

    // Additional Public Declarations
      //## begin WorkProcess%3C7B6A3702E5.public preserve=yes
      // import in the message result-codes
      typedef Message::MessageResults MessageResults;
        
      // This is a typical debug() method setup. The sdebug()
      // method creates a debug info string at the given level of detail.
      virtual string sdebug ( int detail = 1,const string &prefix = "",
                const char *name = 0 ) const;
      const char * debug ( int detail = 1,const string &prefix = "",
                           const char *name = 0 ) const
      { return Debug::staticBuffer(sdebug(detail,prefix,name)); }
        
      //## end WorkProcess%3C7B6A3702E5.public
  protected:
    // Additional Protected Declarations
      //## begin WorkProcess%3C7B6A3702E5.protected preserve=yes
      //## end WorkProcess%3C7B6A3702E5.protected

  private:
    //## Constructors (generated)
      WorkProcess(const WorkProcess &right);

    //## Assignment Operation (generated)
      WorkProcess & operator=(const WorkProcess &right);

    // Additional Private Declarations
      //## begin WorkProcess%3C7B6A3702E5.private preserve=yes
      //## end WorkProcess%3C7B6A3702E5.private

  private: //## implementation
    // Data Members for Class Attributes

      //## begin WorkProcess::id%3C7B8EA50093.attr preserve=no  public: AtomicID {U} 
      AtomicID id;
      //## end WorkProcess::id%3C7B8EA50093.attr

      //## begin WorkProcess::address%3C7CBA880058.attr preserve=no  public: MsgAddress {U} 
      MsgAddress address;
      //## end WorkProcess::address%3C7CBA880058.attr

      //## begin WorkProcess::state%3C7E343002FB.attr preserve=no  public: int {U} 
      int state;
      //## end WorkProcess::state%3C7E343002FB.attr

    // Data Members for Associations

      //## Association: PSCF::<unnamed>%3C7B736603AF
      //## begin WorkProcess::queue%3C7B736702F3.role preserve=no  public: WPQueue {1 -> 1RFHN}
      WPQueue *queue;
      //## end WorkProcess::queue%3C7B736702F3.role

      //## Association: PSCF::<unnamed>%3C7E14150352
      //## begin WorkProcess::dsp%3C7E1416017C.role preserve=no  public: Dispatcher { -> 1RFHN}
      Dispatcher *dsp;
      //## end WorkProcess::dsp%3C7E1416017C.role

    // Additional Implementation Declarations
      //## begin WorkProcess%3C7B6A3702E5.implementation preserve=yes
      //## end WorkProcess%3C7B6A3702E5.implementation

};

//## begin WorkProcess%3C7B6A3702E5.postscript preserve=yes
//## end WorkProcess%3C7B6A3702E5.postscript

// Class WorkProcess 


//## Other Operations (inline)
inline const char * WorkProcess::name () const
{
  //## begin WorkProcess::name%3C83541601B4.body preserve=yes
  return "WP";
  //## end WorkProcess::name%3C83541601B4.body
}

inline bool WorkProcess::isAttached () const
{
  //## begin WorkProcess::isAttached%3C7CBBD101E1.body preserve=yes
  return queue != 0;
  //## end WorkProcess::isAttached%3C7CBBD101E1.body
}

//## Get and Set Operations for Class Attributes (inline)

inline AtomicID WorkProcess::getId () const
{
  //## begin WorkProcess::getId%3C7B8EA50093.get preserve=no
  return id;
  //## end WorkProcess::getId%3C7B8EA50093.get
}

inline const MsgAddress& WorkProcess::getAddress () const
{
  //## begin WorkProcess::getAddress%3C7CBA880058.get preserve=no
  return address;
  //## end WorkProcess::getAddress%3C7CBA880058.get
}

inline int WorkProcess::getState () const
{
  //## begin WorkProcess::getState%3C7E343002FB.get preserve=no
  return state;
  //## end WorkProcess::getState%3C7E343002FB.get
}

//## Get and Set Operations for Associations (inline)

inline const WPQueue * WorkProcess::getQueue () const
{
  //## begin WorkProcess::getQueue%3C7B736702F3.get preserve=no
  return queue;
  //## end WorkProcess::getQueue%3C7B736702F3.get
}

inline const Dispatcher * WorkProcess::getDsp () const
{
  //## begin WorkProcess::getDsp%3C7E1416017C.get preserve=no
  return dsp;
  //## end WorkProcess::getDsp%3C7E1416017C.get
}

//## begin module%3C7B7F3000C3.epilog preserve=yes
//## end module%3C7B7F3000C3.epilog


#endif
