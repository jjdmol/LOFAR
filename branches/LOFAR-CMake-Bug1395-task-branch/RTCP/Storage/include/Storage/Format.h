//#  Format.h: Virtual baseclass 
//#
//#  Copyright (C) 2009
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id: $

#ifndef LOFAR_STORAGE_FORMAT_H
#define LOFAR_STORAGE_FORMAT_H

namespace LOFAR
{
  namespace RTCP
  {
    class Format
    {
    public:
      Format();
      virtual ~Format();
      
      virtual void addSubband(unsigned subband);

    };

    inline Format::Format() {}
    inline Format::~Format() {}
    inline void Format::addSubband(unsigned) {}

  }
}

#endif
