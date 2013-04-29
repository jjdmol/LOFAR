if(0)
add_library(gpuproccuda
  Context.cc
  Device.cc
  DeviceMemory.cc
  Error.cc
  Event.cc
  Function.cc
  HostMemory.cc
  Module.cc
  Platform.cc
  Stream.cc)
else()
add_library(gpuproccuda
  gpu_wrapper.cc)
endif()

