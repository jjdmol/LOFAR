//# ConverterImpl.h: implementation of the AMC Converter interface.
//#
//# Copyright (C) 2002-2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_AMCIMPL_CONVERTERIMPL_H
#define LOFAR_AMCIMPL_CONVERTERIMPL_H

// \file
// Implementation of the AMC Converter interface

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <AMCBase/Converter.h>

namespace LOFAR
{
  namespace AMC
  {
    //# Forward declarations
    struct RequestData;
    struct ResultData;

    // \addtogroup AMCImpl
    // @{

    // This class represents the real implementation of the Converter
    // interface. The hard conversion work is done by this class.
    class ConverterImpl : public Converter
    {
    public:
      ConverterImpl();

      virtual ~ConverterImpl() {}

      virtual void j2000ToAzel(ResultData&, const RequestData&);
      
      virtual void azelToJ2000(ResultData&, const RequestData&);
      
      virtual void j2000ToItrf(ResultData&, const RequestData&);
      
      virtual void itrfToJ2000(ResultData&, const RequestData&);
    };

    // @}

  } // namespace AMC
  
} // namespace LOFAR

#endif
