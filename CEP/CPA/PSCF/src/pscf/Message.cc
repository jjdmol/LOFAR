//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C7B7F2F024A.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C7B7F2F024A.cm

//## begin module%3C7B7F2F024A.cp preserve=no
//## end module%3C7B7F2F024A.cp

//## Module: Message%3C7B7F2F024A; Package body
//## Subsystem: PSCF%3C5A73670223
//## Source file: F:\lofar8\oms\LOFAR\cep\cpa\pscf\src\pscf\Message.cc

//## begin module%3C7B7F2F024A.additionalIncludes preserve=no
//## end module%3C7B7F2F024A.additionalIncludes

//## begin module%3C7B7F2F024A.includes preserve=yes
//## end module%3C7B7F2F024A.includes

// Message
#include "Message.h"
//## begin module%3C7B7F2F024A.declarations preserve=no
//## end module%3C7B7F2F024A.declarations

//## begin module%3C7B7F2F024A.additionalDeclarations preserve=yes
//## end module%3C7B7F2F024A.additionalDeclarations


// Class Message 

Message::Message()
  //## begin Message::Message%3C7B6A2D01F0_const.hasinit preserve=no
  //## end Message::Message%3C7B6A2D01F0_const.hasinit
  //## begin Message::Message%3C7B6A2D01F0_const.initialization preserve=yes
  //## end Message::Message%3C7B6A2D01F0_const.initialization
{
  //## begin Message::Message%3C7B6A2D01F0_const.body preserve=yes
  //## end Message::Message%3C7B6A2D01F0_const.body
}

Message::Message(const Message &right)
  //## begin Message::Message%3C7B6A2D01F0_copy.hasinit preserve=no
  //## end Message::Message%3C7B6A2D01F0_copy.hasinit
  //## begin Message::Message%3C7B6A2D01F0_copy.initialization preserve=yes
    : CountedRefTarget()
  //## end Message::Message%3C7B6A2D01F0_copy.initialization
{
  //## begin Message::Message%3C7B6A2D01F0_copy.body preserve=yes
  (*this) = right;
  //## end Message::Message%3C7B6A2D01F0_copy.body
}

Message::Message (const HIID &id1, BlockableObject *pload, int flags, int pri)
  //## begin Message::Message%3C7B9C490384.hasinit preserve=no
  //## end Message::Message%3C7B9C490384.hasinit
  //## begin Message::Message%3C7B9C490384.initialization preserve=yes
   : priority_(pri),state_(0),id_(id1)
  //## end Message::Message%3C7B9C490384.initialization
{
  //## begin Message::Message%3C7B9C490384.body preserve=yes
  payload_.attach(pload,flags|DMI::PERSIST|DMI::WRITE);
  //## end Message::Message%3C7B9C490384.body
}

Message::Message (const HIID &id1, ObjRef &pload, int flags, int pri)
  //## begin Message::Message%3C7B9D0A01FB.hasinit preserve=no
  //## end Message::Message%3C7B9D0A01FB.hasinit
  //## begin Message::Message%3C7B9D0A01FB.initialization preserve=yes
   : priority_(pri),state_(0),id_(id1)
  //## end Message::Message%3C7B9D0A01FB.initialization
{
  //## begin Message::Message%3C7B9D0A01FB.body preserve=yes
  if( flags&DMI::COPYREF )
    payload_.copy(pload,flags|DMI::PERSIST|DMI::WRITE);
  else
    payload_.xfer(pload).persist();
  //## end Message::Message%3C7B9D0A01FB.body
}

Message::Message (const HIID &id1, SmartBlock *bl, int flags, int pri)
  //## begin Message::Message%3C7B9D3B02C3.hasinit preserve=no
  //## end Message::Message%3C7B9D3B02C3.hasinit
  //## begin Message::Message%3C7B9D3B02C3.initialization preserve=yes
   : priority_(pri),state_(0),id_(id1)
  //## end Message::Message%3C7B9D3B02C3.initialization
{
  //## begin Message::Message%3C7B9D3B02C3.body preserve=yes
  block_.attach(bl,flags|DMI::PERSIST|DMI::WRITE);
  //## end Message::Message%3C7B9D3B02C3.body
}

Message::Message (const HIID &id1, BlockRef &bl, int flags, int pri)
  //## begin Message::Message%3C7B9D59014A.hasinit preserve=no
  //## end Message::Message%3C7B9D59014A.hasinit
  //## begin Message::Message%3C7B9D59014A.initialization preserve=yes
   : priority_(pri),state_(0),id_(id1)
  //## end Message::Message%3C7B9D59014A.initialization
{
  //## begin Message::Message%3C7B9D59014A.body preserve=yes
  if( flags&DMI::COPYREF )
    block_.copy(bl,flags|DMI::PERSIST|DMI::WRITE);
  else
    block_.xfer(bl).persist();
  //## end Message::Message%3C7B9D59014A.body
}

