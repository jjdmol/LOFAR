//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3CEA390F01EA.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3CEA390F01EA.cm

//## begin module%3CEA390F01EA.cp preserve=no
//## end module%3CEA390F01EA.cp

//## Module: MSFillerWP%3CEA390F01EA; Package specification
//## Subsystem: UVD%3CD133E2028D
//## Source file: F:\lofar8\oms\LOFAR\src-links\UVD\MSFillerWP.h

#ifndef MSFillerWP_h
#define MSFillerWP_h 1

//## begin module%3CEA390F01EA.additionalIncludes preserve=no
#include "DMI/Common.h"
#include "DMI/DMI.h"
//## end module%3CEA390F01EA.additionalIncludes

//## begin module%3CEA390F01EA.includes preserve=yes
//## end module%3CEA390F01EA.includes

// MSFiller
#include "UVD/MSFiller.h"
// WorkProcess
#include "OCTOPUSSY/WorkProcess.h"
//## begin module%3CEA390F01EA.declarations preserve=no
//## end module%3CEA390F01EA.declarations

//## begin module%3CEA390F01EA.additionalDeclarations preserve=yes
#pragma aid MSFillerWP
//## end module%3CEA390F01EA.additionalDeclarations


//## begin MSFillerWP%3CEA38B303B4.preface preserve=yes
//## end MSFillerWP%3CEA38B303B4.preface

//## Class: MSFillerWP%3CEA38B303B4
//## Category: UVD%3CD130710240
//## Subsystem: UVD%3CD133E2028D
//## Persistence: Transient
//## Cardinality/Multiplicity: n



class MSFillerWP : public WorkProcess  //## Inherits: <unnamed>%3CEA38BC037B
{
  //## begin MSFillerWP%3CEA38B303B4.initialDeclarations preserve=yes
  //## end MSFillerWP%3CEA38B303B4.initialDeclarations

  public:
    //## Constructors (generated)
      MSFillerWP();

    //## Destructor (generated)
      ~MSFillerWP();


    //## Other Operations (specified)
      //## Operation: setHeader%3CEA38D802A9
      void setHeader (const HIID &id);

      //## Operation: setSegmentHeader%3CF60D490192
      void setSegmentHeader (const HIID& id);

      //## Operation: setChunk%3CF60D5602F0
      void setChunk (const HIID& id);

      //## Operation: setFooter%3CF6215402A9
      void setFooter (const HIID &id);

      //## Operation: setMSName%3CF60D670394
      void setMSName (const string &ms);

      //## Operation: init%3CEA38C50374
      virtual void init ();

      //## Operation: receive%3CEA38CD00E0
      virtual int receive (MessageRef &mref);

    // Additional Public Declarations
      //## begin MSFillerWP%3CEA38B303B4.public preserve=yes
      //## end MSFillerWP%3CEA38B303B4.public

  protected:
    // Additional Protected Declarations
      //## begin MSFillerWP%3CEA38B303B4.protected preserve=yes
      //## end MSFillerWP%3CEA38B303B4.protected

  private:
    //## Constructors (generated)
      MSFillerWP(const MSFillerWP &right);

    //## Assignment Operation (generated)
      MSFillerWP & operator=(const MSFillerWP &right);

    // Additional Private Declarations
      //## begin MSFillerWP%3CEA38B303B4.private preserve=yes
      //## end MSFillerWP%3CEA38B303B4.private

  private: //## implementation
    // Data Members for Associations

      //## Association: UVD::<unnamed>%3CEA4AD80223
      //## Role: MSFillerWP::filler%3CEA4AD90075
      //## begin MSFillerWP::filler%3CEA4AD90075.role preserve=no  private: MSFiller { -> 1VHgN}
      MSFiller filler;
      //## end MSFillerWP::filler%3CEA4AD90075.role

    // Additional Implementation Declarations
      //## begin MSFillerWP%3CEA38B303B4.implementation preserve=yes
      typedef enum {IDLE=0,DATA=1} State;
      
      HIID hdr_id,chunk_hdr_id,chunk_id,footer_id;
      string msname,current_ms;
      //## end MSFillerWP%3CEA38B303B4.implementation
};

//## begin MSFillerWP%3CEA38B303B4.postscript preserve=yes
//## end MSFillerWP%3CEA38B303B4.postscript

// Class MSFillerWP 

//## begin module%3CEA390F01EA.epilog preserve=yes
//## end module%3CEA390F01EA.epilog


#endif
