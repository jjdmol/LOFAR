//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C7B7F2F0380.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C7B7F2F0380.cm

//## begin module%3C7B7F2F0380.cp preserve=no
//## end module%3C7B7F2F0380.cp

//## Module: MsgAddress%3C7B7F2F0380; Package body
//## Subsystem: PSCF%3C5A73670223
//## Source file: f:\lofar8\oms\LOFAR\cep\cpa\pscf\src\pscf\MsgAddress.cc

//## begin module%3C7B7F2F0380.additionalIncludes preserve=no
//## end module%3C7B7F2F0380.additionalIncludes

//## begin module%3C7B7F2F0380.includes preserve=yes
//## end module%3C7B7F2F0380.includes

// MsgAddress
#include "MsgAddress.h"
//## begin module%3C7B7F2F0380.declarations preserve=no
//## end module%3C7B7F2F0380.declarations

//## begin module%3C7B7F2F0380.additionalDeclarations preserve=yes
//## end module%3C7B7F2F0380.additionalDeclarations


// Class MsgAddress 

MsgAddress::MsgAddress()
  //## begin MsgAddress::MsgAddress%3C7B6F790197_const.hasinit preserve=no
  //## end MsgAddress::MsgAddress%3C7B6F790197_const.hasinit
  //## begin MsgAddress::MsgAddress%3C7B6F790197_const.initialization preserve=yes
  //## end MsgAddress::MsgAddress%3C7B6F790197_const.initialization
{
  //## begin MsgAddress::MsgAddress%3C7B6F790197_const.body preserve=yes
  //## end MsgAddress::MsgAddress%3C7B6F790197_const.body
}

MsgAddress::MsgAddress(const MsgAddress &right)
  //## begin MsgAddress::MsgAddress%3C7B6F790197_copy.hasinit preserve=no
  //## end MsgAddress::MsgAddress%3C7B6F790197_copy.hasinit
  //## begin MsgAddress::MsgAddress%3C7B6F790197_copy.initialization preserve=yes
    : HIID(right)
  //## end MsgAddress::MsgAddress%3C7B6F790197_copy.initialization
{
  //## begin MsgAddress::MsgAddress%3C7B6F790197_copy.body preserve=yes
  //## end MsgAddress::MsgAddress%3C7B6F790197_copy.body
}


MsgAddress::~MsgAddress()
{
  //## begin MsgAddress::~MsgAddress%3C7B6F790197_dest.body preserve=yes
  //## end MsgAddress::~MsgAddress%3C7B6F790197_dest.body
}


MsgAddress & MsgAddress::operator=(const MsgAddress &right)
{
  //## begin MsgAddress::operator=%3C7B6F790197_assign.body preserve=yes
  *static_cast<HIID*>(this) = static_cast<const HIID&>(right);
  return *this;
  //## end MsgAddress::operator=%3C7B6F790197_assign.body
}


// Additional Declarations
  //## begin MsgAddress%3C7B6F790197.declarations preserve=yes
  //## end MsgAddress%3C7B6F790197.declarations

//## begin module%3C7B7F2F0380.epilog preserve=yes
//## end module%3C7B7F2F0380.epilog


// Detached code regions:
#if 0
//## begin MsgAddress::MsgAddress%3C7B6FAE00FD.initialization preserve=yes
    : HIID(3)
//## end MsgAddress::MsgAddress%3C7B6FAE00FD.initialization

//## begin MsgAddress::MsgAddress%3C7B6FAE00FD.body preserve=yes
  (*this)[0] = host;
  (*this)[1] = proc;
  (*this)[2] = wp;
//## end MsgAddress::MsgAddress%3C7B6FAE00FD.body

//## begin MsgAddress::wp%3C7B6FED02B7.body preserve=yes
  return (*this)[2];
//## end MsgAddress::wp%3C7B6FED02B7.body

#endif
