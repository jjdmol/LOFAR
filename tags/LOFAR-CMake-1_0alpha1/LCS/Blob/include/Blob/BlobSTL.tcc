//# BlobSTL.tcc: Blob handling for STLs
//#
//# Copyright (C) 2007
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

#ifndef LOFAR_BLOB_BLOBSTL_TCC
#define LOFAR_BLOB_BLOBSTL_TCC

#include <Blob/BlobSTL.h>
#include <Blob/BlobArray.h>
#include <Common/TypeNames.h>

namespace LOFAR
{
  template<typename T, typename U>
  BlobOStream& operator<< (BlobOStream& bs, const std::map<T,U>& m)
  {
    bs.putStart ("map<" + typeName((T*)0) + ',' + typeName((U*)0) + '>', 1);
    bs << static_cast<uint32>(m.size());
    for (typename std::map<T,U>::const_iterator it=m.begin();
         it!=m.end();
         ++it) {
      bs << it->first << it->second;
    }
    bs.putEnd();
    return bs;
  }

  template<typename T, typename U>
  BlobIStream& operator>> (BlobIStream& bs, std::map<T,U>& m)
  {
    bs.getStart ("map<" + typeName((T*)0) + ',' + typeName((U*)0) + '>');
    m.clear();
    uint32 size;
    bs >> size;
    T t;
    U u;
    for (uint32 i=0; i<size; ++i) {
      bs >> t >> u;
      m[t] = u;
    }
    bs.getEnd();
    return bs;
  }

  template<typename Seq>
  void sequenceToBlob (BlobOStream& bs, const Seq& s)
  {
    uint32 n = s.size();
    putBlobArrayHeader (bs, true,
                        LOFAR::typeName((const typename Seq::value_type**)0),
                        &n, 1, true, 1);
    for (typename Seq::const_iterator it=s.begin();
         it!=s.end();
         ++it) {
      bs << *it;
    }
    bs.putEnd();
  }

  template<typename Seq>
  void sequenceFromBlob (BlobIStream& bs, Seq& s)
  {
    bs.getStart (LOFAR::typeName((const typename Seq::value_type**)0));
    bool fortranOrder;
    uint16 ndim;
    uint nalign = getBlobArrayStart (bs, fortranOrder, ndim);
    ASSERT(ndim == 1);
    uint32 size;
    getBlobArrayShape (bs, &size, 1, false, nalign);
    typename Seq::value_type t;
    s.clear();
    for (uint32 i=0; i<size; ++i) {
      bs >> t;
      s.push_back (t);
    }
    bs.getEnd();
  }

  template<typename T>
  void sequenceFromBlob (BlobIStream& bs, std::set<T>& s)
  {
    bs.getStart (LOFAR::typeName((const T**)0));
    bool fortranOrder;
    uint16 ndim;
    uint nalign = getBlobArrayStart (bs, fortranOrder, ndim);
    ASSERT(ndim == 1);
    uint32 size;
    getBlobArrayShape (bs, &size, 1, false, nalign);
    T t;
    s.clear();
    for (uint32 i=0; i<size; ++i) {
      bs >> t;
      s.insert (t);
    }
    bs.getEnd();
  }

} // end namespace LOFAR

#endif
