//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3CF2376F01D9.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3CF2376F01D9.cm

//## begin module%3CF2376F01D9.cp preserve=no
//## end module%3CF2376F01D9.cp

//## Module: MSReader%3CF2376F01D9; Package specification
//## Subsystem: UVD%3CD133E2028D
//## Source file: F:\lofar8\oms\LOFAR\src-links\UVD\MSReader.h

#ifndef MSReader_h
#define MSReader_h 1

//## begin module%3CF2376F01D9.additionalIncludes preserve=no
#include "DMI/Common.h"
#include "DMI/DMI.h"
//## end module%3CF2376F01D9.additionalIncludes

//## begin module%3CF2376F01D9.includes preserve=yes
#include "DMI/DataRecord.h"
#include <aips/MeasurementSets.h>
//## end module%3CF2376F01D9.includes

//## begin module%3CF2376F01D9.declarations preserve=no
//## end module%3CF2376F01D9.declarations

//## begin module%3CF2376F01D9.additionalDeclarations preserve=yes
//## end module%3CF2376F01D9.additionalDeclarations


//## begin MSReader%3CF237080203.preface preserve=yes
//## end MSReader%3CF237080203.preface

//## Class: MSReader%3CF237080203
//## Category: UVD%3CD130710240
//## Subsystem: UVD%3CD133E2028D
//## Persistence: Transient
//## Cardinality/Multiplicity: n



class MSReader 
{
  //## begin MSReader%3CF237080203.initialDeclarations preserve=yes
  //## end MSReader%3CF237080203.initialDeclarations

  public:
    //## Constructors (generated)
      MSReader();

    //## Constructors (specified)
      //## Operation: MSReader%3CF237080217
      MSReader (const MeasurementSet &ms);

    //## Destructor (generated)
      ~MSReader();


    //## Other Operations (specified)
      //## Operation: attach%3CF4A1F100DC
      void attach (const MeasurementSet &ms);

      //## Operation: makeHeader%3CF4AC140048
      void makeHeader (DataRecord &hdr);

    // Additional Public Declarations
      //## begin MSReader%3CF237080203.public preserve=yes
      //## end MSReader%3CF237080203.public

  protected:
    // Additional Protected Declarations
      //## begin MSReader%3CF237080203.protected preserve=yes
      //## end MSReader%3CF237080203.protected

  private:
    //## Constructors (generated)
      MSReader(const MSReader &right);

    //## Assignment Operation (generated)
      MSReader & operator=(const MSReader &right);

    // Additional Private Declarations
      //## begin MSReader%3CF237080203.private preserve=yes
      //## end MSReader%3CF237080203.private

  private: //## implementation
    // Additional Implementation Declarations
      //## begin MSReader%3CF237080203.implementation preserve=yes
      MeasurementSet ms_;
  
      void readAntenna        (DataRecord &hdr);
      void readSource         (DataRecord &hdr);
      void readSpectralWindow (DataRecord &hdr);
      void readField          (DataRecord &hdr);
      void readFeed           (DataRecord &hdr);
      //## end MSReader%3CF237080203.implementation
};

//## begin MSReader%3CF237080203.postscript preserve=yes
//## end MSReader%3CF237080203.postscript

// Class MSReader 

//## begin module%3CF2376F01D9.epilog preserve=yes
//## end module%3CF2376F01D9.epilog


#endif
