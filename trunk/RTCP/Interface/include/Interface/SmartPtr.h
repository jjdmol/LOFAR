//#  SmartPtr.h:
//#
//#  Copyright (C) 2006
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
//#  $Id: Parset.h 17623 2011-03-23 13:40:56Z mol $

#ifndef LOFAR_INTERFACE_SMART_PTR_H
#define LOFAR_INTERFACE_SMART_PTR_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!


namespace LOFAR {
namespace RTCP {

template <typename T> class SmartPtr
{
  public:
    SmartPtr(T * = 0);
    SmartPtr(const SmartPtr<T> &orig); // WARNING: move semantics; orig no longer contains pointer

    ~SmartPtr();

    operator T * () const;
    T & operator * () const;
    T * operator -> () const;

    SmartPtr<T> & operator = (T *);
    SmartPtr<T> & operator = (const SmartPtr<T> &);

    T *get();
    T *release();

  private:
    T *ptr;
};


template <typename T> inline SmartPtr<T>::SmartPtr(T *orig)
:
  ptr(orig)
{
}


template <typename T> inline SmartPtr<T>::SmartPtr(const SmartPtr<T> &orig)
:
  ptr(orig.ptr)
{
  const_cast<T *&>(orig.ptr) = 0;
}


template <typename T> inline SmartPtr<T>::~SmartPtr()
{
  delete ptr;
}


template <typename T> inline SmartPtr<T>::operator T * () const
{
  return ptr;
}


template <typename T> inline T &SmartPtr<T>::operator * () const
{
  return *ptr;
}


template <typename T> inline T *SmartPtr<T>::operator -> () const
{
  return ptr;
}


template <typename T> inline SmartPtr<T> &SmartPtr<T>::operator = (T *orig)
{
  delete ptr;
  ptr = orig;
  return *this;
}


template <typename T> inline SmartPtr<T> &SmartPtr<T>::operator = (const SmartPtr<T> &orig)
{
  delete ptr;
  ptr = orig;
  const_cast<T *&>(orig.ptr) = 0;
  return *this;
}


template <typename T> inline T *SmartPtr<T>::get()
{
  return ptr;
}


template <typename T> inline T *SmartPtr<T>::release()
{
  T *tmp = ptr;
  ptr = 0;
  return tmp;
}


} // namespace RTCP
} // namespace LOFAR

#endif
