[+ AutoGen5 template ph +]
//
[+ (dne "//  ") +][+ (out-push-add "/dev/null") +]
[+ DEFINE prefix_cap +][+ IF (exist? "prefix") +][+ (get "prefix") +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE prefix_ucase +][+ IF (exist? "prefix") +][+ (string-upcase (get "prefix")) +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE protocol_id +][+ IF (exist? "id") +][+ (get "id") +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE signal_name +][+ prefix_ucase +]_[+ (get "signal") +][+ ENDDEF +]
[+ DEFINE signal_id +][+ signal_name +]_ID[+ ENDDEF +]
[+ DEFINE cap_signal +][+ (string-substitute (string-capitalize! (get "signal")) '( "_" )' ( "" )) +][+ ENDDEF +]
[+ DEFINE event_class_name +][+ prefix_cap +][+ cap_signal +]Event[+ ENDDEF +]
[+ DEFINE event_class_decl +][+ event_class_name +] : public GCFEvent[+ ENDDEF +]
[+ DEFINE eventext_class_name +][+ prefix_cap +][+ cap_signal +]EventExt[+ ENDDEF +]
[+ DEFINE eventext_class_decl +][+ eventext_class_name +] : public GCFEventExt[+ ENDDEF +]
[+ DEFINE protocol_name +][+ (string-upcase (base-name)) +][+ ENDDEF +]
[+ DEFINE event_class_member +][+ (get "type") +] [+ (get "name") +][+ IF (exist? "dim") +][[+ (get "dim") +]][+ ENDIF +][+ ENDDEF +]
[+ DEFINE eventext_class_member +][+ (get "type") +]* [+ (get "name") +]; unsigned int [+ (get "name") +]Dim[+ ENDDEF +]
[+ DEFINE sizeofeventext_class_member +]sizeof([+ (get "name") +]Dim) + ([+ (get "name") +]Dim * sizeof([+ (get "type") +])) [+ ENDDEF +]
[+ DEFINE arg_name +]a[+ (string-capitalize! (get "name")) +][+ ENDDEF +]
[+ DEFINE event_class_arg +][+ (get "type") +] [+ arg_name +][+ IF (exist? "dim") +][[+ (get "dim") +]][+ ENDIF +][+ ENDDEF +]
[+ DEFINE event_class_arg_init +][+ IF (exist? "init") +],[+ (get "name") +]([+ (get "init") +])[+ ENDIF +][+ ENDDEF +]
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
#ifdef SWIG
%module [+ (base-name) +]
%{
#include "[+ (base-name) +].ph"
#include <string.h> // needed for memcpy
#include <GCF/GCF_TMProtocols.h>
#include <GCF/GCF_Event.h>
%}
#endif
#include <string.h> // needed for memcpy
#include <GCF/GCF_TMProtocols.h>
[+ FOR include "" +]
#include [+ (get "include") +][+ ENDFOR +]
[+ (get "prelude") +]

namespace [+ (base-name) +]
{
  //
  // Define protocol ID
  //
  enum {
    [+ protocol_name +] = [+ protocol_id +]
  };

  //
  // Define protocol message types
  //
  enum { [+ FOR event "," +]
    [+ signal_id +][+ IF (= 0 (for-index)) +] = 1[+ ENDIF +][+ ENDFOR event +]
  };
[+ FOR event "" +] 
#define [+ prefix_ucase +]_[+ (get "signal") +] F_SIGNAL([+ protocol_name +], [+ prefix_ucase +]_[+ (get "signal") +]_ID, F_[+ (get "dir")+])[+ ENDFOR event +]
[+ FOR event "" +]
  class [+ event_class_decl +]
  {
    public:
      [+ event_class_name +]([+ FOR param "," +]
        [+ event_class_arg +][+ ENDFOR +])
      : GCFEvent([+ signal_name +]),[+ FOR param "," +]
        [+ IF (not (exist? "dim")) +][+ (get "name") +]([+ arg_name +])[+ ENDIF +][+ ENDFOR +]
      {
        length = sizeof([+ event_class_name +]);[+ FOR param "" +]
        [+ IF (exist? "dim") +]memcpy([+ (get "name") +], [+ arg_name +], [+ (get "dim") +] * sizeof([+ (get "type") +]));[+ ENDIF +][+ ENDFOR +]
      }

      [+ event_class_name +]() : GCFEvent([+ signal_name +])[+ FOR param "" +][+ event_class_arg_init +][+ ENDFOR +]
      {
        length = sizeof([+ event_class_name +]);
      }
      [+ FOR param ";" +]
      [+ event_class_member +][+ ENDFOR +];
  };
  [+ IF (< 0 (count "sequence")) +]
  class [+ eventext_class_decl +]
  {
    public:
      [+ eventext_class_name +]([+ event_class_name +]& be, bool dounpack = false) 
      : base(be),[+ FOR sequence "," +]
        [+ (get "name") +]Dim(0),
        [+ (get "name") +](0)[+ ENDFOR +]
      {
        if (dounpack) unpack();
      }
      
      virtual ~[+ eventext_class_name +]() { }
      [+ FOR sequence ";" +]
      [+ eventext_class_member +][+ ENDFOR +];
      
      void* pack(unsigned int& packsize)
      {
        unsigned int requiredSize = sizeof([+ FOR sequence "+" +]
          [+ sizeofeventext_class_member +][+ ENDFOR +]);

        resizeBuf(requiredSize);
        unsigned int offset = 0;[+ FOR sequence ";" +]
        offset += packMember([+ (get "name") +], [+ (get "name") +]Dim,  sizeof([+ (get "type") +]), offset)[+ ENDFOR +];
        packsize = offset;
        base.length += offset;
        return buffer;
      }

      void unpack()
      {
        unsigned int offset = sizeof([+ event_class_name +]);
        if (offset < base.length)
        {
          char* data = (char*) &base;[+ FOR sequence ";" +]
          [+ (get "name") +] = ([+ (get "type") +]*) unpackMember(data, [+ (get "name") +]Dim,  sizeof([+ (get "type") +]), offset)[+ ENDFOR +];
        }
      }
      
      GCFEvent& getEvent() { return base; }

#ifdef SWIG
    private:
#endif
      [+ event_class_name +]& base;
      
    private:
      [+ eventext_class_name +]();
  };[+ ENDIF +][+ ENDFOR +]
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
