#ifndef MSIntegratorWP_h
#define MSIntegratorWP_h 1

#include "DMI/Common.h"
#include "DMI/DMI.h"

#include "UVD/UVD.h"
#include "DMI/DataRecord.h"

// WorkProcess
#include "OCTOPUSSY/WorkProcess.h"
#pragma aid MSIntegratorWP 
#pragma aid Control MS Integrate

const HIID MSIntegrate = AidControl|AidMS|AidIntegrate;

class MeasurementSet;
class ROVisibilityIterator;
class VisBuffer;


//##ModelId=3CD133700076
class MSIntegratorWP : public WorkProcess
{
  public:
  
    //##ModelId=3DB9387D03D3
  typedef fcomplex ComplexType;

      //##ModelId=3CD781CA01B8
      MSIntegratorWP (string msname = "", const HIID &act_msg = HIID(), int nchan = 8, int ntime = 10, int npatch = 2);

    //##ModelId=3DB9388B0277
      ~MSIntegratorWP();


      //##ModelId=3CD133A303B9
      virtual void init ();

      //##ModelId=3CD1339B0349
      virtual bool start ();

      //##ModelId=3CD133AB011C
      virtual int receive (MessageRef &mref);

  private:
    //##ModelId=3DB9388B0390
      MSIntegratorWP(const MSIntegratorWP &right);

    //##ModelId=3DB9388C0142
      MSIntegratorWP & operator=(const MSIntegratorWP &right);

  private:
    // Additional Implementation Declarations
    //##ModelId=3DB9388C02FB
      bool initMS (const Message &msg,MeasurementSet &ms);
    //##ModelId=3DB9388D01F8
      void initSegment (ROVisibilityIterator &vi);
    //##ModelId=3DB9388E0005
      void finishIntegration (bool reset=True);
    //##ModelId=3DB9388E020E
      void integrate (ROVisibilityIterator &vi,VisBuffer &vb);

    //##ModelId=3DB938840385
      string auto_ms;
    //##ModelId=3DB9388403E0
      int auto_nchan,auto_ntime,auto_npatch;
    //##ModelId=3DB93885012F
      HIID auto_activate;
            
    //##ModelId=3DB93885016A
      string msname;
      
    //##ModelId=3DB9388501E2
      bool ignore_flags;
      
    //##ModelId=3DB938850264
      int window_chan,window_time,num_patches,
          num_antennas,num_times,num_ifrs,num_channels,num_corrs,
          num_integrated_channels,num_integrated_times,
          msid,nchunk,
          count_time_integrations,integrated_timeslot,
          *pnumtimes;
      
    //##ModelId=3DB938880101
      vector<int*> pnumpixels;
    //##ModelId=3DB9388801E7
      vector<ComplexType*> pdata;
      
    //##ModelId=3DB9388802BA
      HIID chunk_hiid;
    //##ModelId=3DB938880313
      vector<int> corrtype,recpt1,recpt2;
      
    //##ModelId=3DB9388901D4
      DataRecord::Ref href,shrec_ref;
    //##ModelId=3DB9388903C0
      DataRecord *shrec;
    //##ModelId=3DB9388A003B
      vector<Message::Ref> patch_header;
    //##ModelId=3DB9388A0136
      vector<Message::Ref> patch_footer;
    //##ModelId=3DB9388A023A
      vector<DataRecord::Ref> accrec0;
          
    //##ModelId=3DB9388A032A
      double integrated_time,*pexp,*puvw;
      
};

// Class MSIntegratorWP 


#endif
