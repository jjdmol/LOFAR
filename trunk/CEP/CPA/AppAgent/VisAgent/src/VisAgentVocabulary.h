#ifndef _VISAGENT_VISAGENTVOCABULARY_H
#define _VISAGENT_VISAGENTVOCABULARY_H

#include <AppAgent/FileSink.h>    
#include <VisAgent/AID-VisAgent.h>
#include <VisAgent/DataStreamMap.h>
    
#pragma aidgroup VisAgent
#pragma aid Vis Input Output Agent Parameters 
#pragma aid Data Header Footer Tile Suspend Resume
    
    
namespace VisAgent
{
  using namespace AppState;
  
  const HIID 
      // parameter record fields
      FInputParams    = AidVis|AidInput|AidParameters,
      FOutputParams   = AidVis|AidOutput|AidParameters,
  
      // data events
      DataPrefix      = AidVis|AidData,
        HeaderEvent   =   DataPrefix|AidHeader,
        TileEvent     =   DataPrefix|AidTile,
        FooterEvent   =   DataPrefix|AidFooter,
      DataEventMask    = DataPrefix|AidWildcard,
      
      // suspend/resume events
      SuspendEvent    = AidVis|AidSuspend,
      ResumeEvent     = AidVis|AidResume;
      
  extern DataStreamMap datamap_VisAgent;
  void datamap_VisAgent_init ();
  
// //##ModelId=3EB242520022
//   typedef enum
//   {
//     // additional return codes indicate type of returned object
//     HEADER   =     -AidHeader_int,   // received header
//     TILE     =     -AidTile_int,     // received tile
//     FOOTER   =     -AidFooter_int,   // received footer
// 
//   } VisAgent_DataEventCodes;
//   
  inline string codeToString (int code)
  { return AtomicID(-code).toString(); }
};
    
#endif
