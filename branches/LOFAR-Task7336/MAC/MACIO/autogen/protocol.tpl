[+ AutoGen5 template ph cc +]
[+ # manual at: http://www.gnu.org/software/autogen/manual/html_node/Function-Index.html#Function-Index +]
//
[+ (dne "//  ") +][+ (out-push-add "/dev/null") +]
[+ DEFINE prefix_cap +][+ IF (exist? "prefix") +][+ (get "prefix") +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE prefix_ucase +][+ IF (exist? "prefix") +][+ (string-upcase (get "prefix")) +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE protocol_id +][+ IF (exist? "id") +][+ (get "id") +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE signal_name +][+ prefix_ucase +]_[+ (get "signal") +][+ ENDDEF +]
[+ DEFINE signal_id +][+ signal_name +]_ID[+ ENDDEF +]
[+ DEFINE cap_signal +][+ (string-substitute (string-capitalize! (get "signal")) '( "_" )' ( "" )) +][+ ENDDEF +]
[+ DEFINE event_class_name +][+ prefix_cap +][+ cap_signal +]Event[+ ENDDEF +]
[+ DEFINE event_class_decl +][+ event_class_name +] : public MACIO::GCFEvent[+ ENDDEF +]
[+ DEFINE protocol_name +][+ (string-upcase (base-name)) +][+ ENDDEF +]
[+ DEFINE project +][+ IF (exist? "project") +][+ (get "project") +][+ ELSE +]LOFAR[+ ENDIF +][+ ENDDEF +]
[+ DEFINE event_class_member_type +][+ IF (*== (get "type") "]") +][+ (substring (get "type") 0 (string-index (get "type") #\[)) +][+ ELSE +][+ (get "type") +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE event_class_member +][+ event_class_member_type +][+ IF (*== (get "type") "[]") +]*[+ ENDIF +] [+ (get "name") +][+ IF (and (*== (get "type") "]") (not (*== (get "type") "[]"))) +][+ (substring (get "type") (string-index (get "type") #\[) (string-length (get "type"))) +][+ ENDIF +][+ ENDDEF +]

[+ (out-pop) +]
//
//  [+ (base-name) +].[+ (suffix) +]: [+ description +]
//
//  Copyright (C) 2003-2012
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
//
[+ IF (== autogen-version "5.12") +]
//  [+ (lgpl "This program" "ASTRON" "//  ") +]
[+ ELSE "5.10 is fine, 5.12 forgets the 1st // (build err) and 5.12 and 5.16 append 2 chars (don't care)." +]
[+ (lgpl "This program" "ASTRON" "//  ") +]
[+ ENDIF +]
//
//  $Id$
//
[+ IF (== (suffix) "cc") +]
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <MACIO/Marshalling.tcc>
#include "[+ (base-name) +].ph"

using namespace LOFAR::MACIO;
[+ IF (exist? "project") +]using namespace LOFAR;[+ ENDIF +]
using namespace std;

[+ FOR Cfunction "" +][+ (get "result")+] [+ project +]::[+ (base-name) +]::[+ (get "name") +] {
[+ (get "code") +]
}[+ ENDFOR +]

const char* [+ project +]::[+ (base-name) +]::[+ protocol_name +]_signalnames[] = 
{
  "[+ protocol_name +]: invalid signal",[+ FOR event "," +]
  "[+ signal_name +]"[+ ENDFOR +]
};

const char* [+ protocol_name +]_errornames[] = 
{ [+ FOR error "," +]
	"[+ (get "msg") +]"[+ ENDFOR error +]
};

const struct protocolStrings		[+ project +]::[+ (base-name) +]::[+ protocol-name +]_STRINGS = {
	[+ (count "event") +]+1, [+ (count "error") +], 
	[+ project +]::[+ (base-name) +]::[+ protocol_name +]_signalnames,
	[+ protocol_name +]_errornames
};
[+ ELSE +]
#ifndef [+ protocol_name +]_H
#define [+ protocol_name +]_H

#include <lofar_config.h>
[+ FOR include "" +]
#include [+ (get "include") +][+ ENDFOR +]
#include <MACIO/ProtocolDefs.h>
#include <Common/LofarTypes.h>
#include <string>
#include <cstring>

namespace [+ project +]
{
	namespace [+ (base-name) +]
	{

[+ (get "prelude") +]

[+ FOR Cfunction "" +][+ (get "result")+] [+ (get "name") +];
[+ ENDFOR +]

//
// Define protocol ID
//
enum 
{
  [+ protocol_name +] = [+ protocol_id +]
};

//
// Define error numbers and names
//
enum
{ [+ FOR error "," +]
	[+ prefix_ucase +]_[+ (get "id") +]_ERR[+ IF (= 0 (for-index)) +] = F_ERROR([+ protocol_name +], [+ (for-index) +])[+ ENDIF +][+ ENDFOR error +]
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

extern const char* [+ protocol_name +]_signalnames[];  // for backwards compatibility 
extern const struct LOFAR::MACIO::protocolStrings [+ protocol-name +]_STRINGS;

[+ ENDIF +]
[+ FOR event "" +][+ IF (= (suffix) "ph") +][+ FOR param "" +]
[+ IF (*== (get "type") "&") +][+ (error "reference types not supported") +][+ ENDIF +]
[+ IF (and (==* (get "type") "string") (> (string-length (get "type")) 6)) +][+ (error "only scalar 'string' is supported") +][+ ENDIF +][+ ENDFOR +]
  class [+ event_class_decl +]
  {
    public:
      [+ event_class_name +](MACIO::GCFEvent& e);
      [+ event_class_name +]();
      virtual ~[+ event_class_name +]();

      [+ FOR param ";" +]
      [+ IF (== (get "type") "string") +]std::[+ ENDIF +][+ event_class_member +][+ IF (*== (get "type") "[]") +]; uint32 [+ (get "name") +]NOE[+ ENDIF +][+ ENDFOR +];

      void pack();
      virtual [+ event_class_name +]*	clone();
      ostream& print (ostream& os) const;

    private:
      [+ event_class_name +]([+ event_class_name +]&);
      [+ event_class_name +]& operator= (const [+ event_class_name +]&);
      
	  void unpack();
  };  

inline ostream& operator<<(ostream& os, const [+ event_class_name +]& event)
{
    return (event.print(os));
};
 [+ ELSE +]
[+ event_class_name +]::[+ event_class_name +](MACIO::GCFEvent& e)
  : MACIO::GCFEvent(e)[+ FOR param "" +][+ IF (or (*== (get "type") "[]") (*== (get "type") "*")) +],
    [+ (get "name") +](0)[+ ENDIF +][+ IF (*== (get "type") "[]") +],
    [+ (get "name") +]NOE(0)[+ ENDIF +][+ ENDFOR +]
{
	unpack();
}
      
[+ event_class_name +]::[+ event_class_name +]()
  : MACIO::GCFEvent([+ signal_name +])[+ FOR param "" +][+ IF (or (*== (get "type") "[]") (*== (get "type") "*")) +],
    [+ (get "name") +](0)[+ ENDIF +][+ IF (*== (get "type") "[]") +],
    [+ (get "name") +]NOE(0)[+ ENDIF +][+ ENDFOR +]
{        
}

[+ event_class_name +]::~[+ event_class_name +]() 
{
  [+ FOR param "" +][+ IF (and (exist? "userdefined") (*== (get "type") "*")) +]
    if ([+ (get "name") +]) delete [+ (get "name") +];[+ ENDIF +][+ ENDFOR +]
}
    
void [+ event_class_name +]::pack()
{
  [+ FOR param "" +][+ IF (or (*== (get "type") "[]") (*== (get "type") "*")) +][+ IF (*== (get "type") "[]") +]if ([+ (get "name") +]NOE > 0) [+ ENDIF +]assert([+ (get "name") +]);[+ ENDIF +]
  [+ ENDFOR +]
  uint32 __requiredSize = [+ IF (not (exist? "noheader")) +]sizePackedGCFEvent[+ ELSE +]0[+ ENDIF +][+ FOR param "" +]
    [+ IF (exist? "userdefined") +]+ [+ (get "name") +][+ IF (*== (get "type") "*") +]->[+ ELSE +].[+ ENDIF +]getSize()
    [+ ELIF (*== (get "type") "[]") +]+ sizeof([+ (get "name") +]NOE) + ([+ (get "name") +]NOE * sizeof([+ event_class_member_type +]))
    [+ ELSE +]+ MSH_size([+ (get "name") +])[+ ENDIF +][+ ENDFOR +];

  resizeBuf(__requiredSize);
  uint32 __offset = 0;
  [+ IF (not (exist? "noheader")) +]
  GCFEvent::pack();
  __offset = GCFEvent::sizePackedGCFEvent;[+ ENDIF +]
  [+ FOR param "" +]
  [+ IF (exist? "userdefined") +]
  __offset += [+ (get "name") +][+ IF (*== (get "type") "*") +]->[+ ELSE +].[+ ENDIF +]pack(_buffer + __offset);
  [+ ELIF (*== (get "type") "[]") +]
  __offset += packMember(__offset, [+ (get "name") +], [+ (get "name") +]NOE, sizeof([+ event_class_member_type +]));
  [+ ELSE +]
  MSH_pack(_buffer, __offset, [+ (get "name") +]);
  [+ ENDIF +][+ ENDFOR +]
	[+ IF (= (count "param") 0) +]
  // no params in this event to pack
  (void)__offset;
	[+ ENDIF +]
}

void [+ event_class_name +]::unpack()
{
	if (!length) {
		return;
	}
	[+ IF (> (count "param") 0) +]
  	uint32 __offset = sizePackedGCFEvent;
    [+ ELSE +]
    // no params in this event to unpack
  	uint32 __offset = 0;
    [+ ENDIF +]
    [+ FOR param "" +]
    [+ IF (exist? "userdefined") +]
      [+ IF (*== (get "type") "*") +]
    [+ (get "name") +] = new [+ (substring (get "type") 0 (string-index (get "type") #\*)) +]();
      [+ ENDIF +]
    __offset += [+ (get "name") +][+ IF (*== (get "type") "*") +]->[+ ELSE +].[+ ENDIF +]unpack(_buffer + __offset);
    [+ ELIF (*== (get "type") "[]") +]
    [+ (get "name") +] = ([+ event_class_member_type +]*) unpackMember(_buffer, __offset, [+ (get "name") +]NOE,  sizeof([+ event_class_member_type +]));
    [+ ELSE +]
    MSH_unpack(_buffer, __offset, [+ (get "name") +]);
    [+ ENDIF +][+ ENDFOR +]
	_buffer  = 0;
        (void)__offset;
}

[+ event_class_name +]* [+ event_class_name +]::clone()
{
	LOG_TRACE_CALC("[+ event_class_name +]::clone()");
	pack();
	[+ event_class_name +]*		clonedEvent = new [+ event_class_name +]((GCFEvent&)*this);
	ASSERTSTR(clonedEvent, "Could not allocate a new [+ event_class_name +] class");
    return (clonedEvent);
}

ostream& [+ event_class_name +]::print(ostream& os) const
{
using LOFAR::operator<<;
  // Note: 'userdefined' classes are only printed if they are 'printable'
  //       base classes are printed except when they are 'nonprintable'
  MACIO::GCFEvent::print(os);
  [+ FOR param "" +][+ IF (==* (get "type") "struct") +][+ ELSE +]
    [+ IF (or (and (exist? "userdefined") (exist? "printable")) (and (not(exist? "userdefined")) (not(exist? "nonprintable")))) +]
      [+ IF (*== (get "type") "*") +]
      os << " [+ (get "name") +] = " << *[+ (get "name") +] << endl;
      [+ ELSE +]
      os << " [+ (get "name") +] = " <<  [+ (get "name") +] << endl; // [+ (get "type") +]
      [+ ENDIF +][+ ENDIF +][+ ENDIF +]
  [+ ENDFOR +]return (os);
}
[+ ENDIF +][+ ENDFOR +]

[+ IF (= (suffix) "ph") +]
	} // namespace [+ (base-name) +]
} // namespace [+ project +]

using namespace [+ project +]::[+ (base-name) +];

#endif
[+ ENDIF +]
