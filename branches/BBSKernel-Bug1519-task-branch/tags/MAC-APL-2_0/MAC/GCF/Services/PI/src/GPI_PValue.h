//#  GPI_PValue.h: 
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef GPI_PVALUE_H
#define GPI_PVALUE_H

#ifdef SWIG
%module GPIPValue
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
#include "GPI_PValue.h"
%}
#endif

#include <GCF/GCF_Event.h>
#include <GCF/GCF_PValue.h>

class GPIPValue : public GCFTransportable
{
  public:
    GPIPValue() : _pValue(0), _unpacked(false) {};
    GPIPValue(const GCFPValue& value) : _pValue(&value), _unpacked(false) {};

    virtual ~GPIPValue() 
    {
      if (_unpacked) delete _pValue;
    }

#ifdef SWIG
 protected:
#endif

    unsigned int pack(char* buffer)
    {
      assert(_pValue);
      return _pValue->pack(buffer);
    }
    unsigned int unpack(char* buffer)
    {
      assert(!_pValue);
      _unpacked = true;
      _pValue = GCFPValue::unpackValue(buffer);
      return _pValue->getSize();
    }
    unsigned int getSize()
    {
      assert(_pValue);
      return _pValue->getSize();
    }

#ifdef SWIG
  public:
#endif
    const GCFPValue* _pValue;

  private:
    bool _unpacked;
}; 

#endif
