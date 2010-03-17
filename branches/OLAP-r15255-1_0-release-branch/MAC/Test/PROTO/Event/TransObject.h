#ifndef _TRANSOBJECT_H_
#define _TRANSOBJECT_H_

#ifdef SWIG
%module TransObject
%include std_string.i
%typemap(in) std::string* ($*1_ltype tempstr) {
	char * temps; int templ;
	if (PyString_AsStringAndSize($input, &temps, &templ)) return NULL;
	tempstr = $*1_ltype(temps, templ);
	$1 = &tempstr;
}
%typemap(out) std::string* {
	$result = PyString_FromStringAndSize($1->data(), $1->length());
}
%{
#include "TransObject.h"
%}
#endif

#include <GCF_Event.h>

class TransObject : public GCFTransportable
{
  public:
    TransObject(int aValue1, float aValue2, std::string aValue3) :
      value1(aValue1),
      value2(aValue2),
      value3(aValue3)
    {
    }

    TransObject() :
      value1(0),
      value2(0.0),
      value3("")
    {
    }

    virtual ~TransObject() {}

#ifdef SWIG
 protected:
#endif

    unsigned int pack(char* buffer)
    {
      unsigned int offset = 0;
      memcpy(buffer + offset, &value1, sizeof(value1));
      offset += sizeof(value1);
      memcpy(buffer + offset, &value2, sizeof(value2));
      offset += sizeof(value2);
      offset += GCFEvent::packString(buffer + offset, value3);
      return offset;
    }
    unsigned int unpack(char* buffer)
    {
      unsigned int offset = 0;
      memcpy(&value1, buffer + offset, sizeof(value1));
      offset += sizeof(value1);
      memcpy(&value2, buffer + offset, sizeof(value2));
      offset += sizeof(value2);
      offset += GCFEvent::unpackString(value3, buffer + offset);
      return offset;
    }
    unsigned int getSize()
    {
      return sizeof(value1) + sizeof(value2) + sizeof(unsigned int) + value3.length();
    }

#ifdef SWIG
 public:
#endif
  
    int value1;
    float value2;
    std::string value3;  
}; 

#endif
