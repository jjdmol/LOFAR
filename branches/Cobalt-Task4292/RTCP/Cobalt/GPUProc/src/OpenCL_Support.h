#if !defined OPEN_CL_SUPPORT_H
#define OPEN_CL_SUPPORT_H

#define __CL_ENABLE_EXCEPTIONS
#include "CL/cl.hpp"

#include <boost/multi_array.hpp>
#include <vector>


namespace LOFAR
{
  namespace Cobalt
  {

    extern std::string errorMessage(cl_int error);
    extern void createContext(cl::Context &, std::vector<cl::Device> &);
    extern cl::Program createProgram(cl::Context &, std::vector<cl::Device> &, const char *sources, const char *args);


    template <class T>
    class HostBufferAllocator
    {
    public:
      // type definitions
      typedef T value_type;
      typedef T              *pointer;
      typedef const T        *const_pointer;
      typedef T              &reference;
      typedef const T        &const_reference;
      typedef std::size_t size_type;
      typedef std::ptrdiff_t difference_type;

      // rebind allocator to type U
      template <class U>
      struct rebind {
        typedef HostBufferAllocator<U> other;
      };

      // return address of values
      pointer address(reference value) const
      {
        return &value;
      }

      const_pointer address(const_reference value) const
      {
        return &value;
      }

      // constructors and destructor
      // - nothing to do because the allocator has no state
      HostBufferAllocator(cl::CommandQueue &queue, cl_mem_flags flags = CL_MEM_READ_WRITE) throw()
        :
        queue(queue),
        flags(flags)
      {
      }

      HostBufferAllocator(const HostBufferAllocator &other) throw()
        :
        queue(other.queue),
        flags(other.flags)
      {
      }

      template <class U>
      HostBufferAllocator(const HostBufferAllocator<U> &other) throw()
        :
        queue(other.queue),
        flags(other.flags)
      {
      }

      ~HostBufferAllocator() throw()
      {
      }

      // return maximum number of elements that can be allocated
      size_type max_size() const throw()
      {
        return queue.getInfo<CL_QUEUE_DEVICE>().getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() / sizeof(T);
      }

      // allocate but don't initialize num elements of type T
      pointer allocate(size_type num, const void * = 0)
      {
        buffer = cl::Buffer(queue.getInfo<CL_QUEUE_CONTEXT>(), flags | CL_MEM_ALLOC_HOST_PTR, num * sizeof(T));
        return static_cast<pointer>(queue.enqueueMapBuffer(buffer, CL_TRUE, flags & CL_MEM_READ_WRITE ? CL_MAP_READ | CL_MAP_WRITE : flags & CL_MEM_READ_ONLY ? CL_MAP_READ : flags & CL_MEM_WRITE_ONLY ? CL_MAP_WRITE : 0, 0, num * sizeof(T)));
      }

      // deallocate storage p of deleted elements
      void deallocate(pointer ptr, size_type /*num*/)
      {
        queue.enqueueUnmapMemObject(buffer, ptr);
      }

      // initialize elements of allocated storage p with value value
      void construct(pointer p, const T& value)
      {
        // initialize memory with placement new
        new ((void *) p)T(value);
      }

      // destroy elements of initialized storage p
      void destroy(pointer p)
      {
        // destroy objects by calling their destructor
        p->~T();
      }

      cl::CommandQueue queue;
      cl_mem_flags flags;
      cl::Buffer buffer;
    };

    //
    // return that all specializations of this allocator are interchangeable
    template <class T1, class T2>
    bool operator == (const HostBufferAllocator<T1> &, const HostBufferAllocator<T2> &) throw()
    {
      return true;
    }


    template <class T1, class T2>
    bool operator != (const HostBufferAllocator<T1> &, const HostBufferAllocator<T2> &) throw()
    {
      return false;
    }


    template <typename T, std::size_t DIM>
    class MultiArrayHostBuffer : public boost::multi_array<T, DIM, HostBufferAllocator<T> >
    {
    public:
      template <typename ExtentList>
      MultiArrayHostBuffer(const ExtentList &extents, cl::CommandQueue &queue, cl_mem_flags flags)
        :
        boost::multi_array<T, DIM, HostBufferAllocator<T> >(extents, boost::c_storage_order(), HostBufferAllocator<T>(queue, flags))
      {
      }

