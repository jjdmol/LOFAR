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
//## Source file: F:\lofar8\oms\LOFAR\CEP\CPA\PSCF\src\pscf\WorkProcess.h

#ifndef WorkProcess_h
#define WorkProcess_h 1

//## begin module%3C7B7F3000C3.additionalIncludes preserve=no
#include "Common.h"
#include "DMI.h"
//## end module%3C7B7F3000C3.additionalIncludes

//## begin module%3C7B7F3000C3.includes preserve=yes
//## end module%3C7B7F3000C3.includes

// WPInterface
#include "WPInterface.h"
// Dispatcher
#include "Dispatcher.h"
//## begin module%3C7B7F3000C3.declarations preserve=no
//## end module%3C7B7F3000C3.declarations

//## begin module%3C7B7F3000C3.additionalDeclarations preserve=yes
//## end module%3C7B7F3000C3.additionalDeclarations


//## begin WorkProcess%3C8F25430087.preface preserve=yes
//## end WorkProcess%3C8F25430087.preface

//## Class: WorkProcess%3C8F25430087; Abstract
//## Category: PSCF%3BCEC935032A
//## Subsystem: PSCF%3C5A73670223
//## Persistence: Transient
//## Cardinality/Multiplicity: n



//## Uses: <unnamed>%3C8F3679014E;Dispatcher { -> }

class WorkProcess : public WPInterface  //## Inherits: <unnamed>%3C8F263A00E6
{
  //## begin WorkProcess%3C8F25430087.initialDeclarations preserve=yes
  //## end WorkProcess%3C8F25430087.initialDeclarations

  public:
    //## Constructors (specified)
      //## Operation: WorkProcess%3C8F25DB014E
      WorkProcess (AtomicID wpc);


    //## Other Operations (specified)
      //## Operation: start%3C9216B701CA
      virtual void start ();

      //## Operation: stop%3C9216C10015
      virtual void stop ();

      //## Operation: addTimeout%3C7D285803B0
      void addTimeout (const Timestamp &period, const HIID &id = HIID(), int flags = 0, int priority = Message::PRI_EVENT);

      //## Operation: addInput%3C7D2874023E
      void addInput (int fd, int flags, int priority = Message::PRI_EVENT);

      //## Operation: addSignal%3C7DFE520239
      void addSignal (int signum, int flags, int priority = Message::PRI_EVENT);

      //## Operation: removeTimeout%3C7D287F02C6
      bool removeTimeout (const HIID &id);

      //## Operation: removeInput%3C7D28A30141
      bool removeInput (int fd);

      //## Operation: removeSignal%3C7DFE480253
      bool removeSignal (int signum);

      //## Operation: send%3C7CB9E802CF
      //	Sends message to specified address. Note that the ref is taken over
      //	by this call, then privatized for writing. See Dispatcher::send()
      //	for more details.
      int send (MessageRef msg, MsgAddress to);

      //## Operation: publish%3C7CB9EB01CF
      //	Publishes message with the specified scope. Note that the ref is
      //	taken over by this call, then privatized for writing. This method is
      //	just a shorthand for send(), with "Publish" in some parts of the
      //	address, as determined by scope).
      int publish (MessageRef msg, int scope = Message::GLOBAL);

    //## Get and Set Operations for Class Attributes (generated)

      //## Attribute: state%3C8F256E024B
      int getState () const;
      void setState (int value);

    // Additional Public Declarations
      //## begin WorkProcess%3C8F25430087.public preserve=yes
      // import in the message result-codes
      typedef Message::MessageResults MessageResults;
      
      Declare_sdebug( );
      //## end WorkProcess%3C8F25430087.public
  protected:
    // Additional Protected Declarations
      //## begin WorkProcess%3C8F25430087.protected preserve=yes
      //## end WorkProcess%3C8F25430087.protected

  private:
    //## Constructors (generated)
      WorkProcess();

      WorkProcess(const WorkProcess &right);

    //## Assignment Operation (generated)
      WorkProcess & operator=(const WorkProcess &right);

    // Additional Private Declarations
      //## begin WorkProcess%3C8F25430087.private preserve=yes
      //## end WorkProcess%3C8F25430087.private

  private: //## implementation
    // Data Members for Class Attributes

      //## begin WorkProcess::state%3C8F256E024B.attr preserve=no  public: int {U} 
      int state;
      //## end WorkProcess::state%3C8F256E024B.attr

    // Additional Implementation Declarations
      //## begin WorkProcess%3C8F25430087.implementation preserve=yes
      //## end WorkProcess%3C8F25430087.implementation

};

//## begin WorkProcess%3C8F25430087.postscript preserve=yes
//## end WorkProcess%3C8F25430087.postscript

// Class WorkProcess 

//## Get and Set Operations for Class Attributes (inline)

inline int WorkProcess::getState () const
{
  //## begin WorkProcess::getState%3C8F256E024B.get preserve=no
  return state;
  //## end WorkProcess::getState%3C8F256E024B.get
}

inline void WorkProcess::setState (int value)
{
  //## begin WorkProcess::setState%3C8F256E024B.set preserve=no
  state = value;
  //## end WorkProcess::setState%3C8F256E024B.set
}

//## begin module%3C7B7F3000C3.epilog preserve=yes
//## end module%3C7B7F3000C3.epilog


#endif
