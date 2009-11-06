//# BlobStringTraits.h: char_traits<uchar> implementation for GCC
//#
//# Copyright (C) 2003
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

#ifndef LOFAR_BLOB_BLOBSTRINGTRAITS_H
#define LOFAR_BLOB_BLOBSTRINGTRAITS_H

// \file
//
// char_traits<uchar> implementation for GCC.
// This file implements the char_traits<unsigned char> specialisation.
// It is only needed for the GCC version 3 and higher.
// It is taken from the DTL implementation.
// It is not implemented if DTL is used since that has its own implementation.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#if defined(HAVE_DTL)
# include <dtl/dtl_config.h>
#else
# if (defined(__GNUC__) && (__GNUC__ >= 3)) || defined(HAVE_BGL)


#include <Common/LofarTypedefs.h>
#include <string>
#include <cstring>

namespace std 
{

  // GCC needs char_traits inplementation for
  // char_traits<unsigned char>
  //
  // 21.1.4  char_traits specializations

  template<>
    struct char_traits<LOFAR::TYPES::uchar>
    {
      typedef LOFAR::TYPES::uchar char_type;
      typedef int 	          int_type;
      typedef streampos 	  pos_type;
      typedef streamoff 	  off_type;
      typedef mbstate_t 	  state_type;

      static void 
      assign(char_type& __c1, const char_type& __c2)
      { __c1 = __c2; }

      static bool 
      eq(const char_type& __c1, const char_type& __c2)
      { return __c1 == __c2; }

      static bool 
      lt(const char_type& __c1, const char_type& __c2)
      { return __c1 < __c2; }

      static int 
      compare(const char_type* __s1, const char_type* __s2, size_t __n)
      { return memcmp(__s1, __s2, __n); }

      static size_t
      length(const char_type* __s)
      { return strlen((const char *)__s); }

      static const char_type* 
      find(const char_type* __s, size_t __n, const char_type& __a)
      { return static_cast<const char_type*>(memchr(__s, __a, __n)); }

      static char_type* 
      move(char_type* __s1, const char_type* __s2, size_t __n)
      { return static_cast<char_type*>(memmove(__s1, __s2, __n)); }

      static char_type* 
      copy(char_type* __s1, const char_type* __s2, size_t __n)
      {  return static_cast<char_type*>(memcpy(__s1, __s2, __n)); }

      static char_type* 
      assign(char_type* __s, size_t __n, char_type __a)
      { return static_cast<char_type*>(memset(__s, __a, __n)); }

      static char_type 
      to_char_type(const int_type& __c)
      { return static_cast<char_type>(__c); }

      // To keep both the byte 0xff and the eof symbol 0xffffffff
      // from ending up as 0xffffffff.
      static int_type 
      to_int_type(const char_type& __c)
      { return static_cast<int_type>(static_cast<unsigned char>(__c)); }

      static bool 
      eq_int_type(const int_type& __c1, const int_type& __c2)
      { return __c1 == __c2; }

      static int_type 
      eof() { return static_cast<int_type>(EOF); }

      static int_type 
      not_eof(const int_type& __c)
      { return (__c == eof()) ? 0 : __c; }
  };
}


# endif
#endif

#endif
