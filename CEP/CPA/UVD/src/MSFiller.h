//## begin module%1.4%.codegen_version preserve=yes
//   Read the documentation to learn more about C++ code generator
//   versioning.
//## end module%1.4%.codegen_version

//## begin module%3CEA4ACC018F.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%3CEA4ACC018F.cm

//## begin module%3CEA4ACC018F.cp preserve=no
//## end module%3CEA4ACC018F.cp

//## Module: MSFiller%3CEA4ACC018F; Package specification
//## Subsystem: UVD%3CD133E2028D
//## Source file: F:\lofar8\oms\LOFAR\src-links\UVD\MSFiller.h

#ifndef MSFiller_h
#define MSFiller_h 1

//## begin module%3CEA4ACC018F.additionalIncludes preserve=no
#include "DMI/Common.h"
#include "DMI/DMI.h"
//## end module%3CEA4ACC018F.additionalIncludes

//## begin module%3CEA4ACC018F.includes preserve=yes
#include "DMI/HIID.h"
#include "DMI/DataRecord.h"
#include <aips/MeasurementSets.h>
//## end module%3CEA4ACC018F.includes

//## begin module%3CEA4ACC018F.declarations preserve=no
//## end module%3CEA4ACC018F.declarations

//## begin module%3CEA4ACC018F.additionalDeclarations preserve=yes
//## end module%3CEA4ACC018F.additionalDeclarations


//## begin MSFiller%3CEA4A9E0070.preface preserve=yes
//## end MSFiller%3CEA4A9E0070.preface

//## Class: MSFiller%3CEA4A9E0070
//## Category: UVD%3CD130710240
//## Subsystem: UVD%3CD133E2028D
//## Persistence: Transient
//## Cardinality/Multiplicity: n



class MSFiller 
{
  //## begin MSFiller%3CEA4A9E0070.initialDeclarations preserve=yes
  //## end MSFiller%3CEA4A9E0070.initialDeclarations

  public:
    //## Constructors (specified)
      //## Operation: MSFiller%3CEB5F5A027D
      MSFiller (const string &msname = "", const DataRecord &hdr = DataRecord());

    //## Destructor (generated)
      ~MSFiller();


    //## Other Operations (specified)
      //## Operation: create%3CEA4AFB02B9
      bool create (const string &msname, const DataRecord &hdr);

      //## Operation: startSegment%3CEB5AA201D5
      bool startSegment (const DataRecord &hdr);

      //## Operation: addChunk%3CEB5E3E00C6
      bool addChunk (const DataRecord &rec);

      //## Operation: endSegment%3CEB5E4F01CF
      bool endSegment ();

      //## Operation: close%3CECBBD102DA
      void close ();

    // Additional Public Declarations
      //## begin MSFiller%3CEA4A9E0070.public preserve=yes
      //## end MSFiller%3CEA4A9E0070.public

  protected:
    // Additional Protected Declarations
      //## begin MSFiller%3CEA4A9E0070.protected preserve=yes
      //## end MSFiller%3CEA4A9E0070.protected

  private:
    //## Constructors (generated)
      MSFiller(const MSFiller &right);

    //## Assignment Operation (generated)
      MSFiller & operator=(const MSFiller &right);

    // Additional Private Declarations
      //## begin MSFiller%3CEA4A9E0070.private preserve=yes
      void allocateDDI (int spwid,const vector<int> &corrs);
      
      void fillAntenna (const DataRecord &rec);
      void fillFeed    (const DataRecord &rec);
      void fillSource  (const DataRecord &rec);
      void fillField   (const DataRecord &rec);
      void fillSpectralWindow (const DataRecord &rec);
      //## end MSFiller%3CEA4A9E0070.private
  private: //## implementation
    // Additional Implementation Declarations
      //## begin MSFiller%3CEA4A9E0070.implementation preserve=yes
      MeasurementSet ms_;
      MSMainColumns *mscol_;
      MSPolarization mspol_;
      MSPolarizationColumns *mspolCol_;
      MSDataDescription msdd_;
      MSDataDescColumns *msddCol_;
      
      // MS header
      DataRecord::Ref header_;
      
      // map of correlation types to POLARIZATION ids
      typedef map<int,int> PolMap;
      typedef PolMap::iterator PMI;
      typedef PolMap::const_iterator CPMI;
      PolMap polmap;

      // map of correlation types to DDIs
      typedef map<int,int> DDIMap;
      typedef DDIMap::iterator DDMI;
      typedef DDIMap::const_iterator CDDMI;
      DDIMap ddimap;
          
      int field_id,num_channels;
      
      //## end MSFiller%3CEA4A9E0070.implementation
};

//## begin MSFiller%3CEA4A9E0070.postscript preserve=yes
//## end MSFiller%3CEA4A9E0070.postscript

// Class MSFiller 

//## begin module%3CEA4ACC018F.epilog preserve=yes
//## end module%3CEA4ACC018F.epilog


#endif
