#ifndef MSFillerWP_h
#define MSFillerWP_h 1

#include "DMI/Common.h"
#include "DMI/DMI.h"

// MSFiller
#include "UVD/MSFiller.h"
// WorkProcess
#include "OCTOPUSSY/WorkProcess.h"
#pragma aid MSFillerWP


//##ModelId=3CEA38B303B4
class MSFillerWP : public WorkProcess
{
  public:
    //##ModelId=3DB93884015F
      MSFillerWP();

    //##ModelId=3DB938840187
      ~MSFillerWP();


      //##ModelId=3CEA38D802A9
      void setHeader (const HIID &id);

      //##ModelId=3CF60D490192
      void setSegmentHeader (const HIID& id);

      //##ModelId=3CF60D5602F0
      void setChunk (const HIID& id);

      //##ModelId=3CF6215402A9
      void setFooter (const HIID &id);

      //##ModelId=3CF60D670394
      void setMSName (const string &ms);

      //##ModelId=3CEA38C50374
      virtual void init ();

      //##ModelId=3CEA38CD00E0
      virtual int receive (MessageRef &mref);

  private:
    //##ModelId=3DB9388401EB
      MSFillerWP(const MSFillerWP &right);

    //##ModelId=3DB938840295
      MSFillerWP & operator=(const MSFillerWP &right);

  private:
    // Data Members for Associations

      //##ModelId=3CEA4AD90075
      MSFiller filler;

    // Additional Implementation Declarations
    //##ModelId=3DB9387D0397
      typedef enum {IDLE=0,DATA=1} State;
      
    //##ModelId=3DB93883038F
      HIID hdr_id,chunk_hdr_id,chunk_id,footer_id;
    //##ModelId=3DB938840078
      string msname,current_ms;
};

// Class MSFillerWP 


#endif
