#ifndef MSReader_h
#define MSReader_h 1

#include "DMI/Common.h"
#include "DMI/DMI.h"

#include "DMI/DataRecord.h"
#include <aips/MeasurementSets.h>


//##ModelId=3CF237080203
class MSReader 
{
  public:
    //##ModelId=3CF237080217
      MSReader();

      //##ModelId=3DB9388F01AB
      MSReader (const MeasurementSet &ms);

    //##ModelId=3DB9388F01DD
      ~MSReader();


      //##ModelId=3CF4A1F100DC
      void attach (const MeasurementSet &ms);

      //##ModelId=3CF4AC140048
      void makeHeader (DataRecord &hdr);

  private:
    //##ModelId=3DB9388F01F1
      MSReader(const MSReader &right);

    //##ModelId=3DB9388F024B
      MSReader & operator=(const MSReader &right);

  private:
    // Additional Implementation Declarations
    //##ModelId=3DB9388F016F
      MeasurementSet ms_;
  
    //##ModelId=3DB9388F02A5
      void readAntenna        (DataRecord &hdr);
    //##ModelId=3DB9388F031D
      void readSource         (DataRecord &hdr);
    //##ModelId=3DB9388F0396
      void readSpectralWindow (DataRecord &hdr);
    //##ModelId=3DB93890003A
      void readField          (DataRecord &hdr);
    //##ModelId=3DB9389000E4
      void readFeed           (DataRecord &hdr);
};

// Class MSReader 


#endif
