#ifndef LOFAR_INTERFACE_MULTI_DIM_ARRAY_H
#define LOFAR_INTERFACE_MULTI_DIM_ARRAY_H

#include <Interface/Allocator.h>
#include <boost/multi_array.hpp>
#include <boost/noncopyable.hpp>

#include <memory>
#include <stdexcept>

#if _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600
#include <cstdlib>
#else
#include <malloc.h>
#endif


namespace LOFAR {
namespace RTCP {


template <typename T, unsigned DIM> class MultiDimArray : public boost::multi_array_ref<T, DIM>, boost::noncopyable
{
  public:
    typedef boost::multi_array_ref<T, DIM> SuperType;
    typedef boost::detail::multi_array::extent_gen<DIM> ExtentList;

    MultiDimArray(Allocator &allocator = heapAllocator)
    :
      SuperType(0, boost::detail::multi_array::extent_gen<DIM>()),
      allocator(&allocator)
    {
    }

    MultiDimArray(const ExtentList &extents, size_t alignment = defaultAlignment(), Allocator &allocator = heapAllocator)
    :
      // Use 'placement new' to force initialisation through constructors if T is a class

      // TODO: Not sure how to handle an exception raised by the constructor of T. The placement
      // delete[] will be called, but that's an empty stub.
      SuperType(new(allocator.allocate(nrElements(extents) * sizeof(T), alignment))T[nrElements(extents)], extents),
      allocator(&allocator)
    {
    }

    ~MultiDimArray()
    {
      // explicitly call the destructors in the 'placement new' array since C++
      // cannot do this for us. The delete[] operator cannot know the size of the
      // array, and the placement delete[] operator exists (since new()[] will look
      // for it) but does nothing.
      T *elem = this->origin();

      for( size_t i = 0; i < this->num_elements_; i++ ) {
        // amazingly, this also works for T = float etc.
        (elem++)->~T();
      }

      allocator->deallocate(this->origin());
    }

    void resize(const ExtentList &extents, const size_t alignment, Allocator &allocator)
    {
      MultiDimArray newArray(extents, alignment, allocator);
      std::swap(this->base_, newArray.base_);
      std::swap(this->storage_, newArray.storage_);
      std::swap(this->extent_list_, newArray.extent_list_);
      std::swap(this->stride_list_, newArray.stride_list_);
      std::swap(this->index_base_list_, newArray.index_base_list_);
      std::swap(this->origin_offset_, newArray.origin_offset_);
      std::swap(this->directional_offset_, newArray.directional_offset_);
      std::swap(this->num_elements_, newArray.num_elements_);
      std::swap(this->allocator, newArray.allocator);
    }

    void resize(const ExtentList &extents, const size_t alignment = defaultAlignment())
    {
      resize( extents, alignment, *allocator );
    }

    static size_t defaultAlignment()
    {
      return sizeof(T) < 16 ? 8 : sizeof(T) < 32 ? 16 : 32;
    }


    static size_t nrElements(const ExtentList &extents)
    {
      size_t size = 1;

      for (unsigned i = 0; i < extents.ranges_.size(); i ++)
	size *= extents.ranges_[i].size();

      return size;
    }

  private:
    // needs to be a pointer to be swappable in resize()
    Allocator *allocator;
};


template <typename T> class Vector : public MultiDimArray<T, 1>
{
  public:
    typedef MultiDimArray<T, 1>		   SuperType;
    typedef typename SuperType::ExtentList ExtentList;

    Vector(Allocator &allocator = heapAllocator)
    :
      SuperType(allocator)
    {
    }

    Vector(const size_t x, const size_t alignment = SuperType::defaultAlignment(), Allocator &allocator = heapAllocator)
    :
      SuperType(boost::extents[x], alignment, allocator)
    {
    }

    Vector(const ExtentList &extents, const size_t alignment = SuperType::defaultAlignment(), Allocator &allocator = heapAllocator)
    :
      SuperType(extents, alignment, allocator)
    {
    }

    using SuperType::resize;

    void resize(const size_t x, const size_t alignment = SuperType::defaultAlignment(), Allocator &allocator = heapAllocator )
    {
      SuperType::resize(boost::extents[x], alignment, allocator);
    }
};


template <typename T> class Matrix : public MultiDimArray<T, 2>
{
  public:
    typedef MultiDimArray<T, 2>		   SuperType;
    typedef typename SuperType::ExtentList ExtentList;

    Matrix(Allocator &allocator = heapAllocator)
    :
      SuperType(allocator)
    {
    }

    Matrix(const size_t x, const size_t y, const size_t alignment = SuperType::defaultAlignment(), Allocator &allocator = heapAllocator)
    :
      SuperType(boost::extents[x][y], alignment, allocator)
    {
    }

    Matrix(const ExtentList &extents, const size_t alignment = SuperType::defaultAlignment(), Allocator &allocator = heapAllocator)
    :
      SuperType(extents, alignment, allocator)
    {
    }

    using SuperType::resize;

    void resize(const size_t x, const size_t y, const size_t alignment = SuperType::defaultAlignment(), Allocator &allocator = heapAllocator)
    {
      SuperType::resize(boost::extents[x][y], alignment, allocator);
    }
};


template <typename T> class Cube : public MultiDimArray<T, 3>
{
  public:
    typedef MultiDimArray<T, 3>		   SuperType;
    typedef typename SuperType::ExtentList ExtentList;

    Cube(Allocator &allocator = heapAllocator)
    :
      SuperType(allocator)
    {
    }

    Cube(const size_t x, const size_t y, const size_t z, const size_t alignment = SuperType::defaultAlignment(), Allocator &allocator = heapAllocator)
    :
      SuperType(boost::extents[x][y][z], alignment, allocator)
    {
    }

    Cube(const ExtentList &extents, const size_t alignment = SuperType::defaultAlignment(), Allocator &allocator = heapAllocator)
    :
      SuperType(extents, alignment, allocator)
    {
    }

    using SuperType::resize;

    void resize(const size_t x, const size_t y, const size_t z, const size_t alignment = SuperType::defaultAlignment(), Allocator &allocator = heapAllocator )
    {
      SuperType::resize(boost::extents[x][y][z], alignment, allocator);
    }
};


} // namespace RTCP
} // namespace LOFAR

#endif