Message::Message (const HIID &id1, const char *data, size_t sz, int pri)
  //## begin Message::Message%3C7BB3BD0266.hasinit preserve=no
  //## end Message::Message%3C7BB3BD0266.hasinit
  //## begin Message::Message%3C7BB3BD0266.initialization preserve=yes
   : priority_(pri),state_(0),id_(id1)
  //## end Message::Message%3C7BB3BD0266.initialization
{
  //## begin Message::Message%3C7BB3BD0266.body preserve=yes
  SmartBlock *bl = new SmartBlock(sz);
  block_.attach( bl,DMI::ANON|DMI::WRITE|DMI::PERSIST);
  memcpy(bl->data(),data,sz);
  //## end Message::Message%3C7BB3BD0266.body
}

Message::Message (char *block, size_t sz)
  //## begin Message::Message%3C7B9E29013F.hasinit preserve=no
  //## end Message::Message%3C7B9E29013F.hasinit
  //## begin Message::Message%3C7B9E29013F.initialization preserve=yes
  //## end Message::Message%3C7B9E29013F.initialization
{
  //## begin Message::Message%3C7B9E29013F.body preserve=yes
  //## end Message::Message%3C7B9E29013F.body
}


Message::~Message()
{
  //## begin Message::~Message%3C7B6A2D01F0_dest.body preserve=yes
  //## end Message::~Message%3C7B6A2D01F0_dest.body
}


Message & Message::operator=(const Message &right)
{
  //## begin Message::operator=%3C7B6A2D01F0_assign.body preserve=yes
  id_ = right.id_;
  priority_ = right.priority_;
  from_ = right.from_;
  to_ = right.to_;
  state_ = right.state_;
//  timestamp_ = right.timestamp_;
  payload_.copy(right.payload_,DMI::PRESERVE_RW|DMI::PERSIST);
  block_.copy(right.block_,DMI::PRESERVE_RW|DMI::PERSIST);
  return *this;
  //## end Message::operator=%3C7B6A2D01F0_assign.body
}



//## Other Operations (implementation)
Message & Message::operator <<= (BlockableObject *pload)
{
  //## begin Message::operator <<=%3C7B9DDE0137.body preserve=yes
  payload_.attach(pload,DMI::ANON|DMI::WRITE|DMI::PERSIST);
  return *this;
  //## end Message::operator <<=%3C7B9DDE0137.body
}

Message & Message::operator <<= (ObjRef &pload)
{
  //## begin Message::operator <<=%3C7B9DF20014.body preserve=yes
  payload_.xfer(pload).persist();
  return *this;
  //## end Message::operator <<=%3C7B9DF20014.body
}

Message & Message::operator <<= (SmartBlock *bl)
{
  //## begin Message::operator <<=%3C7B9E0A02AD.body preserve=yes
  block_.attach(bl,DMI::ANON|DMI::WRITE|DMI::PERSIST);
  return *this;
  //## end Message::operator <<=%3C7B9E0A02AD.body
}

Message & Message::operator <<= (BlockRef &bl)
{
  //## begin Message::operator <<=%3C7B9E1601CE.body preserve=yes
  block_.xfer(bl).persist();
  return *this;
  //## end Message::operator <<=%3C7B9E1601CE.body
}

CountedRefTarget* Message::clone (int flags, int depth) const
{
  //## begin Message::clone%3C7E32BE01E0.body preserve=yes
  Message *newmsg = new Message(*this);
  newmsg->privatize(flags,depth);
  return newmsg;
  //## end Message::clone%3C7E32BE01E0.body
}

void Message::privatize (int flags, int depth)
{
  //## begin Message::privatize%3C7E32C1022B.body preserve=yes
  if( flags&DMI::DEEP || depth>0 )
  {
    if( payload_.valid() )
      payload_.privatize(flags,depth-1);
    if( block_.valid() )
      block_.privatize(flags,depth-1);
  }
  //## end Message::privatize%3C7E32C1022B.body
}

// Additional Declarations
  //## begin Message%3C7B6A2D01F0.declarations preserve=yes
string Message::sdebug ( int detail,const string &prefix,const char *name ) const
{
  string out;
  if( detail>=0 ) // basic detail
  {
    out = name?name:"Message";
    out += "/" + id_.toString();
    if( detail>3 )
      out += Debug::ssprintf("/%08x",this);
  }
  if( detail >= 1 || detail == -1 )   // normal detail
  {
    Debug::appendf(out,"%s->%s p/%d s/%x",
        from_.toString().c_str(),to_.toString().c_str(),priority_,state_);
    if( detail == 1 )
    {
      Debug::appendf(out,payload_.valid() ? "w/payload" : "",
                         block_.valid() ? "w/block" : "");
    }
  }
  if( detail >=2 || detail <= -2) // high detail
  {
    if( out.length() )
      out += "\n"+prefix;
    if( payload_.valid() )
      out += "  payload: "+payload_.sdebug(abs(detail)-1,prefix+"    ");
    else
      out += "  no payload";
    if( block_.valid() )
      out += "\n"+prefix+"  block: "+block_.sdebug(abs(detail)-1,prefix+"    ");
    else
      out += "  no block";
  }
  return out;
}
  //## end Message%3C7B6A2D01F0.declarations
//## begin module%3C7B7F2F024A.epilog preserve=yes
//## end module%3C7B7F2F024A.epilog
