//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C7B7F300005.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C7B7F300005.cm

//## begin module%3C7B7F300005.cp preserve=no
//## end module%3C7B7F300005.cp

//## Module: WPQueue%3C7B7F300005; Package specification
//## Subsystem: PSCF%3C5A73670223
//## Source file: f:\lofar8\oms\LOFAR\cep\cpa\pscf\src\pscf\WPQueue.h

#ifndef WPQueue_h
#define WPQueue_h 1

//## begin module%3C7B7F300005.additionalIncludes preserve=no
#include "Common.h"
#include "DMI.h"
//## end module%3C7B7F300005.additionalIncludes

//## begin module%3C7B7F300005.includes preserve=yes
#include <list>
//## end module%3C7B7F300005.includes

// HIIDSet
#include "HIIDSet.h"
// PSCFDebugContext
#include "PSCFDebugContext.h"
// WorkProcess
#include "WorkProcess.h"
// Message
#include "Message.h"

class Dispatcher;

//## begin module%3C7B7F300005.declarations preserve=no
//## end module%3C7B7F300005.declarations

//## begin module%3C7B7F300005.additionalDeclarations preserve=yes
//## end module%3C7B7F300005.additionalDeclarations


//## begin WPQueue%3C7B72F901E6.preface preserve=yes
//## end WPQueue%3C7B72F901E6.preface

//## Class: WPQueue%3C7B72F901E6
//## Category: PSCF%3BCEC935032A
//## Subsystem: PSCF%3C5A73670223
//## Persistence: Transient
//## Cardinality/Multiplicity: n



//## Uses: <unnamed>%3C7E1289004F;Message { -> }

class WPQueue : public PSCFDebugContext  //## Inherits: <unnamed>%3C7FA32600CE
{
  //## begin WPQueue%3C7B72F901E6.initialDeclarations preserve=yes
  //## end WPQueue%3C7B72F901E6.initialDeclarations

  public:
    //## Constructors (generated)
      WPQueue();

    //## Constructors (specified)
      //## Operation: WPQueue%3C7E301D03D7
      WPQueue (Dispatcher* pdsp, WorkProcess* pwp);

    //## Destructor (generated)
      ~WPQueue();


    //## Other Operations (specified)
      //## Operation: attach%3C7B92AA007F
      void attach (Dispatcher* pdsp, WorkProcess *pwp);

      //## Operation: subscribe%3C7B935E03B3
      bool subscribe (const HIID &id);

      //## Operation: unsubscribe%3C7B9367008A
      bool unsubscribe (const HIID &id);

      //## Operation: subscribesTo%3C7B945D028D
      bool subscribesTo (const HIID &id);

      //## Operation: place%3C7B94670179
      bool place (const MessageRef &msg);

      //## Operation: topPriority%3C7CCD05016E
      int topPriority ();

      //## Operation: queueLocked%3C7D255F0153
      bool queueLocked ();

      //## Operation: performDelivery%3C7CCD0A014D
      bool performDelivery ();

    //## Get and Set Operations for Class Attributes (generated)

      //## Attribute: repoll%3C7CBF3C0065
      bool repoll () const;
      void setRepoll (bool value);

    //## Get and Set Operations for Associations (generated)

      //## Association: PSCF::<unnamed>%3C7B736603AF
      //## Role: WPQueue::wp%3C7B736702E9
      WorkProcess * getWp ();

      //## Association: PSCF::<unnamed>%3C7B73B800E6
      //## Role: WPQueue::dsp%3C7B73B900A2
      Dispatcher * getDsp ();

    // Additional Public Declarations
      //## begin WPQueue%3C7B72F901E6.public preserve=yes
      string sdebug ( int detail = 1,const string &prefix = "",
                const char *name = 0 ) const;
      const char * debug ( int detail = 1,const string &prefix = "",
                           const char *name = 0 ) const
      { return Debug::staticBuffer(sdebug(detail,prefix,name)); }
      //## end WPQueue%3C7B72F901E6.public

