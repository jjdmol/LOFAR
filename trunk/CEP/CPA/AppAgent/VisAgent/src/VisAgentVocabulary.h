#ifndef _VISAGENT_VISAGENTVOCABULARY_H
#define _VISAGENT_VISAGENTVOCABULARY_H

#include <AppAgent/FileSink.h>    
#include <VisAgent/AID-VisAgent.h>
#include <VisAgent/DataStreamMap.h>
#include <VisCube/VisVocabulary.h>
    
#pragma aidgroup VisAgent
#pragma aid Vis Input Output Agent Parameters 
#pragma aid Data Header Footer Tile Suspend Resume
    
    
namespace VisAgent
{
  using namespace AppState;
  
  using VisVocabulary::FVDSID;
  
  const AtomicID VisEventPrefix = AidVis;
  
  const HIID  _VisEventMask = VisEventPrefix|AidWildcard;

  inline const HIID & VisEventMask () 
  { return _VisEventMask; }
  
  inline HIID VisEventHIID (int type,const HIID &instance)
  { return VisEventPrefix|AtomicID(-type)|instance; }
  
  inline HIID VisEventMask (int type)
  { return VisEventHIID(type,AidWildcard); }
  
  inline int VisEventType  (const HIID &event)
  { return -( event[1].id() ); }
  
  inline HIID VisEventInstance (const HIID &event)
  { return event.subId(2); }
  
  const HIID 
       // suspend/resume events
       SuspendEvent    = AidVis|AidSuspend,
       ResumeEvent     = AidVis|AidResume;
      
  extern DataStreamMap datamap_VisAgent;
  void datamap_VisAgent_init ();
  
  inline string codeToString (int code)
  { return AtomicID(-code).toString(); }
};
    
#endif
