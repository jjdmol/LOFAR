#ifndef _BLITZTOAIPS_H
#define _BLITZTOAIPS_H
    
#ifndef HAVE_BLITZ
#error Blitz support required
#endif

#ifndef HAVE_AIPSPP
#error AIPS++ support required
#endif
    
#include "Common/Lorrays.h"
#include <casa/Arrays.h>

#ifndef LORRAYS_USE_BLITZ
#error Must use Blitz Lorrays
#endif
   
template<int N>
void convertShape (IPosition &ipos,const blitz::TinyVector<int,N> &tv)
{
  ipos.resize(N);
  for( int i=0; i<N; i++ )
    ipos(i) = tv[i];
}

template<int N>
void convertShape( blitz::TinyVector<int,N> &tv,const IPosition &ipos)
{
  FailWhen(ipos.nelements() != N,"IPosition size mismatch");
  for( int i=0; i<N; i++ )
    tv[i] = ipos(i);
}

// Versions of the above that return result by value
template<int N>
IPosition blitzToAips (const blitz::TinyVector<int,N> &tv)
{
  IPosition ret;
  convertShape(ret,tv);
  return ret;
}

template<int N>
blitz::TinyVector<int,N> aipsToBlitz (const IPosition &ipos)
{
  blitz::TinyVector<int,N> ret;
  convertShape(ret,tv);
  return ret;
}

   
// Makes the AIPS++ array "out" a copy of the blitz array "in".
// The data is always copied.
template<class T1,class T2,int N>
void copyArray ( Array<T1> &out,const blitz::Array<T2,N> &in )
{
  T1 *data = new T1[in.size()],*ptr = data;
  for( typename blitz::Array<T2,N>::const_iterator iter = in.begin();
      iter != in.end(); iter++,ptr++ )
    *ptr = static_cast<T1>(*iter);
  IPosition shape;
  convertShape(shape,in.shape());
  out.takeStorage(shape,data,TAKE_OVER);
}

// Makes the AIPS++ array "out" a reference to the blitz array "in".
// The AIPS++ array will not delete the data when done, hence it is up to the
// caller to ensure that the "in" object outlives the out object.
// The blitz array must be contiguous for this (otherwise a copy is made) 
template<class T1,class T2,int N>
void refArray ( Array<T1> &out,blitz::Array<T2,N> &in )
{
  FailWhen( in.dimensions() != N,"array rank mismatch" );
  if( !in.isStorageContiguous() )
    copyArray(out,in);
  else
  {
    IPosition shape;
    convertShape(shape,in.shape());
    out.takeStorage(shape,reinterpret_cast<T1*>(in.data()),SHARE);
  }
}

// Versions of the above that return result by value
template<class T,int N>
Array<T> copyBlitzToAips ( const blitz::Array<T,N> &in )
{
  Array<T> out;
  copyArray(out,in);
  return out;
}

template<class T,int N>
Array<T> refBlitzToAips ( blitz::Array<T,N> &in )
{
  Array<T> out;
  refArray(out,in);
  return out;
}


// Helper function for converting AIPS++ arrays to blitz (refArray() and 
// copyArray() below use it)
template<class T,int N>
void aipsToBlitz ( blitz::Array<T,N> &out,Array<T> &in,blitz::preexistingMemoryPolicy policy )
{
  FailWhen( in.ndim() != N,"array rank mismatch" );
  blitz::TinyVector<int,N> shape;
  convertShape(shape,in.shape());
  bool deleteData;
  T* ptr = in.getStorage(deleteData);
  // if deleteData is True, we can take over the storage. Else make copy
  blitz::Array<T,N> tmp(ptr,shape,
      deleteData ? blitz::deleteDataWhenDone : policy );
  out.reference(tmp);
}

// Makes the blitz array "out" a copy of the AIPS++ array "in".
// The data is always copied.
template<class T,int N>
void copyArray ( blitz::Array<T,N> &out,const Array<T> &in )
{
  // cast away const but that's OK since data will be duplicated
  aipsToBlitz(out,const_cast<Array<T>&>(in),blitz::duplicateData);
}

// Makes the Blitz array "out" a reference to the AIPS++ array "in".
// The Blitz array will not delete the data when done, hence it is up to the
// caller to ensure that the "in" object outlives the out object.
// The AIPS++ array must be contiguous for this (otherwise a copy is always made) 
template<class T,int N>
void refArray ( blitz::Array<T,N> &out,Array<T> &in )
{
  aipsToBlitz(out,in,blitz::neverDeleteData);
}

// Versions of the above that return result by value
template<class T,int N>
blitz::Array<T,N> copyAipsToBlitz ( const Array<T> &in )
{
  blitz::Array<T,N> out;
  copyArray(out,in);
  return out;
}

template<class T,int N>
blitz::Array<T,N> refAipsToBlitz ( Array<T> &in )
{
  blitz::Array<T,N> out;
  refArray(out,in);
  return out;
}


// Copies data between arrays. Shapes must match to begin with
template<class T,int N>
void assignArray( Array<T> &to,const blitz::Array<T,N> &from )
{
  FailWhen( to.ndim() != N,"array rank mismatch" );
  for( int i=0; i<N; i++ )
  {
    FailWhen( to.shape()(i) != from.shape()[i],"array shape mismatch" );
  }
  // BUG!!
  // Use of getStorage() is not terribly efficient in case of not-contiguous
  // AIPS++ arrays. Check with Ger, how do we quickly iterate through one?
  bool del;
  T* data = to.getStorage(del),*ptr = data;
  // copy data
  for( typename blitz::Array<T,N>::const_iterator iter = from.begin();
      iter != from.end(); iter++,ptr++ )
    *ptr = *iter;
  // reset storage
  to.putStorage(data,del);
}

template<class T,int N>
void assignArray( blitz::Array<T,N> &to,const Array<T> &from )
{
  FailWhen( to.ndim() != N,"array rank mismatch" );
  for( int i=0; i<N; i++ )
  {
    FailWhen( to.shape()(i) != from.shape()[i],"array shape mismatch" );
  }
  // BUG!!
  // Use of getStorage() is not terribly efficient in case of not-contiguous
  // AIPS++ arrays. Check with Ger, how do we quickly iterate through one?
  bool del;
  const T* data = from.getStorage(del),*ptr = data;
  // copy data
  for( typename blitz::Array<T,N>::iterator iter = to.begin();
      iter != to.end(); iter++,ptr++ )
    *iter = *iter;
  // reset storage
  from.freeStorage(data,del);
}

#endif
