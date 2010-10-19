//# TypeNames.cc: Return a string giving the type name to be stored in blobs
//#
//# Copyright (C) 2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$


//# Includes
#include <Common/TypeNames.h>

namespace LOFAR
{
  const std::string& typeName (const void*)
    {
      static std::string str ("unknown");
      return str;
    }

  const std::string& typeName (const bool*)
    {
      static std::string str ("bool");
      return str;
    }

  const std::string& typeName (const char*)
    {
      static std::string str ("char");
      return str;
    }

  const std::string& typeName (const uchar*)
    {
      static std::string str ("uchar");
      return str;
    }

  const std::string& typeName (const int16*)
    {
      static std::string str ("int16");
      return str;
    }

  const std::string& typeName (const uint16*)
    {
      static std::string str ("uint16");
      return str;
    }

  const std::string& typeName (const int32*)
    {
      static std::string str ("int32");
      return str;
    }

  const std::string& typeName (const uint32*)
    {
      static std::string str ("uint32");
      return str;
    }

  const std::string& typeName (const int64*)
    {
      static std::string str ("int64");
      return str;
    }

  const std::string& typeName (const uint64*)
    {
      static std::string str ("uint64");
      return str;
    }

  const std::string& typeName (const float*)
    {
      static std::string str ("float");
      return str;
    }

  const std::string& typeName (const double*)
    {
      static std::string str ("double");
      return str;
    }

  const std::string& typeName (const fcomplex*)
    {
      static std::string str ("fcomplex");
      return str;
    }

  const std::string& typeName (const dcomplex*)
    {
      static std::string str ("dcomplex");
      return str;
    }
}
