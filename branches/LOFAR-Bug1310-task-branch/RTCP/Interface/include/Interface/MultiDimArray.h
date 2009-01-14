#ifndef LOFAR_INTERFACE_MULTI_DIM_ARRAY_H
#define LOFAR_INTERFACE_MULTI_DIM_ARRAY_H

#include <Interface/Allocator.h>
#include <boost/multi_array.hpp>

#include <memory>
#include <stdexcept>

#if _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600
#include <cstdlib>
#else
#include <malloc.h>
#endif


namespace LOFAR {
namespace RTCP {


template <typename T, unsigned DIM> class MultiDimArray : public boost::multi_array_ref<T, DIM>
{
  public:
    typedef boost::multi_array_ref<T, DIM> SuperType;
    typedef boost::detail::multi_array::extent_gen<DIM> ExtentList;

    MultiDimArray(Allocator &allocator = heapAllocator)
    :
      SuperType(0, boost::detail::multi_array::extent_gen<DIM>()),
      allocator(allocator)
    {
    }

    MultiDimArray(const ExtentList &extents, size_t alignment = defaultAlignment(), Allocator &allocator = heapAllocator)
    :
      SuperType(static_cast<T *>(allocator.allocate(nrElements(extents) * sizeof(T), alignment)), extents),
      allocator(allocator)
    {
    }

    ~MultiDimArray()
    {
      allocator.deallocate(this->origin());
    }

    void resize(const ExtentList &extents, size_t alignment = defaultAlignment())
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
    }

    static size_t defaultAlignment()
    {
      return sizeof(T) < 16 ? 8 : sizeof(T) < 32 ? 16 : 32;
    }

  private:
    Allocator &allocator;

    // don't allow copy constructors -- they are not yet implemented
    // the default boost implementation just copies the pointers
    // to the data area, resulting in a double free when deallocating
    // both copies.
    MultiDimArray( const MultiDimArray<T,DIM> &other );

    static size_t nrElements(const ExtentList &extents)
    {
      size_t size = 1;

      for (unsigned i = 0; i < extents.ranges_.size(); i ++)
	size *= extents.ranges_[i].size();

      return size;
    }
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

    Vector(size_t x, size_t alignment = SuperType::defaultAlignment(), Allocator &allocator = heapAllocator)
    :
      SuperType(boost::extents[x], alignment, allocator)
    {
    }

    Vector(const ExtentList &extents, size_t alignment = SuperType::defaultAlignment(), Allocator &allocator = heapAllocator)
    :
      SuperType(extents, alignment, allocator)
    {
    }

    using SuperType::resize;

    void resize(size_t x, size_t alignment = SuperType::defaultAlignment())
    {
      SuperType::resize(boost::extents[x], alignment);
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

    Matrix(size_t x, size_t y, size_t alignment = SuperType::defaultAlignment(), Allocator &allocator = heapAllocator)
    :
      SuperType(boost::extents[x][y], alignment, allocator)
    {
    }

    Matrix(const ExtentList &extents, size_t alignment = SuperType::defaultAlignment(), Allocator &allocator = heapAllocator)
    :
      SuperType(extents, alignment, allocator)
    {
    }

    using SuperType::resize;

    void resize(size_t x, size_t y, size_t alignment = SuperType::defaultAlignment())
    {
      SuperType::resize(boost::extents[x][y], alignment);
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

    Cube(size_t x, size_t y, size_t z, size_t alignment = SuperType::defaultAlignment(), Allocator &allocator = heapAllocator)
    :
      SuperType(boost::extents[x][y][z], alignment, allocator)
    {
    }

    Cube(const ExtentList &extents, size_t alignment = SuperType::defaultAlignment(), Allocator &allocator = heapAllocator)
    :
      SuperType(extents, alignment, allocator)
    {
    }

    using SuperType::resize;

    void resize(size_t x, size_t y, size_t z, size_t alignment = SuperType::defaultAlignment())
    {
      SuperType::resize(boost::extents[x][y][z], alignment);
    }
};


} // namespace RTCP
} // namespace LOFAR

#endif
