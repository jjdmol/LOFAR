#ifndef LOFAR_INTERFACE_MULTI_DIM_ARRAY_H
#define LOFAR_INTERFACE_MULTI_DIM_ARRAY_H

#include <Interface/Align.h>
#include <Interface/Allocator.h>
#include <Interface/Exceptions.h>
#include <boost/multi_array.hpp>

#include <memory>
#include <ostream>
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
      allocator(&allocator),
      alignment(0)
    {
    }

    MultiDimArray(const ExtentList &extents, size_t alignment = defaultAlignment(), Allocator &allocator = heapAllocator, bool padToAlignment = false)
    :
      // Use 'placement new' to force initialisation through constructors if T is a class

      // TODO: Not sure how to handle an exception raised by the constructor of T. The placement
      // delete[] will be called, but that's an empty stub.
      SuperType(new(allocator.allocate(padToAlignment ? align(nrElements(extents) * sizeof(T), alignment) : nrElements(extents) * sizeof(T), alignment))T[nrElements(extents)], extents),
      allocator(&allocator),
      alignment(alignment),
      padToAlignment(padToAlignment)
    {
    }

    MultiDimArray(const MultiDimArray<T,DIM> &other)
    :
      SuperType(other.num_elements_ ? new(other.allocator->allocate(padToAlignment ? align(other.num_elements_ * sizeof(T), other.alignment) : other.num_elements_ * sizeof(T), other.alignment))T[other.num_elements_] : 0, other.extent_list_),
      allocator(other.allocator),
      alignment(other.alignment)
    {
      T *me  = this->origin();
      const T *him = other.origin();

      for (size_t i = 0; i < this->num_elements_; i ++)
        *(me++) = *(him++); 
    }

    ~MultiDimArray()
    {
      // explicitly call the destructors in the 'placement new' array since C++
      // cannot do this for us. The delete[] operator cannot know the size of the
      // array, and the placement delete[] operator exists (since new()[] will look
      // for it) but does nothing.
      T *elem = this->origin();

      for (size_t i = 0; i < this->num_elements_; i ++) {
        // amazingly, this also works for T = float etc.
        (elem++)->~T();
      }

      allocator->deallocate(this->origin());
    }

    MultiDimArray<T,DIM> &operator= (const MultiDimArray<T,DIM> &other)
    {
      if (other.num_elements_ != this->num_elements_) {
        THROW(InterfaceException, "Tried to assign an array with " << other.num_elements_ << " elements to an array with " << this->num_elements_ << "elements.");
      }

      T *me  = this->origin();
      const T *him = other.origin();

      for (size_t i = 0; i < this->num_elements_; i ++)
        *(me++) = *(him++); 

      return *this;
    }

    void resize(const ExtentList &extents, size_t alignment, Allocator &allocator, bool padToAlignment = false)
    {
      MultiDimArray newArray(extents, alignment, allocator, padToAlignment);
      std::swap(this->base_, newArray.base_);
      std::swap(this->storage_, newArray.storage_);
      std::swap(this->extent_list_, newArray.extent_list_);
      std::swap(this->stride_list_, newArray.stride_list_);
      std::swap(this->index_base_list_, newArray.index_base_list_);
      std::swap(this->origin_offset_, newArray.origin_offset_);
      std::swap(this->directional_offset_, newArray.directional_offset_);
      std::swap(this->num_elements_, newArray.num_elements_);
      std::swap(this->allocator, newArray.allocator);
      std::swap(this->alignment, newArray.alignment);
    }

    void resize(const ExtentList &extents, size_t alignment = defaultAlignment())
    {
      resize(extents, alignment, *allocator);
    }

    bool hasShape(const ExtentList &extents)
    {
      return std::equal( this->extent_list_.begin(), this->extent_list_.end(), extents.begin() );
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
    unsigned  alignment;
    bool      padToAlignment;
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

    void resize(size_t x, size_t alignment = SuperType::defaultAlignment(), Allocator &allocator = heapAllocator, bool padToAlignment = false)
    {
      SuperType::resize(boost::extents[x], alignment, allocator, padToAlignment);
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

    Matrix(size_t x, size_t y, size_t alignment = SuperType::defaultAlignment(), Allocator &allocator = heapAllocator, bool padToAlignment = false)
    :
      SuperType(boost::extents[x][y], alignment, allocator, padToAlignment)
    {
    }

    Matrix(const ExtentList &extents, size_t alignment = SuperType::defaultAlignment(), Allocator &allocator = heapAllocator, bool padToAlignment = false)
    :
      SuperType(extents, alignment, allocator, padToAlignment)
    {
    }

    using SuperType::resize;

    void resize(size_t x, size_t y, size_t alignment = SuperType::defaultAlignment(), Allocator &allocator = heapAllocator, bool padToAlignment = false)
    {
      SuperType::resize(boost::extents[x][y], alignment, allocator, padToAlignment);
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

    void resize(size_t x, size_t y, size_t z, size_t alignment = SuperType::defaultAlignment(), Allocator &allocator = heapAllocator, bool padToAlignment = false)
    {
      SuperType::resize(boost::extents[x][y][z], alignment, allocator, padToAlignment);
    }
};

// output function for full MultiDimArrays
template <typename T, unsigned DIM> inline std::ostream &operator<< (std::ostream& str, const MultiDimArray<T,DIM> &array)
{
  str << "[ ";

  for (size_t i = 0; i < array.size(); i ++) {
    if (i > 0)
      str << ", ";

    str << array[i];
  }

  str << " ]";
  return str;
}

// output function for subdimensions of MultiDimArrays
template <typename T, unsigned DIM, typename TPtr> inline std::ostream &operator<< (std::ostream& str, const typename boost::detail::multi_array::const_sub_array<T,DIM,TPtr> &array)
{
  str << "[ ";

  for (size_t i = 0; i < array.size(); i ++) {
    if (i > 0)
      str << ", ";

    str << array[i];
  }

  str << " ]";
  return str;
}

} // namespace RTCP
} // namespace LOFAR

#endif
