[+ AutoGen5 template ph cc +]
//
[+ (dne "//  ") +][+ (out-push-add "/dev/null") +]
[+ DEFINE prefix_cap +][+ IF (exist? "prefix") +][+ (get "prefix") +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE prefix_ucase +][+ IF (exist? "prefix") +][+ (string-upcase (get "prefix")) +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE protocol_id +][+ IF (exist? "id") +][+ (get "id") +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE signal_name +][+ prefix_ucase +]_[+ (get "signal") +][+ ENDDEF +]
[+ DEFINE signal_id +][+ signal_name +]_ID[+ ENDDEF +]
[+ DEFINE cap_signal +][+ (string-substitute (string-capitalize! (get "signal")) '( "_" )' ( "" )) +][+ ENDDEF +]
[+ DEFINE event_class_name +][+ prefix_cap +][+ cap_signal +]Event[+ ENDDEF +]
[+ DEFINE event_class_decl +][+ event_class_name +] : public GCF::TM::GCFEvent[+ ENDDEF +]
[+ DEFINE protocol_name +][+ (string-upcase (base-name)) +][+ ENDDEF +]
[+ DEFINE event_class_member_type +][+ IF (*== (get "type") "]") +][+ (substring (get "type") 0 (string-index (get "type") #\[)) +][+ ELSE +][+ (get "type") +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE event_class_member +][+ event_class_member_type +][+ IF (*== (get "type") "[]") +]*[+ ENDIF +] [+ (get "name") +][+ IF (and (*== (get "type") "]") (not (*== (get "type") "[]"))) +][+ (substring (get "type") (string-index (get "type") #\[) (string-length (get "type"))) +][+ ENDIF +][+ ENDDEF +]

[+ (out-pop) +]
//
//  [+ (base-name) +].[+ (suffix) +]: [+ description +]
//
//  Copyright (C) 2003
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
[+ (lgpl "This program" "ASTRON" "//  ") +]
//
//  $Id$
//
[+ IF (== (suffix) "cc") +]
#include "[+ (base-name) +].ph"

using namespace LOFAR::GCF::TM;

const char* LOFAR::[+ (base-name) +]::[+ protocol_name +]_signalnames[] = 
{
  "[+ protocol_name +]: invalid signal",[+ FOR event "," +]
  "[+ signal_name +] ([+ (get "dir") +])"[+ ENDFOR +]
};
[+ ELSE +]
#ifndef [+ protocol_name +]_H
#define [+ protocol_name +]_H

[+ FOR include "" +]
#include [+ (get "include") +][+ ENDFOR +]
#include <GCF/TM/GCF_Protocols.h>
#include <string>

namespace LOFAR
{
	namespace [+ (base-name) +]
	{

[+ (get "prelude") +]

//
// Define protocol ID
//
enum 
{
  [+ protocol_name +] = [+ protocol_id +]
};

//
// Define protocol message types
//
enum 
{ [+ FOR event "," +]
  [+ signal_id +][+ IF (= 0 (for-index)) +] = 1[+ ENDIF +][+ ENDFOR event +]
};

[+ FOR event "" +] 
#define [+ prefix_ucase +]_[+ (get "signal") +] F_SIGNAL([+ protocol_name +], [+ prefix_ucase +]_[+ (get "signal") +]_ID, F_[+ (get "dir")+])[+ ENDFOR event +]

extern const char* [+ protocol_name +]_signalnames[];

[+ ENDIF +]
[+ FOR event "" +][+ IF (= (suffix) "ph") +][+ FOR param "" +]
[+ IF (*== (get "type") "&") +][+ (error "reference types not supported") +][+ ENDIF +]
[+ IF (and (==* (get "type") "string") (> (string-length (get "type")) 6)) +][+ (error "only scalar 'string' is supported") +][+ ENDIF +][+ ENDFOR +]
  class [+ event_class_decl +]
  {
    public:
      [+ event_class_name +](GCF::TM::GCFEvent& e);
      [+ event_class_name +]();
      virtual ~[+ event_class_name +]();

      [+ FOR param ";" +]
      [+ IF (== (get "type") "string") +]std::[+ ENDIF +][+ event_class_member +][+ IF (*== (get "type") "[]") +]; uint32 [+ (get "name") +]NOE[+ ENDIF +][+ ENDFOR +];

      void* pack(uint32& __packsize);

    private:
      [+ event_class_name +]([+ event_class_name +]&);
      [+ event_class_name +]& operator= (const [+ event_class_name +]&);
      
	    void unpack();
  };   [+ ELSE +]
[+ event_class_name +]::[+ event_class_name +](GCF::TM::GCFEvent& e)
  : GCF::TM::GCFEvent(e)[+ FOR param "" +][+ IF (or (*== (get "type") "[]") (*== (get "type") "*")) +],
    [+ (get "name") +](0)[+ ENDIF +][+ IF (*== (get "type") "[]") +],
    [+ (get "name") +]NOE(0)[+ ENDIF +][+ ENDFOR +]
{
	unpack();
}
      
[+ event_class_name +]::[+ event_class_name +]()
  : GCF::TM::GCFEvent([+ signal_name +])[+ FOR param "" +][+ IF (or (*== (get "type") "[]") (*== (get "type") "*")) +],
    [+ (get "name") +](0)[+ ENDIF +][+ IF (*== (get "type") "[]") +],
    [+ (get "name") +]NOE(0)[+ ENDIF +][+ ENDFOR +]
{        
}

[+ event_class_name +]::~[+ event_class_name +]() 
{
  if (_unpackDone)
  {[+ FOR param "" +][+ IF (and (exist? "userdefined") (*== (get "type") "*")) +]
    if ([+ (get "name") +]) delete [+ (get "name") +];[+ ENDIF +][+ ENDFOR +]
  }
}
    
void* [+ event_class_name +]::pack(uint32& __packsize)
{
  [+ FOR param "" +][+ IF (or (*== (get "type") "[]") (*== (get "type") "*")) +][+ IF (*== (get "type") "[]") +]if ([+ (get "name") +]NOE > 0) [+ ENDIF +]assert([+ (get "name") +]);[+ ENDIF +]
  [+ ENDFOR +]
  uint32 __requiredSize = [+ IF (not (exist? "noheader")) +]sizeof(signal) + sizeof(length)[+ ELSE +]0[+ ENDIF +][+ FOR param "" +]
    [+ IF (exist? "userdefined") +]+ [+ (get "name") +][+ IF (*== (get "type") "*") +]->[+ ELSE +].[+ ENDIF +]getSize()
    [+ ELIF (not (*== (get "type") "]")) +]+ [+ IF (== (get "type") "string") +][+ (get "name") +].length() + sizeof(uint16)[+ ELSE +]sizeof([+ (get "name") +])[+ ENDIF+]
    [+ ELIF (*== (get "type") "[]") +]+ sizeof([+ (get "name") +]NOE) + ([+ (get "name") +]NOE * sizeof([+ event_class_member_type +]))
    [+ ELSE +]+ sizeof([+ (get "name") +])[+ ENDIF +][+ ENDFOR +];

  resizeBuf(__requiredSize);
  uint32 __offset = __packsize = 0;
  [+ IF (not (exist? "noheader")) +]
  GCF::TM::GCFEvent::pack(__offset);[+ ENDIF +]
  [+ FOR param "" +]
  [+ IF (exist? "userdefined") +]
  __offset += [+ (get "name") +][+ IF (*== (get "type") "*") +]->[+ ELSE +].[+ ENDIF +]pack(_buffer + __offset);
  [+ ELIF (not (*== (get "type") "]")) +]
    [+ IF (== (get "type") "string") +]
  __offset += packString(_buffer + __offset, [+ (get "name") +]);
    [+ ELSE +]
  memcpy(_buffer + __offset, &[+ (get "name") +], sizeof([+ (get "type") +]));
  __offset += sizeof([+ (get "type") +]);
    [+ ENDIF +]
  [+ ELIF (*== (get "type") "[]") +]
  __offset += packMember(__offset, [+ (get "name") +], [+ (get "name") +]NOE, sizeof([+ event_class_member_type +]));
  [+ ELSE +]
  memcpy(_buffer + __offset, [+ (get "name") +], sizeof([+ (get "name") +]));
  __offset += sizeof([+ (get "name") +]);
  [+ ENDIF +][+ ENDFOR +]
	[+ IF (= (count "param") 0) +]
  // no params in this event to pack
	[+ ENDIF +]
          
  __packsize = __offset;
  return _buffer;
}

void [+ event_class_name +]::unpack()
{
  if (length > 0)
  {
  	[+ IF (> (count "param") 0) +]
  	uint32 __offset = sizeof(GCF::TM::GCFEvent);
    char* __data = (char*) _base;
    [+ ELSE +]
    // no params in this event to unpack
    [+ ENDIF +]
    [+ FOR param "" +]
    [+ IF (exist? "userdefined") +]
      [+ IF (*== (get "type") "*") +]
    [+ (get "name") +] = new [+ (substring (get "type") 0 (string-index (get "type") #\*)) +]();
      [+ ENDIF +]
    __offset += [+ (get "name") +][+ IF (*== (get "type") "*") +]->[+ ELSE +].[+ ENDIF +]unpack(__data + __offset);
    [+ ELIF (not (*== (get "type") "]")) +]
      [+ IF (== (get "type") "string") +]
    __offset += GCF::TM::GCFEvent::unpackString([+ (get "name") +], __data + __offset);
      [+ ELSE +]
    memcpy(&[+ (get "name") +], __data + __offset, sizeof([+ (get "type") +]));
    __offset += sizeof([+ (get "type") +]);
      [+ ENDIF +]
    [+ ELIF (*== (get "type") "[]") +]
    [+ (get "name") +] = ([+ event_class_member_type +]*) unpackMember(__data, __offset, [+ (get "name") +]NOE,  sizeof([+ event_class_member_type +]));
    [+ ELSE +]
    memcpy([+ (get "name") +], (__data + __offset), sizeof([+ (get "name") +]));
    __offset += sizeof([+ (get "name") +]);
    [+ ENDIF +][+ ENDFOR +]
  }
}[+ ENDIF +][+ ENDFOR +]
[+ IF (= (suffix) "ph") +]
	} // namespace [+ (base-name) +]
} // namespace LOFAR


using namespace LOFAR::[+ (base-name) +];

#endif
[+ ENDIF +]
