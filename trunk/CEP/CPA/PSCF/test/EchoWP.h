//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3C7E49E90390.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3C7E49E90390.cm

//## begin module%3C7E49E90390.cp preserve=no
//## end module%3C7E49E90390.cp

//## Module: EchoWP%3C7E49E90390; Package specification
//## Subsystem: Testing%3C7E494F0184
//## Source file: F:\lofar8\oms\LOFAR\cep\cpa\pscf\test\EchoWP.h

#ifndef EchoWP_h
#define EchoWP_h 1

//## begin module%3C7E49E90390.additionalIncludes preserve=no
#include "Common.h"
#include "DMI.h"
//## end module%3C7E49E90390.additionalIncludes

//## begin module%3C7E49E90390.includes preserve=yes
//## end module%3C7E49E90390.includes

// WorkProcess
#include "WorkProcess.h"
//## begin module%3C7E49E90390.declarations preserve=no
//## end module%3C7E49E90390.declarations

//## begin module%3C7E49E90390.additionalDeclarations preserve=yes
#pragma aidgroup Testing
#pragma aid EchoWP Ping Pong
#pragma aid Reply Timestamp Invert Data Count
#include "AID-Testing.h"
//## end module%3C7E49E90390.additionalDeclarations


//## begin EchoWP%3C7E498E00D1.preface preserve=yes
//## end EchoWP%3C7E498E00D1.preface

//## Class: EchoWP%3C7E498E00D1
//## Category: Testing%3C7E49840235
//## Subsystem: Testing%3C7E494F0184
//## Persistence: Transient
//## Cardinality/Multiplicity: n



class EchoWP : public WorkProcess  //## Inherits: <unnamed>%3C8F26580162
{
  //## begin EchoWP%3C7E498E00D1.initialDeclarations preserve=yes
  //## end EchoWP%3C7E498E00D1.initialDeclarations

  public:
    //## Constructors (specified)
      //## Operation: EchoWP%3C7E49B60327
      EchoWP (int pingcount = 0);

    //## Destructor (generated)
      ~EchoWP();


    //## Other Operations (specified)
      //## Operation: init%3C7F884A007D
      virtual void init ();

      //## Operation: start%3C7E4AC70261
      virtual void start ();

      //## Operation: receive%3C7E49AC014C
      virtual int receive (MessageRef& mref);

      //## Operation: timeout%3C98CB600343
      virtual int timeout (const HIID &);

    // Additional Public Declarations
      //## begin EchoWP%3C7E498E00D1.public preserve=yes
      //## end EchoWP%3C7E498E00D1.public

  protected:
    // Additional Protected Declarations
      //## begin EchoWP%3C7E498E00D1.protected preserve=yes
      int pcount;
  
      void sendPing ();
      //## end EchoWP%3C7E498E00D1.protected
  private:
    //## Constructors (generated)
      EchoWP(const EchoWP &right);

    //## Assignment Operation (generated)
      EchoWP & operator=(const EchoWP &right);

    // Additional Private Declarations
      //## begin EchoWP%3C7E498E00D1.private preserve=yes
      //## end EchoWP%3C7E498E00D1.private

  private: //## implementation
    // Additional Implementation Declarations
      //## begin EchoWP%3C7E498E00D1.implementation preserve=yes
      //## end EchoWP%3C7E498E00D1.implementation

};

//## begin EchoWP%3C7E498E00D1.postscript preserve=yes
//## end EchoWP%3C7E498E00D1.postscript

// Class EchoWP 

//## begin module%3C7E49E90390.epilog preserve=yes
//## end module%3C7E49E90390.epilog


#endif


// Detached code regions:
#if 0
//## begin EchoWP::name%3C83546A03DC.body preserve=yes
  return "EchoWP";
//## end EchoWP::name%3C83546A03DC.body

#endif
