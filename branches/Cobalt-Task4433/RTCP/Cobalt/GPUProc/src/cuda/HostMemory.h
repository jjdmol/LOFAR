#pragma once

#include <cstddef>

class HostMemory
{
public:
  HostMemory(size_t bytesize, unsigned flags = 0);
  ~HostMemory();
  template<typename T> operator T * ();
  template<typename T> operator const T * () const;
private:
  friend class Stream;
  void *_ptr;
  size_t _size;
};

template<typename T> HostMemory::operator T * ()
{
  return static_cast<T *>(_ptr);
}

template<typename T> HostMemory::operator const T * () const
{
  return static_cast<const T *>(_ptr);
}

