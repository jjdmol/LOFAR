//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C7B7F2F0248.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C7B7F2F0248.cm

//## begin module%3C7B7F2F0248.cp preserve=no
//## end module%3C7B7F2F0248.cp

//## Module: Message%3C7B7F2F0248; Package specification
//## Subsystem: PSCF%3C5A73670223
//## Source file: F:\lofar8\oms\LOFAR\cep\cpa\pscf\src\pscf\Message.h

#ifndef Message_h
#define Message_h 1

//## begin module%3C7B7F2F0248.additionalIncludes preserve=no
#include "Common.h"
#include "DMI.h"
//## end module%3C7B7F2F0248.additionalIncludes

//## begin module%3C7B7F2F0248.includes preserve=yes
//## end module%3C7B7F2F0248.includes

// PSCFDebugContext
#include "PSCFDebugContext.h"
// MsgAddress
#include "MsgAddress.h"
// CountedRef
#include "CountedRef.h"
// CountedRefTarget
#include "CountedRefTarget.h"
// SmartBlock
#include "SmartBlock.h"
// NestableContainer
#include "NestableContainer.h"
// HIID
#include "HIID.h"
// BlockableObject
#include "BlockableObject.h"
//## begin module%3C7B7F2F0248.declarations preserve=no
//## end module%3C7B7F2F0248.declarations

//## begin module%3C7B7F2F0248.additionalDeclarations preserve=yes
#pragma aidgroup PSCF
// standard event messages
#pragma aid MsgEvent MsgTimeout MsgInput MsgSignal
// hello/bye messages for WPs
#pragma aid MsgHello MsgBye

#include "AID-PSCF.h"
//## end module%3C7B7F2F0248.additionalDeclarations


//## begin Message%3C7B6A2D01F0.preface preserve=yes
class WPQueue;
//## end Message%3C7B6A2D01F0.preface

//## Class: Message%3C7B6A2D01F0
//## Category: PSCF%3BCEC935032A
//## Subsystem: PSCF%3C5A73670223
//## Persistence: Transient
//## Cardinality/Multiplicity: n



//## Uses: <unnamed>%3C7B70830385;MsgAddress { -> }
//## Uses: <unnamed>%3C7B708B00DD;HIID { -> }
//## Uses: <unnamed>%3C7E4D87012D;NestableContainer { -> }

