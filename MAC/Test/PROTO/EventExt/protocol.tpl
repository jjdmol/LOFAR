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
[+ DEFINE object_name +]p[+ (string-capitalize! (get "name")) +][+ ENDDEF +]
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
%include GCF/GCF_Event.h
%include carrays.i
%include std_string.i
[+ FOR include "" +]
%include [+ (get "include") +][+ ENDFOR +]
%array_class(int, int_array)
%array_class(char, char_array)
%{
#include "[+ (base-name) +].ph"
[+ FOR include "" +]
#include [+ (get "include") +][+ ENDFOR +]
#include <GCF/GCF_TMProtocols.h>
%}
#else
#include "[+ (base-name) +].ph"
[+ FOR include "" +]
#include [+ (get "include") +][+ ENDFOR +]
#include <GCF/GCF_TMProtocols.h>
#endif

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
  [+ IF (< 0 (sum (count "sequence" ) (count "object"))) +]
  class [+ eventext_class_decl +]
  {
    public:
      [+ eventext_class_name +]([+ event_class_name +]& be, ACTION action = SENDING) 
      : base(be)[+ IF (< 0 (count "sequence")) +],[+ FOR sequence "," +]
        [+ (get "name") +]Dim(0), [+ (get "name") +](0)[+ ENDFOR +][+ ENDIF +][+ IF (< 0 (count "object")) +],[+ FOR object "," +][+ IF (not (== (get "type") "string")) +]
        [+ object_name +](0)[+ ELSE +]
        [+ (get "name") +]()[+ ENDIF +][+ ENDFOR +][+ ENDIF +]
      {        
	      if (action == RECEIVED) 
	      {
	      	unpack();	      
		      _unpackDone = true;
		    }
      }
      
      virtual ~[+ eventext_class_name +]() 
      {
      	if (_unpackDone)
      	{[+ FOR object "" +][+ IF (not (== (get "type") "string")) +]
		      if ([+ object_name +]) delete [+ object_name +];[+ ENDIF +][+ ENDFOR +]
      	}
      }
#ifdef SWIG
    private:
#endif
      [+ event_class_name +]& base;      
#ifdef SWIG
    public:
#endif
      GCFEvent& getEvent() { return base; }

			// IMPORTANT: User may only free the member data if he also constructed it.
      // sequence parameters
      [+ FOR sequence ";" +]
      [+ eventext_class_member +][+ ENDFOR +];
      
      // parameters of userdefined types (incl. string)[+ FOR object "" +][+ IF (not (== (get "type") "string")) +]      
      GCFTransportable* [+ object_name +];[+ ELSE +]
      std::string [+ (get "name") +];[+ ENDIF +][+ ENDFOR +]
      
      void* pack(unsigned int& packsize)
      {
        unsigned int requiredSize = [+ IF (< 0 (count "sequence")) +][+ FOR sequence "+" +]
          [+ sizeofeventext_class_member +][+ ENDFOR +][+ ELSE +]0[+ ENDIF +];
        [+ IF (< 0 (count "object")) +][+ FOR object "" +][+ IF (== (get "type") "string") +]
        requiredSize += [+ (get "name") +].length() + sizeof(unsigned int); // needed bufferspace for string [+ (get "name") +][+ ELSE +]
        assert([+ object_name +]);
        requiredSize += [+ object_name +]->getSize();[+ ENDIF +][+ ENDFOR +][+ ENDIF +]

        resizeBuf(requiredSize);
        unsigned int offset = 0;
        // pack sequence members[+ FOR sequence "" +]
        offset += packMember(offset, [+ (get "name") +], [+ (get "name") +]Dim,  sizeof([+ (get "type") +]));[+ ENDFOR +]
        
        // pack members of user defined types (incl. "string")[+ FOR object "" +][+ IF (== (get "type") "string") +]
        offset += packMember(offset, [+ (get "name") +].c_str(), [+ (get "name") +].length(),  sizeof(char));[+ ELSE +]
        offset += [+ object_name +]->pack(_buffer + offset);[+ ENDIF +][+ ENDFOR +]
        
        packsize = offset;
        base.length += offset;
        return _buffer;
      }

	private:
      void unpack()
      {
        unsigned int offset = sizeof([+ event_class_name +]);
        if (offset < base.length)
        {
          char* data = (char*) &base;
          // unpack sequence members[+ FOR sequence "" +]
          [+ (get "name") +] = ([+ (get "type") +]*) unpackMember(data, offset, [+ (get "name") +]Dim,  sizeof([+ (get "type") +]));[+ ENDFOR +]
	        // unpack members of user defined types (incl. "string")[+ FOR object "" +][+ IF (== (get "type") "string") +]
				  offset += GCFEventExt::unpackString([+ (get "name") +], data + offset);[+ ELSE +]          
	        [+ object_name +] = new [+ (get "type")+]();
	        offset += [+ object_name +]->unpack(data + offset);[+ ENDIF +][+ ENDFOR +]
        }
      }
            
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
