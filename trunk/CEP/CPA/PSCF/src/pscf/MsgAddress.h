//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C7B7F2F037E.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C7B7F2F037E.cm

//## begin module%3C7B7F2F037E.cp preserve=no
//## end module%3C7B7F2F037E.cp

//## Module: MsgAddress%3C7B7F2F037E; Package specification
//## Subsystem: PSCF%3C5A73670223
//## Source file: f:\lofar8\oms\LOFAR\cep\cpa\pscf\src\pscf\MsgAddress.h

#ifndef MsgAddress_h
#define MsgAddress_h 1

//## begin module%3C7B7F2F037E.additionalIncludes preserve=no
#include "Common.h"
#include "DMI.h"
//## end module%3C7B7F2F037E.additionalIncludes

//## begin module%3C7B7F2F037E.includes preserve=yes
//## end module%3C7B7F2F037E.includes

// HIID
#include "HIID.h"
//## begin module%3C7B7F2F037E.declarations preserve=no
//## end module%3C7B7F2F037E.declarations

//## begin module%3C7B7F2F037E.additionalDeclarations preserve=yes
#pragma aidgroup PSCF
#pragma aid Dispatcher Local Broadcast
//## end module%3C7B7F2F037E.additionalDeclarations


//## begin MsgAddress%3C7B6F790197.preface preserve=yes
//## end MsgAddress%3C7B6F790197.preface

//## Class: MsgAddress%3C7B6F790197
//## Category: PSCF%3BCEC935032A
//## Subsystem: PSCF%3C5A73670223
//## Persistence: Transient
//## Cardinality/Multiplicity: n



class MsgAddress : public HIID  //## Inherits: <unnamed>%3C7B6F9B0088
{
  //## begin MsgAddress%3C7B6F790197.initialDeclarations preserve=yes
  //## end MsgAddress%3C7B6F790197.initialDeclarations

  public:
    //## Constructors (generated)
      MsgAddress();

      MsgAddress(const MsgAddress &right);

    //## Constructors (specified)
      //## Operation: MsgAddress%3C7B6FAE00FD
      MsgAddress (AtomicID wp, AtomicID proc = 0, AtomicID host = 0);

    //## Destructor (generated)
      ~MsgAddress();

    //## Assignment Operation (generated)
      MsgAddress & operator=(const MsgAddress &right);


    //## Other Operations (specified)
      //## Operation: wp%3C7B6FED02B7
      AtomicID wp () const;

      //## Operation: process%3C7B6FF7033D
      AtomicID process () const;

      //## Operation: host%3C7B6FFC0330
      AtomicID host () const;

    // Additional Public Declarations
      //## begin MsgAddress%3C7B6F790197.public preserve=yes
      // special address constants
      static const int DISPATCHER = 1,
                       LOCAL = 2,
                       BROADCAST = 3;
      
      static const size_t byte_size = 3*sizeof(int);
      //## end MsgAddress%3C7B6F790197.public
  protected:
    // Additional Protected Declarations
      //## begin MsgAddress%3C7B6F790197.protected preserve=yes
      //## end MsgAddress%3C7B6F790197.protected

  private:
    // Additional Private Declarations
      //## begin MsgAddress%3C7B6F790197.private preserve=yes
      //## end MsgAddress%3C7B6F790197.private

  private: //## implementation
    // Additional Implementation Declarations
      //## begin MsgAddress%3C7B6F790197.implementation preserve=yes
      //## end MsgAddress%3C7B6F790197.implementation

};

//## begin MsgAddress%3C7B6F790197.postscript preserve=yes
//## end MsgAddress%3C7B6F790197.postscript

// Class MsgAddress 

inline MsgAddress::MsgAddress (AtomicID wp, AtomicID proc, AtomicID host)
  //## begin MsgAddress::MsgAddress%3C7B6FAE00FD.hasinit preserve=no
  //## end MsgAddress::MsgAddress%3C7B6FAE00FD.hasinit
  //## begin MsgAddress::MsgAddress%3C7B6FAE00FD.initialization preserve=yes
    : HIID(wp)
  //## end MsgAddress::MsgAddress%3C7B6FAE00FD.initialization
{
  //## begin MsgAddress::MsgAddress%3C7B6FAE00FD.body preserve=yes
  add(proc);
  add(host);
  //## end MsgAddress::MsgAddress%3C7B6FAE00FD.body
}



//## Other Operations (inline)
inline AtomicID MsgAddress::wp () const
{
  //## begin MsgAddress::wp%3C7B6FED02B7.body preserve=yes
  return (*this)[0];
  //## end MsgAddress::wp%3C7B6FED02B7.body
}

inline AtomicID MsgAddress::process () const
{
  //## begin MsgAddress::process%3C7B6FF7033D.body preserve=yes
  return (*this)[1];
  //## end MsgAddress::process%3C7B6FF7033D.body
}

inline AtomicID MsgAddress::host () const
{
  //## begin MsgAddress::host%3C7B6FFC0330.body preserve=yes
  return (*this)[2];
  //## end MsgAddress::host%3C7B6FFC0330.body
}

//## begin module%3C7B7F2F037E.epilog preserve=yes
//## end module%3C7B7F2F037E.epilog


#endif
