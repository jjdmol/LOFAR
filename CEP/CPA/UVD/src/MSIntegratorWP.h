//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3CD133DD022C.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3CD133DD022C.cm

//## begin module%3CD133DD022C.cp preserve=no
//## end module%3CD133DD022C.cp

//## Module: MSIntegratorWP%3CD133DD022C; Package specification
//## Subsystem: UVD%3CD133E2028D
//## Source file: F:\lofar8\oms\LOFAR\src-links\UVD\MSIntegratorWP.h

#ifndef MSIntegratorWP_h
#define MSIntegratorWP_h 1

//## begin module%3CD133DD022C.additionalIncludes preserve=no
#include "DMI/Common.h"
#include "DMI/DMI.h"
//## end module%3CD133DD022C.additionalIncludes

//## begin module%3CD133DD022C.includes preserve=yes
#include "UVD/UVD.h"
#include "DMI/DataRecord.h"
//## end module%3CD133DD022C.includes

// WorkProcess
#include "OCTOPUSSY/WorkProcess.h"
//## begin module%3CD133DD022C.declarations preserve=no
//## end module%3CD133DD022C.declarations

//## begin module%3CD133DD022C.additionalDeclarations preserve=yes
#pragma aid MSIntegratorWP 
#pragma aid Control MS Integrate

const HIID MSIntegrate = AidControl|AidMS|AidIntegrate;

class MeasurementSet;
class ROVisibilityIterator;
class VisBuffer;
//## end module%3CD133DD022C.additionalDeclarations


//## begin MSIntegratorWP%3CD133700076.preface preserve=yes
//## end MSIntegratorWP%3CD133700076.preface

//## Class: MSIntegratorWP%3CD133700076
//## Category: UVD%3CD130710240
//## Subsystem: UVD%3CD133E2028D
//## Persistence: Transient
//## Cardinality/Multiplicity: n



class MSIntegratorWP : public WorkProcess  //## Inherits: <unnamed>%3CD1338701E2
{
  //## begin MSIntegratorWP%3CD133700076.initialDeclarations preserve=yes
  //## end MSIntegratorWP%3CD133700076.initialDeclarations

  public:
    //## Constructors (generated)
      MSIntegratorWP();

    //## Constructors (specified)
      //## Operation: MSIntegratorWP%3CD781CA01B8
      MSIntegratorWP (string msname = "", const HIID &act_msg = HIID(), int nchan = 8, int ntime = 10, int npatch = 2);

    //## Destructor (generated)
      ~MSIntegratorWP();


    //## Other Operations (specified)
      //## Operation: init%3CD133A303B9
      virtual void init ();

      //## Operation: start%3CD1339B0349
      virtual bool start ();

      //## Operation: receive%3CD133AB011C
      virtual int receive (MessageRef &mref);

    // Additional Public Declarations
      //## begin MSIntegratorWP%3CD133700076.public preserve=yes
      //## end MSIntegratorWP%3CD133700076.public

  protected:
    // Additional Protected Declarations
      //## begin MSIntegratorWP%3CD133700076.protected preserve=yes
      //## end MSIntegratorWP%3CD133700076.protected

  private:
    //## Constructors (generated)
      MSIntegratorWP(const MSIntegratorWP &right);

    //## Assignment Operation (generated)
      MSIntegratorWP & operator=(const MSIntegratorWP &right);

    // Additional Private Declarations
      //## begin MSIntegratorWP%3CD133700076.private preserve=yes
      //## end MSIntegratorWP%3CD133700076.private

  private: //## implementation
    // Additional Implementation Declarations
      //## begin MSIntegratorWP%3CD133700076.implementation preserve=yes
      bool initMS (const Message &msg,MeasurementSet &ms);
      void initSegment (ROVisibilityIterator &vi);
      void finishIntegration (bool reset=True);
      void integrate (ROVisibilityIterator &vi,VisBuffer &vb);

      string auto_ms;
      int auto_nchan,auto_ntime,auto_npatch;
      HIID auto_activate;
            
      string msname;
      
      int window_chan,window_time,num_patches,
          num_antennas,num_times,num_ifrs,num_channels,num_corrs,
          num_integrated_channels,num_integrated_times,
          msid,nchunk,
          count_time_integrations,integrated_timeslot,
          *pnumtimes;
      
      vector<int*> pnumpixels;
      vector<dcomplex*> pdata;
      
      HIID chunk_hiid;
      vector<int> corrtype,recpt1,recpt2;
      
      DataRecord::Ref href,shrec_ref;
      DataRecord *shrec;
      vector<Message::Ref> patch_header;
      vector<Message::Ref> patch_footer;
      vector<DataRecord::Ref> accrec0;
          
      double integrated_time,*pexp,*puvw;
      
      //## end MSIntegratorWP%3CD133700076.implementation
};

//## begin MSIntegratorWP%3CD133700076.postscript preserve=yes
//## end MSIntegratorWP%3CD133700076.postscript

// Class MSIntegratorWP 

//## begin module%3CD133DD022C.epilog preserve=yes
//## end module%3CD133DD022C.epilog


#endif