class Message : public CountedRefTarget, //## Inherits: <unnamed>%3C7B903501C5
                	public PSCFDebugContext  //## Inherits: <unnamed>%3C7FA31802FF
{
  //## begin Message%3C7B6A2D01F0.initialDeclarations preserve=yes
  //## end Message%3C7B6A2D01F0.initialDeclarations

  public:
    //## Constructors (generated)
      Message();

      Message(const Message &right);

    //## Constructors (specified)
      //## Operation: Message%3C8CB2CE00DC
      Message (const HIID &id1, short pri = 0);

      //## Operation: Message%3C7B9C490384
      Message (const HIID &id1, BlockableObject *pload = 0, int flags = 0, int pri = 0);

      //## Operation: Message%3C7B9D0A01FB
      Message (const HIID &id1, ObjRef &pload, int flags = 0, int pri = 0);

      //## Operation: Message%3C7B9D3B02C3
      Message (const HIID &id1, SmartBlock *bl, int flags = 0, int pri = 0);

      //## Operation: Message%3C7B9D59014A
      Message (const HIID &id1, BlockRef &bl, int flags = 0, int pri = 0);

      //## Operation: Message%3C7BB3BD0266
      Message (const HIID &id1, const char *data, size_t sz, int pri = 0);

      //## Operation: Message%3C7B9E29013F
      Message (char *block, size_t sz);

    //## Destructor (generated)
      ~Message();

    //## Assignment Operation (generated)
      Message & operator=(const Message &right);


    //## Other Operations (specified)
      //## Operation: operator <<=%3C7B9DDE0137
      Message & operator <<= (BlockableObject *pload);

      //## Operation: operator <<=%3C7B9DF20014
      Message & operator <<= (ObjRef &pload);

      //## Operation: operator <<=%3C7B9E0A02AD
      Message & operator <<= (SmartBlock *bl);

      //## Operation: operator <<=%3C7B9E1601CE
      Message & operator <<= (BlockRef &bl);

      //## Operation: clone%3C7E32BE01E0; C++
      //	Abstract method for cloning an object. Should return pointer to new
      //	object. Flags: DMI::WRITE if writable clone is required, DMI::DEEP
      //	for deep cloning (i.e. contents of object will be cloned as well).
      virtual CountedRefTarget* clone (int flags = 0, int depth = 0) const;

      //## Operation: privatize%3C7E32C1022B
      //	Virtual method for privatization of an object. If the object
      //	contains other refs, they should be privatized by this method. The
      //	DMI::DEEP flag should be passed on to child refs, for deep
      //	privatization.
      virtual void privatize (int flags = 0, int depth = 0);

      //## Operation: operator []%3C7F56ED007D
      NestableContainer::Hook operator [] (const HIID &id);

      //## Operation: operator []%3C7E4C310348
      NestableContainer::Hook operator [] (int n);

      //## Operation: operator []%3C7E4C3E003A
      NestableContainer::ConstHook operator [] (const HIID &id) const;

      //## Operation: operator []%3C7F56D90197
      NestableContainer::ConstHook operator [] (int n) const;

      //## Operation: data%3C7E443A016A
      void * data ();

      //## Operation: data%3C7E446B02B5
      const void * data () const;

      //## Operation: datasize%3C7E443E01B6
      size_t datasize () const;

    //## Get and Set Operations for Class Attributes (generated)

      //## Attribute: priority%3C7B94970023
      int priority () const;
      void setPriority (int value);

      //## Attribute: state%3C7E33F40330
      int state () const;
      void setState (int value);

    //## Get and Set Operations for Associations (generated)

      //## Association: PSCF::<unnamed>%3C7B70FF033D
      //## Role: Message::to%3C7B7100015E
      const MsgAddress& to () const;
      void setTo (const MsgAddress& value);

      //## Association: PSCF::<unnamed>%3C7B71050151
      //## Role: Message::from%3C7B7106029D
      const MsgAddress& from () const;
      void setFrom (const MsgAddress& value);

      //## Association: PSCF::<unnamed>%3C7B71820219
      //## Role: Message::id%3C7B718500FB
      const HIID& id () const;
      void setId (const HIID& value);

      //## Association: PSCF::<unnamed>%3C7B9796024D
      //## Role: Message::payload%3C7B97970096
      const ObjRef& payload () const;

      //## Association: PSCF::<unnamed>%3C7B9799014D
      //## Role: Message::block%3C7B97990388
      const BlockRef& block () const;

    // Additional Public Declarations
      //## begin Message%3C7B6A2D01F0.public preserve=yes
      ObjRef& payload ();
      BlockRef& block ();
      
      // explicit versions of [] for string IDs
      NestableContainer::ConstHook operator [] (const string &id1) const
      { return (*this)[HIID(id1)]; }
      NestableContainer::ConstHook operator [] (const char *id1) const
      { return (*this)[HIID(id1)]; }
      NestableContainer::Hook operator [] (const string &id1) 
      { return (*this)[HIID(id1)]; }
      NestableContainer::Hook operator [] (const char *id1) 
      { return (*this)[HIID(id1)]; }
      
      // some predefined priority levels
      static const int PRI_NORMAL  = 0,
                       PRI_HIGH    = 10,
                       PRI_HIGHER  = 20,
                       PRI_EVENT   = 25,
                       PRI_LOW     = -10,
                       PRI_LOWER   = -20,
                       PRI_LOWEST  = -30;
      
      typedef CountedRef<Message> Ref;
      
      typedef enum {  
        ACCEPT   = 0, // message processed, OK to remove from queue
        HOLD     = 1, // hold the message (and block queue) until something else happens
        REQUEUE  = 2, // requeue the message and try next one

        CANCEL   = 3  // for input()/timeout()/signal(), cancel the input or timeout

      } MessageResults;
      
      typedef enum {
           GLOBAL        = 2,
           HOST          = 1,
           PROCESS       = 0
      } PublicationScope;
           
      typedef struct {
        WPQueue *wpq;
        int handle;
        HIID id;
        void *data;
      } TimeoutData;
      
      // This is a typical debug() method setup. The sdebug()
      // method creates a debug info string at the given level of detail.
      string sdebug ( int detail = 1,const string &prefix = "",
                const char *name = 0 ) const;
      const char * debug ( int detail = 1,const string &prefix = "",
                           const char *name = 0 ) const
      { return Debug::staticBuffer(sdebug(detail,prefix,name)); }

      //## end Message%3C7B6A2D01F0.public
  protected:
    // Additional Protected Declarations
      //## begin Message%3C7B6A2D01F0.protected preserve=yes
      BlockSet payload_set;
      typedef struct {
        size_t idsize;
        int priority;
        int state;
        char to[MsgAddress::byte_size],
             from[MsgAddress::byte_size];
        int num_payload_blocks;
        size_t block_size;
      } MessageBlock;
      //## end Message%3C7B6A2D01F0.protected
  private:
    // Additional Private Declarations
      //## begin Message%3C7B6A2D01F0.private preserve=yes
      //## end Message%3C7B6A2D01F0.private

  private: //## implementation
    // Data Members for Class Attributes

      //## begin Message::priority%3C7B94970023.attr preserve=no  public: int {U} 
      int priority_;
      //## end Message::priority%3C7B94970023.attr

      //## begin Message::state%3C7E33F40330.attr preserve=no  public: int {U} 
      int state_;
      //## end Message::state%3C7E33F40330.attr

    // Data Members for Associations

      //## Association: PSCF::<unnamed>%3C7B70FF033D
      //## begin Message::to%3C7B7100015E.role preserve=no  public: MsgAddress { -> 1VHgN}
      MsgAddress to_;
      //## end Message::to%3C7B7100015E.role

      //## Association: PSCF::<unnamed>%3C7B71050151
      //## begin Message::from%3C7B7106029D.role preserve=no  public: MsgAddress { -> 1VHgN}
      MsgAddress from_;
      //## end Message::from%3C7B7106029D.role

      //## Association: PSCF::<unnamed>%3C7B71820219
      //## begin Message::id%3C7B718500FB.role preserve=no  public: HIID { -> 1VHgN}
      HIID id_;
      //## end Message::id%3C7B718500FB.role

      //## Association: PSCF::<unnamed>%3C7B9796024D
      //## begin Message::payload%3C7B97970096.role preserve=no  public: BlockableObject { -> 0..1RHgN}
      ObjRef payload_;
      //## end Message::payload%3C7B97970096.role

      //## Association: PSCF::<unnamed>%3C7B9799014D
      //## begin Message::block%3C7B97990388.role preserve=no  public: SmartBlock { -> 0..1RHgN}
      BlockRef block_;
      //## end Message::block%3C7B97990388.role

    // Additional Implementation Declarations
      //## begin Message%3C7B6A2D01F0.implementation preserve=yes
      //## end Message%3C7B6A2D01F0.implementation

};

