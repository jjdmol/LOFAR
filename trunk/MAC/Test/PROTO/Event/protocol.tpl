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
[+ DEFINE event_class_decl +][+ event_class_name +] : public GCFEvent[+ ENDDEF +]
[+ DEFINE protocol_name +][+ (string-upcase (base-name)) +][+ ENDDEF +]
[+ DEFINE event_class_member_type +][+ IF (*== (get "type") "]") +][+ (substring (get "type") 0 (string-index (get "type") #\[)) +][+ ELSE +][+ (get "type") +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE event_class_member +][+ event_class_member_type +][+ IF (*== (get "type") "[]") +]*[+ ENDIF +] [+ (get "name") +][+ IF (and (*== (get "type") "]") (not (*== (get "type") "[]"))) +][+ (substring (get "type") (string-index (get "type") #\[) (string-length (get "type"))) +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE from_type +][+ IF (or (== (get "type") "long") (== (get "type") "int")) +]Py[+ (string-capitalize! (get "type")) +]_FromLong[+ ELSE +][+ IF (or (== (get "type") "float") (== (get "type") "double")) +]PyFloat_FromDouble[+ ELSE +][+ ENDIF +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE to_type +][+ IF (or (== (get "type") "long") (== (get "type") "int")) +]Py[+ (string-capitalize! (get "type")) +]_AsLong(o);[+ ELSE +][+ IF (or (== (get "type") "float") (== (get "type") "double")) +]PyFloat_AsDouble(o);[+ ELSE +][+ ENDIF +][+ ENDIF +][+ ENDDEF +]

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

using namespace [+ (base-name) +];

const char* [+ protocol_name +]_signalnames[] = 
{
  "[+ protocol_name +]: invalid signal",[+ FOR event "," +]
  "[+ signal_name +]"[+ ENDFOR +]
};
[+ ELSE +]
#ifndef [+ protocol_name +]_H
#define [+ protocol_name +]_H

#ifdef SWIG
%module [+ (base-name) +]
%include GCF/GCF_Event.h
%include carrays.i
%include std_string.i
%include typemaps.i
[+ FOR include "" +]
%include [+ (get "include") +][+ ENDFOR +]
[+ FOR unbounded_array_types "" +][+ FOR type "" +]
%array_class([+ (get "type") +], [+ (get "type") +]_array)
[+ ENDFOR +][+ ENDFOR +]
%{
#include "[+ (base-name) +].ph"[+ FOR include "" +]
#include [+ (get "include") +][+ ENDFOR +]
#include <GCF/GCF_TMProtocols.h>
%}
#else
[+ FOR include "" +]
#include [+ (get "include") +][+ ENDFOR +]
#include <GCF/GCF_TMProtocols.h>
#include <string>
#endif

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

// extern declaration of protocol event names
#ifndef SWIG
extern const char* [+ protocol_name +]_signalnames[];
#endif

namespace [+ (base-name) +]
{[+ (out-push-add "/dev/null") +]
[+ ENDIF +]
[+ FOR event "" +][+ IF (= (suffix) "ph") +]
[+ FOR param "" +]
[+ IF (*== (get "type") "&") +][+ (error "reference types not supported") +][+ ENDIF +]
[+ IF (and (==* (get "type") "string") (> (string-length (get "type")) 6)) +][+ (error "only scalar 'string' are supported") +][+ ENDIF +][+ ENDFOR +]
[+ (out-pop) +]
  class [+ event_class_decl +]
  {
#ifdef SWIG
%typemap(in) std::string* ($*1_ltype tempstr) {
	char * temps; int templ;
	if (PyString_AsStringAndSize($input, &temps, &templ)) return NULL;
	tempstr = $*1_ltype(temps, templ);
	$1 = &tempstr;
}
%typemap(out) std::string* {
	$result = PyString_FromStringAndSize($1->data(), $1->length());
}
[+ FOR bounded_array_types "" +][+ FOR type "" +][+ IF (not (== (get "type") "char")) +]
%typemap(in) [+ (get "type")+] [ANY] ([+ (get "type") +] temp[$1_dim0]) {
  int i;
  if (!PySequence_Check($input)) {
    PyErr_SetString(PyExc_ValueError,"Expected a sequence");
    return NULL;
  }
  if (PySequence_Length($input) != $1_dim0) {
    PyErr_SetString(PyExc_ValueError,"Size mismatch. Expected $1_dim0 elements");
    return NULL;
  }
  for (i = 0; i < $1_dim0; i++) {
    PyObject *o = PySequence_GetItem($input,i);
    if (PyNumber_Check(o)) {
      temp[i] = ([+ (get "type")+])[+ to_type +]
    } else {
      PyErr_SetString(PyExc_ValueError,"Sequence elements must be numbers");
      return NULL;
    }
  }
  $1 = temp;
}
%typemap(out) [+ (get "type")+] [ANY] {
  int i;
  $result = PyList_New($1_dim0);
  for (i = 0; i < $1_dim0; i++) {
    PyObject *o = [+ from_type +](([+ (get "type") +]) $1[i]);
    PyList_SetItem($result,i,o);
  }
}[+ ENDIF +][+ ENDFOR +][+ ENDFOR +]
#endif
    public:
      [+ event_class_name +](GCFEvent& e);
      [+ event_class_name +]();
      virtual ~[+ event_class_name +]();

      [+ FOR param ";" +]
      [+ IF (== (get "type") "string") +]std::[+ ENDIF +][+ event_class_member +][+ IF (*== (get "type") "[]") +]; unsigned int [+ (get "name") +]Dim[+ ENDIF +][+ ENDFOR +];

      void* pack(unsigned int& packsize);

    private:
      [+ event_class_name +]([+ event_class_name +]&);
      [+ event_class_name +]& operator= (const [+ event_class_name +]&);
      
	    void unpack();
  };   [+ ELSE +]
[+ event_class_name +]::[+ event_class_name +](GCFEvent& e)
  : GCFEvent(e)[+ FOR param "" +][+ IF (or (*== (get "type") "[]") (*== (get "type") "*")) +],
    [+ (get "name") +](0)[+ ENDIF +][+ IF (*== (get "type") "[]") +],
    [+ (get "name") +]Dim(0)[+ ENDIF +][+ ENDFOR +]
{
	unpack();
}
      
[+ event_class_name +]::[+ event_class_name +]()
  : GCFEvent([+ signal_name +])[+ FOR param "" +][+ IF (or (*== (get "type") "[]") (*== (get "type") "*")) +],
    [+ (get "name") +](0)[+ ENDIF +][+ IF (*== (get "type") "[]") +],
    [+ (get "name") +]Dim(0)[+ ENDIF +][+ ENDFOR +]
{        
}

[+ event_class_name +]::~[+ event_class_name +]() 
{
  if (_unpackDone)
  {[+ FOR param "" +][+ IF (and (exist? "userdefined") (*== (get "type") "*")) +]
    if ([+ (get "name") +]) delete [+ (get "name") +];[+ ENDIF +][+ ENDFOR +]
  }
}
    
void* [+ event_class_name +]::pack(unsigned int& packsize)
{
  [+ FOR param "" +][+ IF (or (*== (get "type") "[]") (*== (get "type") "*")) +]assert([+ (get "name") +]);[+ ENDIF +]
  [+ ENDFOR +]
  unsigned int requiredSize = sizeof(signal) + sizeof(length) + [+ FOR param " +" +]
    [+ IF (exist? "userdefined") +][+ (get "name") +][+ IF (*== (get "type") "*") +]->[+ ELSE +].[+ ENDIF +]getSize()
    [+ ELIF (not (*== (get "type") "]")) +][+ IF (== (get "type") "string") +][+ (get "name") +].length() + sizeof(unsigned int)[+ ELSE +]sizeof([+ (get "name") +])[+ ENDIF+]
    [+ ELIF (*== (get "type") "[]") +]sizeof([+ (get "name") +]Dim) + ([+ (get "name") +]Dim * sizeof([+ (get "name") +][0]))
    [+ ELSE +]sizeof([+ (get "name") +])[+ ENDIF +][+ ENDFOR +];

  resizeBuf(requiredSize);
  unsigned int offset = 0;
  GCFEvent::pack(offset);
  [+ FOR param "" +]
  [+ IF (exist? "userdefined") +]
  offset += [+ (get "name") +][+ IF (*== (get "type") "*") +]->[+ ELSE +].[+ ENDIF +]pack(_buffer + offset);
  [+ ELIF (not (*== (get "type") "]")) +]
    [+ IF (== (get "type") "string") +]
  offset += packMember(offset, [+ (get "name") +].c_str(), [+ (get "name") +].length(),  sizeof(char));
    [+ ELSE +]
  memcpy(_buffer + offset, &[+ (get "name") +], sizeof([+ (get "type") +]));
  offset += sizeof([+ (get "type") +]);
    [+ ENDIF +]
  [+ ELIF (*== (get "type") "[]") +]
  offset += packMember(offset, [+ (get "name") +], [+ (get "name") +]Dim, sizeof([+ (get "name") +][0]));
  [+ ELSE +]
  memcpy(_buffer + offset, [+ (get "name") +], sizeof([+ (get "name") +]));
  offset += sizeof([+ (get "name") +]);
  [+ ENDIF +][+ ENDFOR +]
          
  packsize = offset;
  return _buffer;
}

void [+ event_class_name +]::unpack()
{
  unsigned int offset = sizeof(GCFEvent);
  if (offset < length)
  {
    char* data = (char*) _base;
    [+ FOR param "" +]
    [+ IF (exist? "userdefined") +]
      [+ IF (*== (get "type") "*") +]
    [+ (get "name") +] = new [+ (substring (get "type") 0 (string-index (get "type") #\*)) +]();
      [+ ENDIF +]
    offset += [+ (get "name") +][+ IF (*== (get "type") "*") +]->[+ ELSE +].[+ ENDIF +]unpack(data + offset);
    [+ ELIF (not (*== (get "type") "]")) +]
      [+ IF (== (get "type") "string") +]
    offset += GCFEvent::unpackString([+ (get "name") +], data + offset);
      [+ ELSE +]
    memcpy(&[+ (get "name") +], data, sizeof([+ (get "type") +]));
    offset += sizeof([+ (get "type") +]);
      [+ ENDIF +]
    [+ ELIF (*== (get "type") "[]") +]
    [+ (get "name") +] = ([+ event_class_member_type +]*) unpackMember(data, offset, [+ (get "name") +]Dim,  sizeof([+ (get "name") +][0]));
    [+ ELSE +]
    memcpy([+ (get "name") +], (data + offset), sizeof([+ (get "name") +]));
    offset += sizeof([+ (get "name") +]);
    [+ ENDIF +][+ ENDFOR +]
  }
}[+ ENDIF +][+ ENDFOR +]
[+ IF (= (suffix) "ph") +]
} // namespace [+ (base-name) +]

#endif
[+ ENDIF +]
