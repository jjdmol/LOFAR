#ifndef MSFiller_h
#define MSFiller_h 1

#include "DMI/Common.h"
#include "DMI/DMI.h"

#include "DMI/HIID.h"
#include "DMI/DataRecord.h"
#include <aips/MeasurementSets.h>


//##ModelId=3CEA4A9E0070
class MSFiller 
{
  public:
      //##ModelId=3CEB5F5A027D
      MSFiller (const string &msname = "", const DataRecord &hdr = DataRecord());

    //##ModelId=3DB9387F02EB
      ~MSFiller();


      //##ModelId=3CEA4AFB02B9
      bool create (const string &msname, const DataRecord &hdr);

      //##ModelId=3CEB5AA201D5
      bool startSegment (const DataRecord &hdr);

      //##ModelId=3CEB5E3E00C6
      bool addChunk (const DataRecord &rec);

      //##ModelId=3CEB5E4F01CF
      bool endSegment ();

      //##ModelId=3CECBBD102DA
      void close ();

  private:
    //##ModelId=3DB9387F0313
      MSFiller(const MSFiller &right);

    //##ModelId=3DB938800011
      MSFiller & operator=(const MSFiller &right);

    // Additional Private Declarations
    //##ModelId=3DB9388102D7
      void allocateDDI (int spwid,const vector<int> &corrs);
      
    //##ModelId=3DB938820075
      void fillAntenna (const DataRecord &rec);
    //##ModelId=3DB9388201A2
      void fillFeed    (const DataRecord &rec);
    //##ModelId=3DB938820379
      void fillSource  (const DataRecord &rec);
    //##ModelId=3DB9388300D1
      void fillField   (const DataRecord &rec);
    //##ModelId=3DB938830225
      void fillSpectralWindow (const DataRecord &rec);
  private:
    // Additional Implementation Declarations
    //##ModelId=3DB9387E00C7
      MeasurementSet ms_;
    //##ModelId=3DB9387E013F
      MSMainColumns *mscol_;
    //##ModelId=3DB9387E01CC
      MSPolarization mspol_;
    //##ModelId=3DB9387E024E
      MSPolarizationColumns *mspolCol_;
    //##ModelId=3DB9387E02BC
      MSDataDescription msdd_;
    //##ModelId=3DB9387E03D0
      MSDataDescColumns *msddCol_;
      
      // MS header
    //##ModelId=3DB9387F0088
      DataRecord::Ref header_;
      
      // map of correlation types to POLARIZATION ids
    //##ModelId=3DB9387D0256
      typedef map<int,int> PolMap;
    //##ModelId=3DB9387D0292
      typedef PolMap::iterator PMI;
    //##ModelId=3DB9387D02CE
      typedef PolMap::const_iterator CPMI;
    //##ModelId=3DB9387F0115
      PolMap polmap;

      // map of correlation types to DDIs
    //##ModelId=3DB9387D0301
      typedef map<int,int> DDIMap;
    //##ModelId=3DB9387D0333
      typedef DDIMap::iterator DDMI;
    //##ModelId=3DB9387D0365
      typedef DDIMap::const_iterator CDDMI;
    //##ModelId=3DB9387F015B
      DDIMap ddimap;
          
    //##ModelId=3DB9387F01AA
      int field_id,num_channels;
      
};

// Class MSFiller 


#endif
