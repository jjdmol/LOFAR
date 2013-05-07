#include "Error.h"
#include <driver_types.h>

Error::Error(CUresult result) :
    _result(result),
    _type(resultType)
  {
  }

Error::Error(cudaError error) :
    _error(error),
    _type(errorType)
  {
  }

const char *Error::what() const throw()
{
  if (_type == resultType)
  {
    switch (_result) {
    case CUDA_SUCCESS:
      return "success";
    case CUDA_ERROR_INVALID_VALUE:
      return "invalid value";
    case CUDA_ERROR_OUT_OF_MEMORY:
      return "out of memory";
    case CUDA_ERROR_NOT_INITIALIZED:
      return "not initialized";
    case CUDA_ERROR_DEINITIALIZED:
      return "deinitialized";
    case CUDA_ERROR_PROFILER_DISABLED:
      return "profiler disabled";
    case CUDA_ERROR_PROFILER_NOT_INITIALIZED:
      return "profiler not initialized";
    case CUDA_ERROR_PROFILER_ALREADY_STARTED:
      return "profiler already started";
    case CUDA_ERROR_PROFILER_ALREADY_STOPPED:
      return "profiler already stopped";
    case CUDA_ERROR_NO_DEVICE:
      return "no device";
    case CUDA_ERROR_INVALID_DEVICE:
      return "invalid device";
    case CUDA_ERROR_INVALID_IMAGE:
      return "invalid image";
    case CUDA_ERROR_INVALID_CONTEXT:
      return "invalid context";
    case CUDA_ERROR_CONTEXT_ALREADY_CURRENT:
      return "context already current";
    case CUDA_ERROR_MAP_FAILED:
      return "map failed";
    case CUDA_ERROR_UNMAP_FAILED:
      return "unmap failed";
    case CUDA_ERROR_ARRAY_IS_MAPPED:
      return "array is mapped";
    case CUDA_ERROR_ALREADY_MAPPED:
      return "already mapped";
    case CUDA_ERROR_NO_BINARY_FOR_GPU:
      return "no binary for GPU";
    case CUDA_ERROR_ALREADY_ACQUIRED:
      return "already acquired";
    case CUDA_ERROR_NOT_MAPPED:
      return "not mapped";
    case CUDA_ERROR_NOT_MAPPED_AS_ARRAY:
      return "not mapped as array";
    case CUDA_ERROR_NOT_MAPPED_AS_POINTER:
      return "not mapped as pointer";
    case CUDA_ERROR_ECC_UNCORRECTABLE:
      return "ECC uncorrectable";
    case CUDA_ERROR_UNSUPPORTED_LIMIT:
      return "unsupported limit";
    case CUDA_ERROR_CONTEXT_ALREADY_IN_USE:
      return "context already in use";
    case CUDA_ERROR_INVALID_SOURCE:
      return "invalid source";
    case CUDA_ERROR_FILE_NOT_FOUND:
      return "file not found";
    case CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND:
      return "shared object symbol not found";
    case CUDA_ERROR_SHARED_OBJECT_INIT_FAILED:
      return "shared object init failed";
    case CUDA_ERROR_OPERATING_SYSTEM:
      return "operating system";
    case CUDA_ERROR_INVALID_HANDLE:
      return "invalid handle";
    case CUDA_ERROR_NOT_FOUND:
      return "not found";
    case CUDA_ERROR_NOT_READY:
      return "not ready";
    case CUDA_ERROR_LAUNCH_FAILED:
      return "launch failed";
    case CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES:
      return "launch out of resources";
    case CUDA_ERROR_LAUNCH_TIMEOUT:
      return "launch timeout";
    case CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING:
      return "launch incompatible texturing";
    case CUDA_ERROR_PEER_ACCESS_ALREADY_ENABLED:
      return "peer access already enabled";
    case CUDA_ERROR_PEER_ACCESS_NOT_ENABLED:
      return "peer access not enabled";
    case CUDA_ERROR_PRIMARY_CONTEXT_ACTIVE:
      return "primary context active";
    case CUDA_ERROR_CONTEXT_IS_DESTROYED:
      return "context is destroyed";
    case CUDA_ERROR_UNKNOWN:
      return "unknown";
    default:
      return "unknown error code";
    }
  }
  else if (_type == errorType)
  {
    switch (_error) {
        case cudaSuccess:
            return "cudaSuccess";
        case cudaErrorMissingConfiguration:
            return "cudaErrorMissingConfiguration";
        case cudaErrorMemoryAllocation:
            return "cudaErrorMemoryAllocation";
        case cudaErrorInitializationError:
            return "cudaErrorInitializationError";
        case cudaErrorLaunchFailure:
            return "cudaErrorLaunchFailure";
        case cudaErrorPriorLaunchFailure:
            return "cudaErrorPriorLaunchFailure";
        case cudaErrorLaunchTimeout:
            return "cudaErrorLaunchTimeout";
        case cudaErrorLaunchOutOfResources:
            return "cudaErrorLaunchOutOfResources";
        case cudaErrorInvalidDeviceFunction:
            return "cudaErrorInvalidDeviceFunction";
        case cudaErrorInvalidConfiguration:
            return "cudaErrorInvalidConfiguration";
        case cudaErrorInvalidDevice:
            return "cudaErrorInvalidDevice";
        case cudaErrorInvalidValue:
            return "cudaErrorInvalidValue";
        case cudaErrorInvalidPitchValue:
            return "cudaErrorInvalidPitchValue";
        case cudaErrorInvalidSymbol:
            return "cudaErrorInvalidSymbol";
        case cudaErrorMapBufferObjectFailed:
            return "cudaErrorMapBufferObjectFailed";
        case cudaErrorUnmapBufferObjectFailed:
            return "cudaErrorUnmapBufferObjectFailed";
        case cudaErrorInvalidHostPointer:
            return "cudaErrorInvalidHostPointer";
        case cudaErrorInvalidDevicePointer:
            return "cudaErrorInvalidDevicePointer";
        case cudaErrorInvalidTexture:
            return "cudaErrorInvalidTexture";
        case cudaErrorInvalidTextureBinding:
            return "cudaErrorInvalidTextureBinding";
        case cudaErrorInvalidChannelDescriptor:
            return "cudaErrorInvalidChannelDescriptor";
        case cudaErrorInvalidMemcpyDirection:
            return "cudaErrorInvalidMemcpyDirection";
        case cudaErrorAddressOfConstant:
            return "cudaErrorAddressOfConstant";
        case cudaErrorTextureFetchFailed:
            return "cudaErrorTextureFetchFailed";
        case cudaErrorTextureNotBound:
            return "cudaErrorTextureNotBound";
        case cudaErrorSynchronizationError:
            return "cudaErrorSynchronizationError";
        case cudaErrorInvalidFilterSetting:
            return "cudaErrorInvalidFilterSetting";
        case cudaErrorInvalidNormSetting:
            return "cudaErrorInvalidNormSetting";
        case cudaErrorMixedDeviceExecution:
            return "cudaErrorMixedDeviceExecution";
        case cudaErrorCudartUnloading:
            return "cudaErrorCudartUnloading";
        case cudaErrorUnknown:
            return "cudaErrorUnknown";
        case cudaErrorNotYetImplemented:
            return "cudaErrorNotYetImplemented";
        case cudaErrorMemoryValueTooLarge:
            return "cudaErrorMemoryValueTooLarge";
        case cudaErrorInvalidResourceHandle:
            return "cudaErrorInvalidResourceHandle";
        case cudaErrorNotReady:
            return "cudaErrorNotReady";
        case cudaErrorInsufficientDriver:
            return "cudaErrorInsufficientDriver";
        case cudaErrorSetOnActiveProcess:
            return "cudaErrorSetOnActiveProcess";
        case cudaErrorInvalidSurface:
            return "cudaErrorInvalidSurface";
        case cudaErrorNoDevice:
            return "cudaErrorNoDevice";
        case cudaErrorECCUncorrectable:
            return "cudaErrorECCUncorrectable";
        case cudaErrorSharedObjectSymbolNotFound:
            return "cudaErrorSharedObjectSymbolNotFound";
        case cudaErrorSharedObjectInitFailed:
            return "cudaErrorSharedObjectInitFailed";
        case cudaErrorUnsupportedLimit:
            return "cudaErrorUnsupportedLimit";
        case cudaErrorDuplicateVariableName:
            return "cudaErrorDuplicateVariableName";
        case cudaErrorDuplicateTextureName:
            return "cudaErrorDuplicateTextureName";
        case cudaErrorDuplicateSurfaceName:
            return "cudaErrorDuplicateSurfaceName";
        case cudaErrorDevicesUnavailable:
            return "cudaErrorDevicesUnavailable";
        case cudaErrorInvalidKernelImage:
            return "cudaErrorInvalidKernelImage";
        case cudaErrorNoKernelImageForDevice:
            return "cudaErrorNoKernelImageForDevice";
        case cudaErrorIncompatibleDriverContext:
            return "cudaErrorIncompatibleDriverContext";
        case cudaErrorPeerAccessAlreadyEnabled:
            return "cudaErrorPeerAccessAlreadyEnabled";
        case cudaErrorPeerAccessNotEnabled:
            return "cudaErrorPeerAccessNotEnabled";
        case cudaErrorDeviceAlreadyInUse:
            return "cudaErrorDeviceAlreadyInUse";
        case cudaErrorProfilerDisabled:
            return "cudaErrorProfilerDisabled";
        case cudaErrorProfilerNotInitialized:
            return "cudaErrorProfilerNotInitialized";
        case cudaErrorProfilerAlreadyStarted:
            return "cudaErrorProfilerAlreadyStarted";
        case cudaErrorProfilerAlreadyStopped:
            return "cudaErrorProfilerAlreadyStopped";
#if __CUDA_API_VERSION >= 0x4000
        case cudaErrorAssert:
            return "cudaErrorAssert";
        case cudaErrorTooManyPeers:
            return "cudaErrorTooManyPeers";
        case cudaErrorHostMemoryAlreadyRegistered:
            return "cudaErrorHostMemoryAlreadyRegistered";
        case cudaErrorHostMemoryNotRegistered:
            return "cudaErrorHostMemoryNotRegistered";
#endif
        case cudaErrorStartupFailure:
            return "cudaErrorStartupFailure";
        case cudaErrorApiFailureBase:
            return "cudaErrorApiFailureBase";
   
      default:
        return "unknown error code";
    }
  }
  return "Unknown error occurt";  // cannot be reached
}

void checkCudaCall(CUresult result)
{
  if (result != CUDA_SUCCESS)
    throw Error(result);
}

void checkCudaCall(cudaError error)
{
  if (error != cudaSuccess)
    throw Error(error);
}