//## begin Message%3C7B6A2D01F0.postscript preserve=yes
//## end Message%3C7B6A2D01F0.postscript

//## begin MessageRef%3C7B722600DE.preface preserve=yes
//## end MessageRef%3C7B722600DE.preface

//## Class: MessageRef%3C7B722600DE
//## Category: PSCF%3BCEC935032A
//## Subsystem: PSCF%3C5A73670223
//## Persistence: Transient
//## Cardinality/Multiplicity: n



//## Uses: <unnamed>%3C7B7262018F;CountedRef { -> }
//## Uses: <unnamed>%3C7B726503E2;Message { -> }

typedef Message::Ref MessageRef;
//## begin MessageRef%3C7B722600DE.postscript preserve=yes
//## end MessageRef%3C7B722600DE.postscript

// Class Message 

inline Message::Message (const HIID &id1, short pri)
  //## begin Message::Message%3C8CB2CE00DC.hasinit preserve=no
  //## end Message::Message%3C8CB2CE00DC.hasinit
  //## begin Message::Message%3C8CB2CE00DC.initialization preserve=yes
   : priority_(pri),state_(0),id_(id1)
  //## end Message::Message%3C8CB2CE00DC.initialization
{
  //## begin Message::Message%3C8CB2CE00DC.body preserve=yes
  //## end Message::Message%3C8CB2CE00DC.body
}



//## Other Operations (inline)
inline NestableContainer::Hook Message::operator [] (const HIID &id)
{
  //## begin Message::operator []%3C7F56ED007D.body preserve=yes
  FailWhen( !payload_.valid() || !payload_->isNestable(),"payload is not a container" ); 
  return (*static_cast<NestableContainer*>(
      const_cast<BlockableObject*>(&payload_.deref())))[id];
  //## end Message::operator []%3C7F56ED007D.body
}

