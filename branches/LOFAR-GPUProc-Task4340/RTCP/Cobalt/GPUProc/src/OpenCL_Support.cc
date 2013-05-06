//# OpenCL_Support.h
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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

#include <lofar_config.h>

#include "OpenCL_Support.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <Common/Thread/Mutex.h>

namespace LOFAR
{
  namespace Cobalt
  {

    std::string errorMessage(cl_int error)
    {
      switch (error) {
      case CL_SUCCESS:                            return "Success!";
      case CL_DEVICE_NOT_FOUND:                   return "Device not found.";
      case CL_DEVICE_NOT_AVAILABLE:               return "Device not available";
      case CL_COMPILER_NOT_AVAILABLE:             return "Compiler not available";
      case CL_MEM_OBJECT_ALLOCATION_FAILURE:      return "Memory object allocation failure";
      case CL_OUT_OF_RESOURCES:                   return "Out of resources";
      case CL_OUT_OF_HOST_MEMORY:                 return "Out of host memory";
      case CL_PROFILING_INFO_NOT_AVAILABLE:       return "Profiling information not available";
      case CL_MEM_COPY_OVERLAP:                   return "Memory copy overlap";
      case CL_IMAGE_FORMAT_MISMATCH:              return "Image format mismatch";
      case CL_IMAGE_FORMAT_NOT_SUPPORTED:         return "Image format not supported";
      case CL_BUILD_PROGRAM_FAILURE:              return "Program build failure";
      case CL_MAP_FAILURE:                        return "Map failure";
      case CL_INVALID_VALUE:                      return "Invalid value";
      case CL_INVALID_DEVICE_TYPE:                return "Invalid device type";
      case CL_INVALID_PLATFORM:                   return "Invalid platform";
      case CL_INVALID_DEVICE:                     return "Invalid device";
      case CL_INVALID_CONTEXT:                    return "Invalid context";
      case CL_INVALID_QUEUE_PROPERTIES:           return "Invalid queue properties";
      case CL_INVALID_COMMAND_QUEUE:              return "Invalid command queue";
      case CL_INVALID_HOST_PTR:                   return "Invalid host pointer";
      case CL_INVALID_MEM_OBJECT:                 return "Invalid memory object";
      case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:    return "Invalid image format descriptor";
      case CL_INVALID_IMAGE_SIZE:                 return "Invalid image size";
      case CL_INVALID_SAMPLER:                    return "Invalid sampler";
      case CL_INVALID_BINARY:                     return "Invalid binary";
      case CL_INVALID_BUILD_OPTIONS:              return "Invalid build options";
      case CL_INVALID_PROGRAM:                    return "Invalid program";
      case CL_INVALID_PROGRAM_EXECUTABLE:         return "Invalid program executable";
      case CL_INVALID_KERNEL_NAME:                return "Invalid kernel name";
      case CL_INVALID_KERNEL_DEFINITION:          return "Invalid kernel definition";
      case CL_INVALID_KERNEL:                     return "Invalid kernel";
      case CL_INVALID_ARG_INDEX:                  return "Invalid argument index";
      case CL_INVALID_ARG_VALUE:                  return "Invalid argument value";
      case CL_INVALID_ARG_SIZE:                   return "Invalid argument size";
      case CL_INVALID_KERNEL_ARGS:                return "Invalid kernel arguments";
      case CL_INVALID_WORK_DIMENSION:             return "Invalid work dimension";
      case CL_INVALID_WORK_GROUP_SIZE:            return "Invalid work group size";
      case CL_INVALID_WORK_ITEM_SIZE:             return "Invalid work item size";
      case CL_INVALID_GLOBAL_OFFSET:              return "Invalid global offset";
      case CL_INVALID_EVENT_WAIT_LIST:            return "Invalid event wait list";
      case CL_INVALID_EVENT:                      return "Invalid event";
      case CL_INVALID_OPERATION:                  return "Invalid operation";
      case CL_INVALID_GL_OBJECT:                  return "Invalid OpenGL object";
      case CL_INVALID_BUFFER_SIZE:                return "Invalid buffer size";
      case CL_INVALID_MIP_LEVEL:                  return "Invalid mip-map level";
      default:                                    std::stringstream str;
        str << "Unknown (" << error << ')';
        return str.str();
      }
    }


