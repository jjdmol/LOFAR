//  Lorrays-Blitz.h: Define Blitz array types
//
//  Copyright (C) 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#ifndef COMMON_LORRAYS_BLITZ_H
#define COMMON_LORRAYS_BLITZ_H

// other code can use this symbol to check for blitz arrays
#define LORRAYS_USE_BLITZ 1
#define LORRAYS_DEFINE_STRING 1
        
// Redefine the default Blitz array storage to column-major
#define BZ_DEFAULT_ARRAY_STORAGE ColumnMajorArray<N_rank>()
#include <blitz/array.h>

#ifdef HAVE_AIPSPP      
# include <casa/Arrays/IPosition.h>
#endif

#include <Common/Lonumerics.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
    
  //    
  // Define the DoForAllArrayTypes and DoForAllNonArrayTypes macros.
  // These are similar to DoForAllNumericTypes.
  //
  // In the case of blitz, all numeric types are supported, so the definitions
  // are trivial. Other array packages (e.g. AIPS++) may supported a limited
  // subset of array types.
  //  
#ifndef DoForAllArrayTypes

#define DoForAllArrayTypes(Do,arg) DoForAllNumericTypes(Do,arg); Do(string,arg);
#define DoForAllArrayTypes1(Do,arg) DoForAllNumericTypes1(Do,arg), Do(string,arg)
#define DoForAllArrayTypes_Sep(Do,arg,sep) DoForAllNumericTypes_Sep(Do,arg,sep) sep Do(string,arg)

#define DoForAllNonArrayTypes(Do,arg) ;
#define DoForAllNonArrayTypes1(Do,arg) 
#define DoForAllNonArrayTypes_Sep(Do,arg,sep)  

#endif

  //
  // For arrays, we define types as
  //
  //    LoVec_type
  //    LoMat_type
  //    LoCube_type
  //    Lorray4_type    (4D array)
  //    Lorray5_type    (5D array)
  //
  // Alternatively, you may use the LoVec(type), LoMat(type), etc. macros
  //
  // This is done for all built-in numerics, plus fcomplex, dcomplex and string.
  //
#define Typedef_arrays(x,arg) \
  typedef blitz::Array<x,1> LoVec_##x ; \
  typedef blitz::Array<x,2> LoMat_##x; \
  typedef blitz::Array<x,3> LoCube_##x; \
  typedef blitz::Array<x,1> Lorray1_##x ; \
  typedef blitz::Array<x,2> Lorray2_##x ; \
  typedef blitz::Array<x,3> Lorray3_##x ; \
  typedef blitz::Array<x,4> Lorray4_##x; \
  typedef blitz::Array<x,5> Lorray5_##x; 
  DoForAllArrayTypes(Typedef_arrays,);

#define Lorray(n,t) Lorray##n##_##t
#define LoVec(t) LoVec_##t
#define LoMat(t) LoMat_##t
#define LoCube(t) LoCube_##t
#define Lorray4(t) Lorray4_##t
#define Lorray5(t) Lorray5_##t

  const uint MaxLorrayRank = 5;

  // Define type iterator macros for arrays
#define DoForAllArrayRanks(Do,arg) \
  Do(1,arg); Do(2,arg); Do(3,arg); Do(4,arg); Do(5,arg); 
  //  Do(6,arg); Do(7,arg); Do(8,arg); Do(9,arg); Do(10,arg); Do(11,arg);

