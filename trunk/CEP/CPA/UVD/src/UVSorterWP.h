#ifndef UVSorterWP_h
#define UVSorterWP_h 1

#include "DMI/Common.h"
#include "DMI/DMI.h"

#pragma aid UVSorterWP

// WorkProcess
#include "OCTOPUSSY/WorkProcess.h"

//##ModelId=3CD79D3D0227
class UVSorterWP : public WorkProcess
{
  public:
      //##ModelId=3CD7CF6E020D
      UVSorterWP (int ipatch, int icorr);

    //##ModelId=3DB93892008E
      ~UVSorterWP();


      //##ModelId=3CD79D680391
      virtual void init ();

      //##ModelId=3CD79D7301B7
      virtual int receive (MessageRef &mref);

    // Additional Public Declarations
    //##ModelId=3DB9387E0027
      typedef enum {
        IDLE = 0,
        SORTING = 1
      } State;

  private:
    //##ModelId=3DB9389200F2
      UVSorterWP();

    //##ModelId=3DB93892014C
      UVSorterWP(const UVSorterWP &right);

    //##ModelId=3DB938920228
      UVSorterWP & operator=(const UVSorterWP &right);

  private:
    // Additional Implementation Declarations
    //##ModelId=3DB9389001CB
      Timestamp ts_header;
    //##ModelId=3DB9389001E8
      int vis_count;
  
    //##ModelId=3DB93890021B
      HIID header_hiid,chunk_hiid,footer_hiid,msfooter_hiid;
    //##ModelId=3DB9389002BB
      int mypatch,mycorr;
      
    //##ModelId=3DB93890035B
      int uvset_id,segment_id,num_ifrs,num_times,num_channels;
      
    //##ModelId=3DB93891019B
      ObjRef header_ref;
    //##ModelId=3DB938910213
      ObjRef prec_template_ref;
    //##ModelId=3DB938910277
      vector<DataRecord *> prec;
    //##ModelId=3DB9389102EF
      vector<ObjRef> prec_ref;
    //##ModelId=3DB938910371
      vector<dcomplex *> pdata;
    //##ModelId=3DB93892000B
      vector<int *> pnumpoints; 
      

};

// Class UVSorterWP 


#endif