  protected:
    // Additional Protected Declarations
      //## begin WPQueue%3C7B72F901E6.protected preserve=yes
      bool full_lock,receive_lock;
      //## end WPQueue%3C7B72F901E6.protected
  private:
    //## Constructors (generated)
      WPQueue(const WPQueue &right);

    //## Assignment Operation (generated)
      WPQueue & operator=(const WPQueue &right);

    // Additional Private Declarations
      //## begin WPQueue%3C7B72F901E6.private preserve=yes
      //## end WPQueue%3C7B72F901E6.private

  private: //## implementation
    // Data Members for Class Attributes

      //## begin WPQueue::repoll%3C7CBF3C0065.attr preserve=no  public: bool {U} 
      bool repoll_;
      //## end WPQueue::repoll%3C7CBF3C0065.attr

    // Data Members for Associations

      //## Association: PSCF::<unnamed>%3C7B730A0263
      //## Role: WPQueue::queue%3C7B730B0174
      //## begin WPQueue::queue%3C7B730B0174.role preserve=no  private: MessageRef { -> 0..*VHgN}
      list<MessageRef> queue;
      //## end WPQueue::queue%3C7B730B0174.role

      //## Association: PSCF::<unnamed>%3C7B736603AF
      //## begin WPQueue::wp%3C7B736702E9.role preserve=no  public: WorkProcess {1 -> 1RHN}
      WorkProcess *wp;
      //## end WPQueue::wp%3C7B736702E9.role

      //## Association: PSCF::<unnamed>%3C7B73B800E6
      //## begin WPQueue::dsp%3C7B73B900A2.role preserve=no  public: Dispatcher {0..* -> 1RFHN}
      Dispatcher *dsp;
      //## end WPQueue::dsp%3C7B73B900A2.role

      //## Association: PSCF::<unnamed>%3C7B8C1700A7
      //## Role: WPQueue::subscriptions%3C7B8C1702EB
      //## begin WPQueue::subscriptions%3C7B8C1702EB.role preserve=no  private: HIIDSet { -> 1VHgN}
      HIIDSet subscriptions;
      //## end WPQueue::subscriptions%3C7B8C1702EB.role

    // Additional Implementation Declarations
      //## begin WPQueue%3C7B72F901E6.implementation preserve=yes
      typedef list<MessageRef>::iterator MLI;
      typedef list<MessageRef>::reverse_iterator RMLI;
      typedef list<MessageRef>::const_iterator CMLI;
      typedef list<MessageRef>::const_reverse_iterator CRMLI;
      //## end WPQueue%3C7B72F901E6.implementation
};

//## begin WPQueue%3C7B72F901E6.postscript preserve=yes
//## end WPQueue%3C7B72F901E6.postscript

// Class WPQueue 

//## Get and Set Operations for Class Attributes (inline)

inline bool WPQueue::repoll () const
{
  //## begin WPQueue::repoll%3C7CBF3C0065.get preserve=no
  return repoll_;
  //## end WPQueue::repoll%3C7CBF3C0065.get
}

inline void WPQueue::setRepoll (bool value)
{
  //## begin WPQueue::setRepoll%3C7CBF3C0065.set preserve=no
  repoll_ = value;
  //## end WPQueue::setRepoll%3C7CBF3C0065.set
}

//## Get and Set Operations for Associations (inline)

inline WorkProcess * WPQueue::getWp ()
{
  //## begin WPQueue::getWp%3C7B736702E9.get preserve=no
  return wp;
  //## end WPQueue::getWp%3C7B736702E9.get
}

inline Dispatcher * WPQueue::getDsp ()
{
  //## begin WPQueue::getDsp%3C7B73B900A2.get preserve=no
  return dsp;
  //## end WPQueue::getDsp%3C7B73B900A2.get
}

//## begin module%3C7B7F300005.epilog preserve=yes
//## end module%3C7B7F300005.epilog


#endif