#define DoForAllArrayRanks1(Do,arg) \
  Do(1,arg), Do(2,arg), Do(3,arg), Do(4,arg), Do(5,arg) 
  //  Do(6,arg), Do(7,arg), Do(8,arg), Do(9,arg), Do(10,arg), Do(11,arg)
  
  //
  // Array shapes are expressed via the shape types:
  //
  //    LoShape1 aka LoVecShape    (1D shapes)
  //    LoShape2 aka LoMatShape    (2D shapes)
  //    LoShape3 aka LoCubeShape   (3D shapes)
  //    LoShape4 etc.
  // 
  typedef blitz::TinyVector<int,1> LoShape1;
  typedef LoShape1 LoVecShape;
  typedef blitz::TinyVector<int,2> LoShape2;
  typedef LoShape2 LoMatShape;
  typedef blitz::TinyVector<int,3> LoShape3;
  typedef LoShape3 LoCubeShape;
  typedef blitz::TinyVector<int,4> LoShape4;
  typedef blitz::TinyVector<int,5> LoShape5;

  // 
  // The inlines makeLoShape(n1,...) construct a shape object of the given
  // dimensionality
  //
  inline LoShape1 makeLoShape(int n1)
  { return LoShape1(n1); }

  inline LoShape2 makeLoShape(int n1,int n2)
  { return LoShape2(n1,n2); }

  inline LoShape3 makeLoShape(int n1,int n2,int n3)
  { return LoShape3(n1,n2,n3); }

  inline LoShape4 makeLoShape(int n1,int n2,int n3,int n4)
  { return LoShape4(n1,n2,n3,n4); }

  inline LoShape5 makeLoShape(int n1,int n2,int n3,int n4,int n5)
  { return LoShape5(n1,n2,n3,n4,n5); }

  //
  // Array positions are expressed via the position types:
  //
  //    LoPos1 aka LoVecPos    (1D shapes)
  //    LoPos2 aka LoMatPos    (2D shapes)
  //    LoPos3 aka LoCubePos   (3D shapes)
  //    LoPos4 etc.
  // 
  // Alternatively, the LoPos(n) macro may be used 
  //
  typedef blitz::TinyVector<int,1> LoPos1;
  typedef LoPos1 LoVecPos;
  typedef blitz::TinyVector<int,2> LoPos2;
  typedef LoPos2 LoMatPos;
  typedef blitz::TinyVector<int,3> LoPos3;
  typedef LoPos3 LoCubePos;
  typedef blitz::TinyVector<int,4> LoPos4;
  typedef blitz::TinyVector<int,5> LoPos5;

  // 
  // The inlines makeLoPos(n1,...) construct a position object of the given
  // dimensionality
  //
  inline LoPos1 makeLoPos(int n1)
  { return LoPos1(n1); }

  inline LoPos2 makeLoPos(int n1,int n2)
  { return LoPos2(n1,n2); }

  inline LoPos3 makeLoPos(int n1,int n2,int n3)
  { return LoPos3(n1,n2,n3); }

  inline LoPos4 makeLoPos(int n1,int n2,int n3,int n4)
  { return LoPos4(n1,n2,n3,n4); }

  inline LoPos5 makeLoPos(int n1,int n2,int n3,int n4,int n5)
  { return LoPos5(n1,n2,n3,n4,n5); }

  //
  // VariVector is a helper class. This is basically a wrapper around
  // vector<int>, providing implicit convertions to/from TinyVectors.
  // This class is used to implement the LoShape and LoPos types.
  // 
  class VariVector : public std::vector<int>
  {
  public:
    static const int SETRANK = 0x80000000;
      
    VariVector () {}
    // explicit constructor for 1 dimension. Use (size|VariVector::SIZE)
    // to construct an empty shape for 2 or more dimensions
    explicit VariVector (int n1) 
      : std::vector<int>((n1&SETRANK)?(n1&~SETRANK):1,(n1&SETRANK)?0:n1) {};
    // can be copied from simple vector, too
    VariVector (const std::vector<int> &other)
      : std::vector<int>(other) {}; 
    // constructors for 2..5 dimensions
    VariVector (int n1,int n2) : std::vector<int>(2)
    { iterator iter = begin(); *iter++=n1; *iter++=n2; }
    VariVector (int n1,int n2,int n3) : std::vector<int>(3)
    { iterator iter = begin(); *iter++=n1; *iter++=n2; *iter++=n3; }
    VariVector (int n1,int n2,int n3,int n4) : std::vector<int>(4)
    { iterator iter = begin(); *iter++=n1; *iter++=n2; *iter++=n3; *iter++=n4; }
    VariVector (int n1,int n2,int n3,int n4,int n5) : std::vector<int>(5)
    { iterator iter = begin(); *iter++=n1; *iter++=n2; *iter++=n3; *iter++=n4; *iter++=n5; }
      
    // construct from TinyVector
    // (this assumes contiguity in TinyVector, which is probably pretty safe to assume)
    template<int N>
    VariVector( const blitz::TinyVector<int,N> &tvec )
      : std::vector<int>(tvec.data(),tvec.data() + N) {};
    // convert to TinyVector
    template<int N>
    operator blitz::TinyVector<int,N> () const
    { 
      blitz::TinyVector<int,N> tvec(0);
      for( int i = 0; i < std::min(N,(int)size()); i++ )
        tvec[i] = (*this)[i];
      return tvec;
    }
    // assigns scalar value to all elements
    VariVector & operator = (int value)
    { assign(size(),value); return *this; }
    // nelements() is an alias for size
    int nelements () const
    { return size(); }
    // returns product of all elements (this is used quite frequently)
    int product () const
    { 
      int res = 1; 
      for( const_iterator iter = begin(); iter != end(); iter++ )
        res *= *iter;
      return res;
    }
      
    // dumps to stream
    std::ostream & write (std::ostream &str) const
    {
      str << "[ ";
      for( const_iterator iter = begin(); iter != end(); iter++ )
        str << *iter << " ";
      return str << "]";
    }
      
#ifdef HAVE_AIPSPP      
    // convert to/from AIPS++ IPosition
    VariVector(const IPosition &ipos)
      : std::vector<int>(ipos.storage(),ipos.storage()+ipos.nelements()) {};
    IPosition as_IPosition () const
    {
      IPosition ipos(size());
      const_iterator iter = begin();
      for( uint i=0; i<size(); i++ )
        ipos[i] = *iter++;
      return ipos;
    }
    operator IPosition () const
    { return as_IPosition(); }
        
#endif
  };

  // dumps VariVector to stream
  inline std::ostream & operator << ( std::ostream &str,const VariVector &vec )
  {
    return vec.write(str);
  }

  // comparisons for VariVectors (LoPos/LoShape) and TinyVectors
  template<class T,int N>
  inline bool operator == ( const blitz::TinyVector<T,N> &a,const blitz::TinyVector<T,N> &b )
  {
    for( int i=0; i<N; i++ )
      if( a[i] != b[i] )
        return false;
    return true;
  }
  template<class T,int N>
  inline bool operator != ( const blitz::TinyVector<T,N> &a,const blitz::TinyVector<T,N> &b )
  {
    return !(a==b);
  }
  template<int N>
  inline bool operator == ( const blitz::TinyVector<int,N> &tv,const VariVector &vv )
  {
    if( vv.size() != N )
      return false;
    int i=0;
    for( VariVector::const_iterator ivv = vv.begin(); ivv != vv.end(); ivv++,i++ )
      if( *ivv != tv[i] )
        return false;
    return true;
  }
  template<int N>
  inline bool operator != ( const blitz::TinyVector<int,N> &tv,const VariVector &vv )
  {
    return !(tv == vv);
  }
  template<int N>
  inline bool operator == ( const VariVector &vv,const blitz::TinyVector<int,N> &tv )
  {
    return tv == vv;
  }
  template<int N>
  inline bool operator != ( const VariVector &vv,const blitz::TinyVector<int,N> &tv )
  {
    return !(tv == vv);
  }
    

  // LoShape and LoPos represent variable-dimensionality
  // shapes and positions
  typedef VariVector LoShape;
  typedef VariVector LoPos;

  //
  // The LoRange object represents a range
  // 
  // Two inlines are provided for constructing ranges:
  //    makeLoRange(start,end,stride=1) 
  // returns a range from [start,end] with the given stride, and
  //    makeLoRangeWithLength(start,length,stride=1)
  // return a range of length elements, starting with "start", and with
  // the given stride.
  // Use of these inlines is encouraged, since different array packages
  // use different semantics for range-type objects (e.g. AIPS++ Slice
  // uses start and length, while Blitz uses start and end). The inlines 
  // allow for some kind of translation.
  //

  typedef blitz::Range LoRange;

  inline LoRange makeLoRange (int start,int end,int stride=1) 
  { return LoRange(start,end,stride); }

  inline LoRange makeLoRangeWithLen (int start,int length,int stride=1)
  { return LoRange(start,start+(length-1)*stride,stride); }


  //
  // LoRectDomainN and LoStrDomainN represent rectangular domains
  // (with an optional stride) of rank N.
  // You may also use the LoRectDomain(rank) and LoStrDomain(rank) macros
  // for the typenames.
  //
  // As above, LoVecRectDomain, LoMat..., LoCube... aliases are provided 
  // for ranks 1, 2 and 3.
  //

  typedef blitz::RectDomain<1> LoRectDomain1;
  typedef LoRectDomain1 LoVecRectDomain;
  typedef blitz::RectDomain<2> LoRectDomain2;
  typedef LoRectDomain2 LoMatRectDomain;
  typedef blitz::RectDomain<3> LoRectDomain3;
  typedef LoRectDomain2 LoCubeRectDomain;
  typedef blitz::RectDomain<4> LoRectDomain4;
  typedef blitz::RectDomain<5> LoRectDomain5;
#define LoRectDomain(rank) LoRectDomain##rank

  typedef blitz::StridedDomain<1> LoStrDomain1;
  typedef LoStrDomain1 LoVecStrDomain;
  typedef blitz::StridedDomain<2> LoStrDomain2;
  typedef LoStrDomain2 LoMatStrDomain;
  typedef blitz::StridedDomain<3> LoStrDomain3;
  typedef LoStrDomain3 LoCubeStrDomain;
  typedef blitz::StridedDomain<4> LoStrDomain4;
  typedef blitz::StridedDomain<5> LoStrDomain5;
#define LoStrDomain(rank) LoStrDomain##rank

} // namespace LOFAR

#endif
