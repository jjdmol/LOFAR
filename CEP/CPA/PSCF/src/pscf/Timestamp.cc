//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C7F3B77034D.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C7F3B77034D.cm

//## begin module%3C7F3B77034D.cp preserve=no
//## end module%3C7F3B77034D.cp

//## Module: Timestamp%3C7F3B77034D; Package body
//## Subsystem: PSCF%3C5A73670223
//## Source file: f:\lofar8\oms\LOFAR\cep\cpa\pscf\src\pscf\Timestamp.cc

//## begin module%3C7F3B77034D.additionalIncludes preserve=no
//## end module%3C7F3B77034D.additionalIncludes

//## begin module%3C7F3B77034D.includes preserve=yes
//## end module%3C7F3B77034D.includes

// Timestamp
#include "Timestamp.h"
//## begin module%3C7F3B77034D.declarations preserve=no
//## end module%3C7F3B77034D.declarations

//## begin module%3C7F3B77034D.additionalDeclarations preserve=yes
//## end module%3C7F3B77034D.additionalDeclarations


// Class Timestamp 

Timestamp::Timestamp()
  //## begin Timestamp::Timestamp%3C7F3B1D025E_const.hasinit preserve=no
  //## end Timestamp::Timestamp%3C7F3B1D025E_const.hasinit
  //## begin Timestamp::Timestamp%3C7F3B1D025E_const.initialization preserve=yes
  //## end Timestamp::Timestamp%3C7F3B1D025E_const.initialization
{
  //## begin Timestamp::Timestamp%3C7F3B1D025E_const.body preserve=yes
  struct timeval tm;
  gettimeofday(&tm,0);
  sec_ = tm.tv_sec;
  usec_ = tm.tv_usec;
  //## end Timestamp::Timestamp%3C7F3B1D025E_const.body
}



//## Other Operations (implementation)
Timestamp & Timestamp::operator += (const Timestamp &other)
{
  //## begin Timestamp::operator +=%3C7F3D500287.body preserve=yes
  sec_ += other.sec_;
  usec_ += other.usec_;
  normalize();
  return *this;
  //## end Timestamp::operator +=%3C7F3D500287.body
}

Timestamp & Timestamp::operator -= (const Timestamp &other)
{
  //## begin Timestamp::operator -=%3C7F3D720312.body preserve=yes
  sec_ -= other.sec_;
  usec_ -= other.usec_;
  normalize();
  return *this;
  //## end Timestamp::operator -=%3C7F3D720312.body
}

void Timestamp::normalize ()
{
  //## begin Timestamp::normalize%3C7F3E68035C.body preserve=yes
  while( usec_ >= 1000000 ) 
  {
    sec_++;
    usec_ -= 1000000;
  }
  while( usec_ < 0 ) 
  {
    sec_--;
    usec_ += 1000000;
  }
  //## end Timestamp::normalize%3C7F3E68035C.body
}

// Additional Declarations
  //## begin Timestamp%3C7F3B1D025E.declarations preserve=yes
  //## end Timestamp%3C7F3B1D025E.declarations

//## begin module%3C7F3B77034D.epilog preserve=yes
//## end module%3C7F3B77034D.epilog


