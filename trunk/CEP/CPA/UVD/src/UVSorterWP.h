//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3CD79DB900E5.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3CD79DB900E5.cm

//## begin module%3CD79DB900E5.cp preserve=no
//## end module%3CD79DB900E5.cp

//## Module: UVSorterWP%3CD79DB900E5; Package specification
//## Subsystem: UVD%3CD133E2028D
//## Source file: F:\lofar8\oms\LOFAR\src-links\UVD\UVSorterWP.h

#ifndef UVSorterWP_h
#define UVSorterWP_h 1

//## begin module%3CD79DB900E5.additionalIncludes preserve=no
#include "DMI/Common.h"
#include "DMI/DMI.h"
//## end module%3CD79DB900E5.additionalIncludes

//## begin module%3CD79DB900E5.includes preserve=yes
#pragma aid UVSorterWP
//## end module%3CD79DB900E5.includes

// WorkProcess
#include "OCTOPUSSY/WorkProcess.h"
//## begin module%3CD79DB900E5.declarations preserve=no
//## end module%3CD79DB900E5.declarations

//## begin module%3CD79DB900E5.additionalDeclarations preserve=yes
//## end module%3CD79DB900E5.additionalDeclarations


//## begin UVSorterWP%3CD79D3D0227.preface preserve=yes
//## end UVSorterWP%3CD79D3D0227.preface

//## Class: UVSorterWP%3CD79D3D0227
//## Category: UVD%3CD130710240
//## Subsystem: UVD%3CD133E2028D
//## Persistence: Transient
//## Cardinality/Multiplicity: n



class UVSorterWP : public WorkProcess  //## Inherits: <unnamed>%3CD79D5303AF
{
  //## begin UVSorterWP%3CD79D3D0227.initialDeclarations preserve=yes
  //## end UVSorterWP%3CD79D3D0227.initialDeclarations

  public:
    //## Constructors (specified)
      //## Operation: UVSorterWP%3CD7CF6E020D
      UVSorterWP (int ipatch, int icorr);

    //## Destructor (generated)
      ~UVSorterWP();


    //## Other Operations (specified)
      //## Operation: init%3CD79D680391
      virtual void init ();

      //## Operation: receive%3CD79D7301B7
      virtual int receive (MessageRef &mref);

    // Additional Public Declarations
      //## begin UVSorterWP%3CD79D3D0227.public preserve=yes
      typedef enum {
        IDLE = 0,
        SORTING = 1
      } State;
      //## end UVSorterWP%3CD79D3D0227.public

  protected:
    // Additional Protected Declarations
      //## begin UVSorterWP%3CD79D3D0227.protected preserve=yes
      //## end UVSorterWP%3CD79D3D0227.protected

  private:
    //## Constructors (generated)
      UVSorterWP();

      UVSorterWP(const UVSorterWP &right);

    //## Assignment Operation (generated)
      UVSorterWP & operator=(const UVSorterWP &right);

    // Additional Private Declarations
      //## begin UVSorterWP%3CD79D3D0227.private preserve=yes
      //## end UVSorterWP%3CD79D3D0227.private

  private: //## implementation
    // Additional Implementation Declarations
      //## begin UVSorterWP%3CD79D3D0227.implementation preserve=yes
      Timestamp ts_header;
      int vis_count;
  
      HIID header_hiid,chunk_hiid,footer_hiid;
      int mypatch,mycorr;
      
      int uvset_id,segment_id,num_ifrs,num_times,num_channels;
      
      ObjRef header_ref;
      DataRecord::Ref prec_template_ref;
      vector<DataRecord *> prec;
      vector<DataRecord::Ref> prec_ref;
      vector<dcomplex *> pdata;
      vector<int *> pnumpoints; 
      
      //## end UVSorterWP%3CD79D3D0227.implementation

};

//## begin UVSorterWP%3CD79D3D0227.postscript preserve=yes
//## end UVSorterWP%3CD79D3D0227.postscript

// Class UVSorterWP 

//## begin module%3CD79DB900E5.epilog preserve=yes
//## end module%3CD79DB900E5.epilog


#endif
