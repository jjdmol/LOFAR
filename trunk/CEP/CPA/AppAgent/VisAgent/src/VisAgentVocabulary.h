#ifndef _VISAGENT_VISAGENTVOCABULARY_H
#define _VISAGENT_VISAGENTVOCABULARY_H

#include <VisAgent/AID-VisAgent.h>
    
#pragma aidgroup VisAgent
#pragma aid Vis Input Output Agent Parameters Header Tile Suspend Resume
    
    
namespace VisAgent
{
  const HIID 
      FInputParams    = AidVis|AidInput|AidParameters,
      FOutputParams   = AidVis|AidOutput|AidParameters,
  
      HeaderEvent     = AidVis|AidHeader,
      TileEvent       = AidVis|AidTile,
      SuspendEvent    = AidVis|AidSuspend,
      ResumeEvent     = AidVis|AidResume;
  
};
    
#endif
