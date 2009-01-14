[+ AutoGen5 template ph +]
//
[+ (dne "//  ") +][+ (out-push-add "/dev/null") +]
[+ DEFINE prefix_str +][+ IF (exist? "prefix") +][+ (get "prefix") +]_[+ ENDIF +][+ ENDDEF +]
[+ DEFINE signal_name +][+ prefix_str +][+ (get "signal") +][+ ENDDEF +]
[+ DEFINE signal_id +][+ signal_name +]_ID[+ ENDDEF +]
[+ DEFINE struct_name +][+ (string-capitalize! (get "signal")) +]Event[+ ENDDEF +]
[+ DEFINE struct_decl +][+ struct_name +] : public GCFEvent[+ ENDDEF +]
[+ DEFINE signal_string +][+ signal_name +] ([+ (get "dir") +])[+ ENDDEF +]
[+ DEFINE protocol_name +][+ (string-upcase (base-name)) +][+ ENDDEF +]
[+ DEFINE param +][+ (get "type") +] [+ (get "name") +][+ IF (exist? "dim[1]") +][[+ (get "dim[1]") +]][+ ENDIF +][+ ENDDEF +]
[+ (out-pop) +]
//
//  [+ (base-name) +].h: [+ description +]
//
//  Copyright (C) 2003
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
[+ (lgpl "This program" "ASTRON" "//  ") +]
//
//  $Id$
//

#ifndef _[+ protocol_name +]_H_
#define _[+ protocol_name +]_H_

#include <TASK/GCF_Protocols.hxx>
[+ FOR include "" +]
#include [+ (get "include") +][+ ENDFOR +]

[+ (get "prelude") +]

namespace [+ (base-name) +]
{
  //
  // Define protocol message types
  //
  enum { [+ FOR event "," +]
    [+ signal_id +][+ IF (= 0 (for-index)) +] = 1[+ ENDIF +][+ ENDFOR event +]
  };

[+ FOR event "" +] #define [+ prefix_str +][+ (get "signal") +] F_SIGNAL([+ protocol_name +], [+ prefix_str +][+ (get "signal") +]_ID, F_[+ (get "dir")+])
[+ ENDFOR event +]

[+ FOR event "" +][+ IF (< 0 (count "param")) +]
  struct [+ struct_decl +]
  {
    [+ struct_name +](
      [+ FOR param "," +][+ param +]_arg[+ ENDFOR +]
    ) : GCFEvent([+ signal_name +]),
        [+ FOR param "," +][+ (get "name") +]([+ (get "name") +]_arg)[+ ENDFOR +]
    {
	    length = sizeof([+ struct_name +]);
    }

    [+ struct_name +]() : GCFEvent([+ signal_name +])
    {
      length = sizeof([+ struct_name +]);
    }
    [+ FOR param ";" +]
    [+ param +][+ ENDFOR +];
  };
[+ ENDIF +][+ ENDFOR +]

} // namespace [+ (base-name) +]

using namespace [+ (base-name) +];

#ifdef DECLARE_SIGNAL_NAMES

const char* [+ protocol_name +]_signalnames[] = {
  "[+ protocol_name +]: invalid signal",[+ FOR event "," +]
  "[+ signal_string +]"[+ ENDFOR +]
};

#else

// extern declaration of protocol event names
extern const char* [+ protocol_name +]_signalnames[];

#endif

#endif
