//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C95AADB0101.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C95AADB0101.cm

//## begin module%3C95AADB0101.cp preserve=no
//## end module%3C95AADB0101.cp

//## Module: GWServerWP%3C95AADB0101; Package specification
//## Subsystem: PSCF%3C5A73670223
//## Source file: F:\lofar8\oms\LOFAR\CEP\CPA\PSCF\src\pscf\GWServerWP.h

#ifndef GWServerWP_h
#define GWServerWP_h 1

//## begin module%3C95AADB0101.additionalIncludes preserve=no
#include "Common.h"
#include "DMI.h"
//## end module%3C95AADB0101.additionalIncludes

//## begin module%3C95AADB0101.includes preserve=yes
//## end module%3C95AADB0101.includes

// Socket
#include "pscf/Socket.h"
// GWClientWP
#include "GWClientWP.h"
// GatewayWP
#include "GatewayWP.h"
// WorkProcess
#include "WorkProcess.h"
//## begin module%3C95AADB0101.declarations preserve=no
//## end module%3C95AADB0101.declarations

//## begin module%3C95AADB0101.additionalDeclarations preserve=yes
#pragma aidgroup PSCF
#pragma aid Gateway GWServerWP GWClientWP GatewayWP
//## end module%3C95AADB0101.additionalDeclarations


//## begin GWServerWP%3C8F942502BA.preface preserve=yes
//## end GWServerWP%3C8F942502BA.preface

//## Class: GWServerWP%3C8F942502BA
//## Category: PSCF%3BCEC935032A
//## Subsystem: PSCF%3C5A73670223
//## Persistence: Transient
//## Cardinality/Multiplicity: n



//## Uses: <unnamed>%3C90BEFA02E4;GatewayWP { -> }
//## Uses: <unnamed>%3C95A9DA0327;GWClientWP { -> }

class GWServerWP : public WorkProcess  //## Inherits: <unnamed>%3C8F943E01B2
{
  //## begin GWServerWP%3C8F942502BA.initialDeclarations preserve=yes
  //## end GWServerWP%3C8F942502BA.initialDeclarations

  public:
    //## Constructors (specified)
      //## Operation: GWServerWP%3C8F95710177
      GWServerWP (const string &port1);

    //## Destructor (generated)
      ~GWServerWP();


    //## Other Operations (specified)
      //## Operation: start%3C90BE4A029B
      virtual void start ();

      //## Operation: stop%3C90BE880037
      virtual void stop ();

      //## Operation: timeout%3C90BE8E000E
      virtual int timeout (const HIID &);

      //## Operation: input%3C95B4DC031C
      virtual int input (int , int );

    //## Get and Set Operations for Class Attributes (generated)

      //## Attribute: port%3C90BE3503C7
      const string& getPort () const;

    // Additional Public Declarations
      //## begin GWServerWP%3C8F942502BA.public preserve=yes
      //## end GWServerWP%3C8F942502BA.public

  protected:
    // Additional Protected Declarations
      //## begin GWServerWP%3C8F942502BA.protected preserve=yes
      // tries to open server socket
      void tryOpen ();
      //## end GWServerWP%3C8F942502BA.protected
  private:
    //## Constructors (generated)
      GWServerWP();

      GWServerWP(const GWServerWP &right);

    //## Assignment Operation (generated)
      GWServerWP & operator=(const GWServerWP &right);

    // Additional Private Declarations
      //## begin GWServerWP%3C8F942502BA.private preserve=yes
      //## end GWServerWP%3C8F942502BA.private

  private: //## implementation
    // Data Members for Class Attributes

      //## begin GWServerWP::port%3C90BE3503C7.attr preserve=no  public: string {U} 
      string port;
      //## end GWServerWP::port%3C90BE3503C7.attr

    // Data Members for Associations

      //## Association: PSCF::<unnamed>%3C922571000B
      //## Role: GWServerWP::sock%3C92257101CE
      //## begin GWServerWP::sock%3C92257101CE.role preserve=no  private: Socket { -> 0..1RHgN}
      Socket *sock;
      //## end GWServerWP::sock%3C92257101CE.role

    // Additional Implementation Declarations
      //## begin GWServerWP%3C8F942502BA.implementation preserve=yes
      int open_retries;
      //## end GWServerWP%3C8F942502BA.implementation
};

//## begin GWServerWP%3C8F942502BA.postscript preserve=yes
//## end GWServerWP%3C8F942502BA.postscript

// Class GWServerWP 

//## Get and Set Operations for Class Attributes (inline)

inline const string& GWServerWP::getPort () const
{
  //## begin GWServerWP::getPort%3C90BE3503C7.get preserve=no
  return port;
  //## end GWServerWP::getPort%3C90BE3503C7.get
}

//## begin module%3C95AADB0101.epilog preserve=yes
//## end module%3C95AADB0101.epilog


#endif