inline NestableContainer::Hook Message::operator [] (int n)
{
  //## begin Message::operator []%3C7E4C310348.body preserve=yes
  FailWhen( !payload_.valid() || !payload_->isNestable(),"payload is not a container" ); 
  return (*static_cast<NestableContainer*>(
      const_cast<BlockableObject*>(&payload_.deref())))[n];
  //## end Message::operator []%3C7E4C310348.body
}

inline NestableContainer::ConstHook Message::operator [] (const HIID &id) const
{
  //## begin Message::operator []%3C7E4C3E003A.body preserve=yes
  FailWhen( !payload_.valid() || !payload_->isNestable(),"payload is not a container" ); 
  return (*static_cast<const NestableContainer*>(&payload_.deref()))[id];
  //## end Message::operator []%3C7E4C3E003A.body
}

inline NestableContainer::ConstHook Message::operator [] (int n) const
{
  //## begin Message::operator []%3C7F56D90197.body preserve=yes
  FailWhen( !payload_.valid() || !payload_->isNestable(),"payload is not a container" ); 
  return (*static_cast<const NestableContainer*>(&payload_.deref()))[n];
  //## end Message::operator []%3C7F56D90197.body
}

inline void * Message::data ()
{
  //## begin Message::data%3C7E443A016A.body preserve=yes
  return block_.valid() ? block_().data() : 0;
  //## end Message::data%3C7E443A016A.body
}

inline const void * Message::data () const
{
  //## begin Message::data%3C7E446B02B5.body preserve=yes
  return block_.valid() ? block_->data() : 0;
  //## end Message::data%3C7E446B02B5.body
}

inline size_t Message::datasize () const
{
  //## begin Message::datasize%3C7E443E01B6.body preserve=yes
  return block_.valid() ? block_->size() : 0;
  //## end Message::datasize%3C7E443E01B6.body
}

//## Get and Set Operations for Class Attributes (inline)

inline int Message::priority () const
{
  //## begin Message::priority%3C7B94970023.get preserve=no
  return priority_;
  //## end Message::priority%3C7B94970023.get
}

inline void Message::setPriority (int value)
{
  //## begin Message::setPriority%3C7B94970023.set preserve=no
  priority_ = value;
  //## end Message::setPriority%3C7B94970023.set
}

inline int Message::state () const
{
  //## begin Message::state%3C7E33F40330.get preserve=no
  return state_;
  //## end Message::state%3C7E33F40330.get
}

inline void Message::setState (int value)
{
  //## begin Message::setState%3C7E33F40330.set preserve=no
  state_ = value;
  //## end Message::setState%3C7E33F40330.set
}

//## Get and Set Operations for Associations (inline)

inline const MsgAddress& Message::to () const
{
  //## begin Message::to%3C7B7100015E.get preserve=no
  return to_;
  //## end Message::to%3C7B7100015E.get
}

inline void Message::setTo (const MsgAddress& value)
{
  //## begin Message::setTo%3C7B7100015E.set preserve=no
  to_ = value;
  //## end Message::setTo%3C7B7100015E.set
}

inline const MsgAddress& Message::from () const
{
  //## begin Message::from%3C7B7106029D.get preserve=no
  return from_;
  //## end Message::from%3C7B7106029D.get
}

inline void Message::setFrom (const MsgAddress& value)
{
  //## begin Message::setFrom%3C7B7106029D.set preserve=no
  from_ = value;
  //## end Message::setFrom%3C7B7106029D.set
}

inline const HIID& Message::id () const
{
  //## begin Message::id%3C7B718500FB.get preserve=no
  return id_;
  //## end Message::id%3C7B718500FB.get
}

inline void Message::setId (const HIID& value)
{
  //## begin Message::setId%3C7B718500FB.set preserve=no
  id_ = value;
  //## end Message::setId%3C7B718500FB.set
}

inline const ObjRef& Message::payload () const
{
  //## begin Message::payload%3C7B97970096.get preserve=no
  return payload_;
  //## end Message::payload%3C7B97970096.get
}

inline const BlockRef& Message::block () const
{
  //## begin Message::block%3C7B97990388.get preserve=no
  return block_;
  //## end Message::block%3C7B97990388.get
}

//## begin module%3C7B7F2F0248.epilog preserve=yes
inline ObjRef& Message::payload () 
{
  return payload_;
}

inline BlockRef& Message::block () 
{
  return block_;
}

//## end module%3C7B7F2F0248.epilog


#endif