      size_t bytesize() const
      {
        return this->num_elements() * sizeof(T);
      }
    };


    template <typename T>
    class VectorHostBuffer : public std::vector<T, HostBufferAllocator<T> >
    {
    public:
      VectorHostBuffer(size_t size, cl::CommandQueue &queue, cl_mem_flags flags)
        :
        std::vector<T, HostBufferAllocator<T> >(size, T(), HostBufferAllocator<T>(queue, flags))
      {
      }
    };


#if 0
    template <typename T, std::size_t DIM>
    class MultiArraySharedBuffer
    {
    public:
      template <typename ExtentList>
      MultiArraySharedBuffer(const ExtentList &extents, cl::CommandQueue &queue, cl_mem_flags hostBufferFlags, cl_mem_flags deviceBufferFlags)
        :
        hostBuffer(extents, queue, hostBufferFlags),
        deviceBuffer(queue.getInfo<CL_QUEUE_CONTEXT>(), deviceBufferFlags, hostBuffer.num_elements() * sizeof(T)),
        queue(queue)
      {
      }

      void hostToGPU(cl_bool synchronous = CL_FALSE)
      {
        queue.enqueueWriteBuffer(deviceBuffer, synchronous, 0, hostBuffer.num_elements() * sizeof(T), hostBuffer.origin(), 0, &event);
      }

      void GPUtoHost(cl_bool synchronous = CL_FALSE)
      {
        queue.enqueueReadBuffer(deviceBuffer, synchronous, 0, hostBuffer.num_elements() * sizeof(T), hostBuffer.origin(), 0, &event);
      }

      operator cl::Buffer & ()
      {
        return deviceBuffer;
      }

      MultiArrayHostBuffer<T, DIM> hostBuffer;
      cl::Buffer deviceBuffer;
      cl::CommandQueue queue;
      cl::Event event;
    };
#else
    template <typename T, std::size_t DIM>
    class MultiArraySharedBuffer : public MultiArrayHostBuffer<T, DIM>
    {
    public:
      template <typename ExtentList>
      MultiArraySharedBuffer(const ExtentList &extents, cl::CommandQueue &queue, cl_mem_flags hostBufferFlags, cl_mem_flags deviceBufferFlags)
        :
        MultiArrayHostBuffer<T, DIM>(extents, queue, hostBufferFlags),
        deviceBuffer(queue.getInfo<CL_QUEUE_CONTEXT>(), deviceBufferFlags, this->bytesize()),
        queue(queue)
      {
      }

      template <typename ExtentList>
      MultiArraySharedBuffer(const ExtentList &extents, cl::CommandQueue &queue, cl_mem_flags hostBufferFlags, cl::Buffer &devBuffer)
        :
        MultiArrayHostBuffer<T, DIM>(extents, queue, hostBufferFlags),
        deviceBuffer(devBuffer),
        queue(queue)
      {
      }

      void hostToDevice(cl_bool synchronous = CL_FALSE)
      {
        queue.enqueueWriteBuffer(deviceBuffer, synchronous, 0, this->bytesize(), this->origin(), 0, &event);
      }

      void deviceToHost(cl_bool synchronous = CL_FALSE)
      {
        queue.enqueueReadBuffer(deviceBuffer, synchronous, 0, this->bytesize(), this->origin(), 0, &event);
      }

      operator cl::Buffer & ()
      {
        return deviceBuffer;
      }

      cl::Buffer deviceBuffer;
      cl::CommandQueue queue;
      cl::Event event;
    };
#endif

    namespace OpenCL_Support
    {
      // The sole purpose of this function is to extract detailed error
      // information if a cl::Error was thrown. Since we want the complete
      // backtrace, we cannot simply try-catch in main(), because that would
      // unwind the stack. The only option we have is to use our own terminate
      // handler.
      void terminate();
    }

  } // namespace Cobalt
} // namespace LOFAR

#endif