    void createContext(cl::Context &context, std::vector<cl::Device> &devices)
    {
      const char *platformName = getenv("PLATFORM");

#if defined __linux__
      if (platformName == 0)
#endif
      platformName = "NVIDIA CUDA";
      //platformName = "AMD Accelerated Parallel Processing";

      cl_device_type type = CL_DEVICE_TYPE_DEFAULT;

      const char *deviceType = getenv("TYPE");

      if (deviceType != 0) {
        if (strcmp(deviceType, "GPU") == 0)
          type = CL_DEVICE_TYPE_GPU;
        else if (strcmp(deviceType, "CPU") == 0)
          type = CL_DEVICE_TYPE_CPU;
        else
          LOG_ERROR_STR("Unrecognized device type: " << deviceType);
      }

      const char *deviceName = getenv("DEVICE");

      std::vector<cl::Platform> platforms;
      cl::Platform::get(&platforms);

      for (std::vector<cl::Platform>::iterator platform = platforms.begin(); platform != platforms.end(); platform++) {
        LOG_INFO_STR("Platform profile: " << platform->getInfo<CL_PLATFORM_PROFILE>());
        LOG_INFO_STR("Platform name: " << platform->getInfo<CL_PLATFORM_NAME>());
        LOG_INFO_STR("Platform version: " << platform->getInfo<CL_PLATFORM_VERSION>());
        LOG_INFO_STR("Platform extensions: " << platform->getInfo<CL_PLATFORM_EXTENSIONS>());
      }

      for (std::vector<cl::Platform>::iterator platform = platforms.begin(); platform != platforms.end(); platform++) {
        //if (platform->getInfo<CL_PLATFORM_NAME>() == platformName) {
        gpu::Platform pf; // not so nice; generalize later
        if (pf.getName() == platformName) {
          platform->getDevices(type, &devices);

          if (deviceName != 0)
            for (std::vector<cl::Device>::iterator device = devices.end(); --device >= devices.begin(); )
              if (device->getInfo<CL_DEVICE_NAME>() != deviceName)
                devices.erase(device);

          for (std::vector<cl::Device>::iterator device = devices.begin(); device != devices.end(); device++) {
            LOG_INFO_STR("Device: " << device->getInfo<CL_DEVICE_NAME>());
            LOG_INFO_STR("Max mem: " << device->getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>()/1024/1024 << " MByte");
          }

          cl_context_properties cps[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)(*platform)(), 0 };
          context = cl::Context(type, cps);
          return;
        }
      }

      LOG_FATAL_STR("Platform not found: " << platformName);
      exit(1);
    }


    cl::Program createProgram(cl::Context &context, std::vector<cl::Device> &devices, const char *sources, const char *args)
    {
      std::stringstream cmd;
      cmd << "#include \"" << std::string(sources) << '"' << std::endl;
      cl::Program program(context, cmd.str());

      try {
        program.build(devices, args);
        std::string msg;
        program.getBuildInfo(devices[0], CL_PROGRAM_BUILD_LOG, &msg);

        LOG_INFO(msg);
      } catch (cl::Error &error) {
        if (strcmp(error.what(), "clBuildProgram") == 0) {
          std::string msg;
          program.getBuildInfo(devices[0], CL_PROGRAM_BUILD_LOG, &msg);

          LOG_FATAL(msg);
          exit(1);
        } else {
          throw;
        }
      }

#if 1
      std::vector<size_t> binarySizes = program.getInfo<CL_PROGRAM_BINARY_SIZES>();
#if 0
      // cl::Program::getInfo<> cl.hpp broken
      std::vector<char *> binaries = program.getInfo<CL_PROGRAM_BINARIES>();
#else
      std::vector<char *> binaries(binarySizes.size());

      for (unsigned b = 0; b < binaries.size(); b++)
        binaries[b] = new char[binarySizes[b]];

      cl_int error = clGetProgramInfo(program(), CL_PROGRAM_BINARIES, binaries.size() * sizeof(char *), &binaries[0], 0);

      if (error != CL_SUCCESS)
        throw cl::Error(error, "clGetProgramInfo");  // FIXME: cleanup binaries[*]
#endif

      for (unsigned i = 0; i < 1 /*binaries.size()*/; i++) {
        std::stringstream filename;
        filename << sources << '-' << i << ".ptx";
        std::ofstream(filename.str().c_str(), std::ofstream::binary).write(binaries[i], binarySizes[i]);
      }

#if 1
      for (unsigned b = 0; b < binaries.size(); b++)
        delete [] binaries[b];
#endif
#endif

      return program;
    }


    namespace OpenCL_Support
    {
      void terminate()
      {
        // terminate() may be called recursively, so we need a mutex that can
        // be locked recursively.
        static Mutex mutex(Mutex::RECURSIVE);

        // Make sure that one thread has exclusive access.
        ScopedLock lock(mutex);

        // We need to safe-guard against recursive calls. E.g., we were called
        // twice, because a rethrow was attempted without an active exception.
        static bool terminating = false;

        if (!terminating) {
          // This is the first time we were called. Make sure there was an active
          // exception by trying to rethrow it. If that fails, std::terminate()
          // will be called, again.
          terminating = true;
          try {
            throw;
          }
          // Print detailed error information if a cl::Error was thrown.
          catch (cl::Error& err) {
            try {
              std::cerr << "cl::Error: " << err.what() << ": "
                        << errorMessage(err.err()) << std::endl;
            } catch (...) {}
          }
          // Catch all other exceptions, otherwise std::terminate() will call
          // abort() immediately.
          catch (...) {}
        }
        // Let the LOFAR Exception::terminate handler take it from here.
        Exception::terminate();
      }
    }

  } // namespace Cobalt
} // namespace LOFAR

