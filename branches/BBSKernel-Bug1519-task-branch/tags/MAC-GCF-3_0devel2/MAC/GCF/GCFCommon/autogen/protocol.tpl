[+ AutoGen5 template ph +]
//
[+ (dne "//  ") +][+ (out-push-add "/dev/null") +]
[+ DEFINE prefix_cap +][+ IF (exist? "prefix") +][+ (get "prefix") +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE prefix_ucase +][+ IF (exist? "prefix") +][+ (string-upcase (get "prefix")) +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE signal_name +][+ prefix_ucase +]_[+ (get "signal") +][+ ENDDEF +]
[+ DEFINE signal_id +][+ signal_name +]_ID[+ ENDDEF +]
[+ DEFINE struct_name +][+ prefix_cap +][+ (string-capitalize! (get "signal")) +]Event[+ ENDDEF +]
[+ DEFINE struct_decl +][+ struct_name +] : public GCFEvent[+ ENDDEF +]
[+ DEFINE protocol_name +][+ (string-upcase (base-name)) +][+ ENDDEF +]
[+ DEFINE param +][+ (get "type") +] [+ (get "name") +][+ IF (exist? "dim")+][[+ (get "dim") +]][+ ENDIF +][+ enddef +]
[+ DEFINE param_arg +][+ (get "type") +] [+ (get "name") +]_arg[+ IF (exist? "dim")+][[+ (get "dim") +]][+ ENDIF +][+ enddef +]
[+ DEFINE param_init +][+ IF (exist? "init") +],[+ (get "name") +]([+ (get "init") +])[+ ENDIF +][+ enddef +]
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

#ifndef [+ protocol_name +]_H
#define [+ protocol_name +]_H

#include <string.h> // needed for memcpy
#include <GCF/GCF_TMProtocols.h>
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

[+ FOR event "" +] #define [+ prefix_ucase +]_[+ (get "signal") +] F_SIGNAL([+ protocol_name +], [+ prefix_ucase +]_[+ (get "signal") +]_ID, F_[+ (get "dir")+])
[+ ENDFOR event +]

[+ FOR event "" +][+ IF (< 0 (count "param")) +]
  struct [+ struct_decl +]
  {
    [+ struct_name +]([+ FOR param "," +]
		[+ param_arg +][+ ENDFOR +]) 
	: GCFEvent([+ signal_name +]),[+ FOR param "," +]
          [+ (get "name") +]([+ IF (not (exist? "dim")) +][+ (get "name") +]_arg[+ ENDIF +])[+ ENDFOR +]
    {
	length = sizeof([+ struct_name +]);

        [+ FOR param "" +]
	[+ IF (exist? "dim") +]memcpy([+ (get "name") +], [+ (get "name") +]_arg, [+ (get "dim") +] * sizeof([+ (get "type") +]));[+ ENDIF +][+ ENDFOR +]
    }

    [+ struct_name +]() : GCFEvent([+ signal_name +])[+ FOR param "" +][+ param_init +][+ ENDFOR +]
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
  "[+ signal_name +]"[+ ENDFOR +]
};

#else

// extern declaration of protocol event names
extern const char* [+ protocol_name +]_signalnames[];

#endif

#endif
