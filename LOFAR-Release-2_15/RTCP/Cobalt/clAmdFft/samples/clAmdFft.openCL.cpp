////////////////////////////////////////////
//	Copyright (C) 2010,2011 Advanced Micro Devices, Inc. All Rights Reserved.
////////////////////////////////////////////

// clAmdFft.opencl.cpp : Provides functions to set up openCL
//

#include "stdafx.h"
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <vector>
#include "clAmdFft.h"
#include "clAmdFft.openCL.h"

void prettyPrintPlatformInfo( const cl_platform_id& pId )
{
	size_t platformProfileSize	= 0;
	OPENCL_V_THROW( ::clGetPlatformInfo( pId, CL_PLATFORM_PROFILE, 0, NULL, &platformProfileSize ),
		"Getting CL_PLATFORM_PROFILE Platform Info string size ( ::clGetPlatformInfo() )" );

	std::vector< char > szPlatformProfile( platformProfileSize );
	OPENCL_V_THROW( ::clGetPlatformInfo( pId, CL_PLATFORM_PROFILE, platformProfileSize, &szPlatformProfile[ 0 ], NULL),
		"Getting CL_PLATFORM_PROFILE Platform Info string ( ::clGetPlatformInfo() )" );

	size_t platformVersionSize	= 0;
	OPENCL_V_THROW( ::clGetPlatformInfo( pId, CL_PLATFORM_VERSION, 0, NULL, &platformVersionSize ),
		"Getting CL_PLATFORM_VERSION Platform Info string size ( ::clGetPlatformInfo() )" );

	std::vector< char > szPlatformVersion( platformVersionSize );
	OPENCL_V_THROW( ::clGetPlatformInfo( pId, CL_PLATFORM_VERSION, platformVersionSize, &szPlatformVersion[ 0 ], NULL),
		"Getting CL_PLATFORM_VERSION Platform Info string ( ::clGetPlatformInfo() )" );

	size_t platformNameSize	= 0;
	OPENCL_V_THROW( ::clGetPlatformInfo( pId, CL_PLATFORM_NAME, 0, NULL, &platformNameSize ),
		"Getting CL_PLATFORM_NAME Platform Info string size ( ::clGetPlatformInfo() )" );

	std::vector< char > szPlatformName( platformNameSize );
	OPENCL_V_THROW( ::clGetPlatformInfo( pId, CL_PLATFORM_NAME, platformNameSize, &szPlatformName[ 0 ], NULL),
		"Getting CL_PLATFORM_NAME Platform Info string ( ::clGetPlatformInfo() )" );

	size_t vendorStringSize	= 0;
	OPENCL_V_THROW( ::clGetPlatformInfo( pId, CL_PLATFORM_VENDOR, 0, NULL, &vendorStringSize ),
		"Getting CL_PLATFORM_VENDOR Platform Info string size ( ::clGetPlatformInfo() )" );

	std::vector< char > szPlatformVendor( vendorStringSize );
	OPENCL_V_THROW( ::clGetPlatformInfo( pId, CL_PLATFORM_VENDOR, vendorStringSize, &szPlatformVendor[ 0 ], NULL),
		"Getting CL_PLATFORM_VENDOR Platform Info string ( ::clGetPlatformInfo() )" );

	size_t platformExtensionsSize	= 0;
	OPENCL_V_THROW( ::clGetPlatformInfo( pId, CL_PLATFORM_EXTENSIONS, 0, NULL, &platformExtensionsSize ),
		"Getting CL_PLATFORM_EXTENSIONS Platform Info string size ( ::clGetPlatformInfo() )" );

	std::vector< char > szPlatformExtensions( platformExtensionsSize );
	OPENCL_V_THROW( ::clGetPlatformInfo( pId, CL_PLATFORM_EXTENSIONS, platformExtensionsSize, &szPlatformExtensions[ 0 ], NULL),
		"Getting CL_PLATFORM_EXTENSIONS Platform Info string ( ::clGetPlatformInfo() )" );

	const int indent = countOf( "    CL_PLATFORM_EXTENSIONS: " );
	std::cout << std::left << std::setw( indent ) << "    CL_PLATFORM_PROFILE: " << &szPlatformProfile[ 0 ] << std::endl;
	std::cout << std::left << std::setw( indent ) << "    CL_PLATFORM_VERSION: " << &szPlatformVersion[ 0 ] << std::endl;
	std::cout << std::left << std::setw( indent ) << "    CL_PLATFORM_NAME: " << &szPlatformName[ 0 ] << std::endl;
	std::cout << std::left << std::setw( indent ) << "    CL_PLATFORM_VENDOR: " << &szPlatformVendor[ 0 ] << std::endl;
	std::cout << std::left << std::setw( indent ) << "    CL_PLATFORM_EXTENSIONS: " << &szPlatformExtensions[ 0 ] << std::endl;
	std::cout << std::right << std::endl;
}

void prettyPrintDeviceInfo( const cl_device_id& dId )
{
	size_t deviceNameSize	= 0;
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DEVICE_NAME, 0, NULL, &deviceNameSize ),
		"Getting CL_DEVICE_NAME Platform Info string size ( ::clGetDeviceInfo() )" );

	std::vector< char > szDeviceName( deviceNameSize );
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DEVICE_NAME, deviceNameSize, &szDeviceName[ 0 ], NULL ),
		"Getting CL_DEVICE_NAME Platform Info string ( ::clGetDeviceInfo() )" );

	size_t deviceVersionSize	= 0;
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DEVICE_VERSION, 0, NULL, &deviceVersionSize ),
		"Getting CL_DEVICE_VERSION Platform Info string size ( ::clGetDeviceInfo() )" );

	std::vector< char > szDeviceVersion( deviceVersionSize );
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DEVICE_VERSION, deviceVersionSize, &szDeviceVersion[ 0 ], NULL ),
		"Getting CL_DEVICE_VERSION Platform Info string ( ::clGetDeviceInfo() )" );

	size_t driverVersionSize	= 0;
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DRIVER_VERSION, 0, NULL, &driverVersionSize ),
		"Getting CL_DRIVER_VERSION Platform Info string size ( ::clGetDeviceInfo() )" );

	std::vector< char > szDriverVersion( driverVersionSize );
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DRIVER_VERSION, driverVersionSize, &szDriverVersion[ 0 ], NULL ),
		"Getting CL_DRIVER_VERSION Platform Info string ( ::clGetDeviceInfo() )" );

	size_t openCLVersionSize	= 0;
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &openCLVersionSize ),
		"Getting CL_DEVICE_OPENCL_C_VERSION Platform Info string size ( ::clGetDeviceInfo() )" );

	std::vector< char > szOpenCLVersion( openCLVersionSize );
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DEVICE_OPENCL_C_VERSION, openCLVersionSize, &szOpenCLVersion[ 0 ], NULL ),
		"Getting CL_DEVICE_OPENCL_C_VERSION Platform Info string ( ::clGetDeviceInfo() )" );

	cl_device_type devType = CL_DEVICE_TYPE_DEFAULT;
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DEVICE_TYPE, sizeof( cl_device_type ), &devType, NULL ),
		"Getting CL_DEVICE_TYPE device info ( ::clGetDeviceInfo() )" );

	cl_uint devAddrBits = 0;
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DEVICE_ADDRESS_BITS, sizeof( cl_uint ), &devAddrBits, NULL ),
		"Getting CL_DEVICE_ADDRESS_BITS device info ( ::clGetDeviceInfo() )" );

	cl_uint maxClockFreq = 0;
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof( cl_uint ), &maxClockFreq, NULL ),
		"Getting CL_DEVICE_MAX_CLOCK_FREQUENCY device info ( ::clGetDeviceInfo() )" );

	cl_bool devAvailable = CL_FALSE;
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DEVICE_AVAILABLE, sizeof( cl_bool ), &devAvailable, NULL ),
		"Getting CL_DEVICE_AVAILABLE device info ( ::clGetDeviceInfo() )" );

	cl_bool devCompAvailable = CL_FALSE;
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DEVICE_COMPILER_AVAILABLE, sizeof( cl_bool ), &devCompAvailable, NULL ),
		"Getting CL_DEVICE_COMPILER_AVAILABLE device info ( ::clGetDeviceInfo() )" );

	size_t devMaxWorkGroup	= 0;
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof( size_t ), &devMaxWorkGroup, NULL ),
		"Getting CL_DEVICE_MAX_WORK_GROUP_SIZE device info ( ::clGetDeviceInfo() )" );

	cl_uint devMaxWorkItemDim = CL_FALSE;
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof( cl_uint ), &devMaxWorkItemDim, NULL ),
		"Getting CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS device info ( ::clGetDeviceInfo() )" );

	std::vector< size_t >	devMaxWorkItemSizes( devMaxWorkItemDim );
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof( size_t )*devMaxWorkItemSizes.size( ), &devMaxWorkItemSizes[0], NULL),
		"Getting CL_DEVICE_MAX_WORK_ITEM_SIZES device info ( ::clGetDeviceInfo() )" );

	cl_bool deviceHostUnified = 0;
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DEVICE_HOST_UNIFIED_MEMORY, sizeof( cl_bool ), &deviceHostUnified, NULL ),
		"Getting CL_DEVICE_HOST_UNIFIED_MEMORY Platform Info string ( ::clGetDeviceInfo() )" );

	cl_ulong devMaxConstantBuffer	= 0;
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof( cl_ulong ), &devMaxConstantBuffer, NULL ),
		"Getting CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE device info ( ::clGetDeviceInfo() )" );

	cl_ulong devLocalMemSize	= 0;
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DEVICE_LOCAL_MEM_SIZE, sizeof( cl_ulong ), &devLocalMemSize, NULL ),
		"Getting CL_DEVICE_LOCAL_MEM_SIZE device info ( ::clGetDeviceInfo() )" );

	cl_ulong deviceGlobalMemSize = 0;
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof( cl_ulong ), &deviceGlobalMemSize, NULL ),
		"Getting CL_DEVICE_GLOBAL_MEM_SIZE device info ( ::clGetDeviceInfo() )" );

	cl_ulong deviceMaxMemAllocSize = 0;
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof( cl_ulong ), &deviceMaxMemAllocSize, NULL ),
		"Getting CL_DEVICE_MAX_MEM_ALLOC_SIZE device info ( ::clGetDeviceInfo() )" );

	size_t deviceExtSize	= 0;
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DEVICE_EXTENSIONS, 0, NULL, &deviceExtSize ),
		"Getting CL_DEVICE_EXTENSIONS Platform Info string size ( ::clGetDeviceInfo() )" );

	std::vector< char > szDeviceExt( deviceExtSize );
	OPENCL_V_THROW( ::clGetDeviceInfo( dId, CL_DEVICE_EXTENSIONS, deviceExtSize, &szDeviceExt[ 0 ], NULL ),
		"Getting CL_DEVICE_EXTENSIONS Platform Info string ( ::clGetDeviceInfo() )" );

	const int indent = countOf( "    CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS: " );
	std::cout << std::left << std::setw( indent ) << "    CL_DEVICE_NAME: " << &szDeviceName[ 0 ] << std::endl;
	std::cout << std::left << std::setw( indent ) << "    CL_DEVICE_VERSION: " << &szDeviceVersion[ 0 ] << std::endl;
	std::cout << std::left << std::setw( indent ) << "    CL_DRIVER_VERSION: " << &szDriverVersion[ 0 ] << std::endl;
	std::cout << std::left << std::setw( indent ) << "    CL_DEVICE_TYPE: "
		<< (CL_DEVICE_TYPE_DEFAULT     & devType ? "default"     : "")
		<< (CL_DEVICE_TYPE_CPU         & devType ? "CPU"         : "")
		<< (CL_DEVICE_TYPE_GPU         & devType ? "GPU"         : "")
		<< (CL_DEVICE_TYPE_ACCELERATOR & devType ? "Accelerator" : "")
		<< std::endl;
	std::cout << std::left << std::setw( indent ) << "    CL_DEVICE_MAX_CLOCK_FREQUENCY: " << maxClockFreq << std::endl;
	std::cout << std::left << std::setw( indent ) << "    CL_DEVICE_ADDRESS_BITS: " << devAddrBits << std::endl;
	std::cout << std::left << std::setw( indent ) << "    CL_DEVICE_AVAILABLE: " << ( devAvailable ? "TRUE": "FALSE") << std::endl;
	std::cout << std::left << std::setw( indent ) << "    CL_DEVICE_COMPILER_AVAILABLE: " << ( devCompAvailable ? "TRUE": "FALSE") << std::endl;
	std::cout << std::left << std::setw( indent ) << "    CL_DEVICE_OPENCL_C_VERSION: " << &szOpenCLVersion[ 0 ] << std::endl;
	std::cout << std::left << std::setw( indent ) << "    CL_DEVICE_MAX_WORK_GROUP_SIZE: " << devMaxWorkGroup << std::endl;
	std::cout << std::left << std::setw( indent ) << "    CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS: " << devMaxWorkItemDim << std::endl;
	for( cl_uint wis = 0; wis < devMaxWorkItemSizes.size( ); ++wis )
	{
		std::stringstream dimString;
		dimString << "Dimension[ " << wis << " ]  ";
		std::cout << std::right << std::setw( indent ) << dimString.str( ) << devMaxWorkItemSizes[wis] << std::endl;
	}
	std::cout << std::left << std::setw( indent ) << "    CL_DEVICE_HOST_UNIFIED_MEMORY: " << ( deviceHostUnified ? "TRUE": "FALSE") << std::endl;
	std::cout << std::left << std::setw( indent ) << "    CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE: " << devMaxConstantBuffer;
	std::cout << " ( " << devMaxConstantBuffer / 1024 << " KB )" << std::endl;
	std::cout << std::left << std::setw( indent ) << "    CL_DEVICE_LOCAL_MEM_SIZE: " << devLocalMemSize;
	std::cout << " ( " << devLocalMemSize / 1024 << " KB )" << std::endl;
	std::cout << std::left << std::setw( indent ) << "    CL_DEVICE_GLOBAL_MEM_SIZE: " << deviceGlobalMemSize;
	std::cout << " ( " << deviceGlobalMemSize / 1048576 << " MB )" << std::endl;
	std::cout << std::left << std::setw( indent ) << "    CL_DEVICE_MAX_MEM_ALLOC_SIZE: " << deviceMaxMemAllocSize;
	std::cout << " ( " << deviceMaxMemAllocSize / 1048576 << " MB )" << std::endl;
	std::cout << std::left << std::setw( indent ) << "    CL_DEVICE_EXTENSIONS: " << &szDeviceExt[ 0 ] << std::endl;

	std::cout << std::right << std::endl;
}

//	Verify a failed condition; return true on fail
inline cl_bool OPENCL_V_FAIL( cl_int res )
{
	if( res == CL_SUCCESS )
		return CL_FALSE;
	else
		return CL_TRUE;
}

std::string prettyPrintclFFTStatus( const cl_int& status )
{
	switch( status )
	{
		case CLFFT_INVALID_GLOBAL_WORK_SIZE:
			return "CLFFT_INVALID_GLOBAL_WORK_SIZE";
		case CLFFT_INVALID_MIP_LEVEL:
			return "CLFFT_INVALID_MIP_LEVEL";
		case CLFFT_INVALID_BUFFER_SIZE:
			return "CLFFT_INVALID_BUFFER_SIZE";
		case CLFFT_INVALID_GL_OBJECT:
			return "CLFFT_INVALID_GL_OBJECT";
		case CLFFT_INVALID_OPERATION:
			return "CLFFT_INVALID_OPERATION";
		case CLFFT_INVALID_EVENT:
			return "CLFFT_INVALID_EVENT";
		case CLFFT_INVALID_EVENT_WAIT_LIST:
			return "CLFFT_INVALID_EVENT_WAIT_LIST";
		case CLFFT_INVALID_GLOBAL_OFFSET:
			return "CLFFT_INVALID_GLOBAL_OFFSET";
		case CLFFT_INVALID_WORK_ITEM_SIZE:
			return "CLFFT_INVALID_WORK_ITEM_SIZE";
		case CLFFT_INVALID_WORK_GROUP_SIZE:
			return "CLFFT_INVALID_WORK_GROUP_SIZE";
		case CLFFT_INVALID_WORK_DIMENSION:
			return "CLFFT_INVALID_WORK_DIMENSION";
		case CLFFT_INVALID_KERNEL_ARGS:
			return "CLFFT_INVALID_KERNEL_ARGS";
		case CLFFT_INVALID_ARG_SIZE:
			return "CLFFT_INVALID_ARG_SIZE";
		case CLFFT_INVALID_ARG_VALUE:
			return "CLFFT_INVALID_ARG_VALUE";
		case CLFFT_INVALID_ARG_INDEX:
			return "CLFFT_INVALID_ARG_INDEX";
		case CLFFT_INVALID_KERNEL:
			return "CLFFT_INVALID_KERNEL";
		case CLFFT_INVALID_KERNEL_DEFINITION:
			return "CLFFT_INVALID_KERNEL_DEFINITION";
		case CLFFT_INVALID_KERNEL_NAME:
			return "CLFFT_INVALID_KERNEL_NAME";
		case CLFFT_INVALID_PROGRAM_EXECUTABLE:
			return "CLFFT_INVALID_PROGRAM_EXECUTABLE";
		case CLFFT_INVALID_PROGRAM:
			return "CLFFT_INVALID_PROGRAM";
		case CLFFT_INVALID_BUILD_OPTIONS:
			return "CLFFT_INVALID_BUILD_OPTIONS";
		case CLFFT_INVALID_BINARY:
			return "CLFFT_INVALID_BINARY";
		case CLFFT_INVALID_SAMPLER:
			return "CLFFT_INVALID_SAMPLER";
		case CLFFT_INVALID_IMAGE_SIZE:
			return "CLFFT_INVALID_IMAGE_SIZE";
		case CLFFT_INVALID_IMAGE_FORMAT_DESCRIPTOR:
			return "CLFFT_INVALID_IMAGE_FORMAT_DESCRIPTOR";
		case CLFFT_INVALID_MEM_OBJECT:
			return "CLFFT_INVALID_MEM_OBJECT";
		case CLFFT_INVALID_HOST_PTR:
			return "CLFFT_INVALID_HOST_PTR";
		case CLFFT_INVALID_COMMAND_QUEUE:
			return "CLFFT_INVALID_COMMAND_QUEUE";
		case CLFFT_INVALID_QUEUE_PROPERTIES:
			return "CLFFT_INVALID_QUEUE_PROPERTIES";
		case CLFFT_INVALID_CONTEXT:
			return "CLFFT_INVALID_CONTEXT";
		case CLFFT_INVALID_DEVICE:
			return "CLFFT_INVALID_DEVICE";
		case CLFFT_INVALID_PLATFORM:
			return "CLFFT_INVALID_PLATFORM";
		case CLFFT_INVALID_DEVICE_TYPE:
			return "CLFFT_INVALID_DEVICE_TYPE";
		case CLFFT_INVALID_VALUE:
			return "CLFFT_INVALID_VALUE";
		case CLFFT_MAP_FAILURE:
			return "CLFFT_MAP_FAILURE";
		case CLFFT_BUILD_PROGRAM_FAILURE:
			return "CLFFT_BUILD_PROGRAM_FAILURE";
		case CLFFT_IMAGE_FORMAT_NOT_SUPPORTED:
			return "CLFFT_IMAGE_FORMAT_NOT_SUPPORTED";
		case CLFFT_IMAGE_FORMAT_MISMATCH:
			return "CLFFT_IMAGE_FORMAT_MISMATCH";
		case CLFFT_MEM_COPY_OVERLAP:
			return "CLFFT_MEM_COPY_OVERLAP";
		case CLFFT_PROFILING_INFO_NOT_AVAILABLE:
			return "CLFFT_PROFILING_INFO_NOT_AVAILABLE";
		case CLFFT_OUT_OF_HOST_MEMORY:
			return "CLFFT_OUT_OF_HOST_MEMORY";
		case CLFFT_OUT_OF_RESOURCES:
			return "CLFFT_OUT_OF_RESOURCES";
		case CLFFT_MEM_OBJECT_ALLOCATION_FAILURE:
			return "CLFFT_MEM_OBJECT_ALLOCATION_FAILURE";
		case CLFFT_COMPILER_NOT_AVAILABLE:
			return "CLFFT_COMPILER_NOT_AVAILABLE";
		case CLFFT_DEVICE_NOT_AVAILABLE:
			return "CLFFT_DEVICE_NOT_AVAILABLE";
		case CLFFT_DEVICE_NOT_FOUND:
			return "CLFFT_DEVICE_NOT_FOUND";
		case CLFFT_SUCCESS:
			return "CLFFT_SUCCESS";
		case CLFFT_NOTIMPLEMENTED:
			return "CLFFT_NOTIMPLEMENTED";
		case CLFFT_FILE_NOT_FOUND:
			return "CLFFT_FILE_NOT_FOUND";
		case CLFFT_FILE_CREATE_FAILURE:
			return "CLFFT_FILE_CREATE_FAILURE";
		case CLFFT_VERSION_MISMATCH:
			return "CLFFT_VERSION_MISMATCH";
		case CLFFT_INVALID_PLAN:
			return "CLFFT_INVALID_PLAN";
		default:
			return "Error code not defined";
		break;
	}
}

std::vector< cl_device_id > initializeCL( cl_device_type deviceType,
										  cl_uint deviceGpuList,
										  cl_context& context,
										  bool printclInfo )
{
	cl_int status = 0;

	/*
		* Have a look at the available platforms and pick either
		* the AMD one if available or a reasonable default.
		*/

	cl_uint numPlatforms	= 0;
	cl_platform_id platform = NULL;
	OPENCL_V_THROW( ::clGetPlatformIDs( 0, NULL, &numPlatforms ),
			"Getting number of platforms( ::clGetPlatformsIDs() )" );

	if( numPlatforms > 0 )
	{
		std::vector< cl_platform_id > platforms( numPlatforms );
		OPENCL_V_THROW( ::clGetPlatformIDs( numPlatforms, &platforms[ 0 ], NULL ),
			"Getting Platform Id's ( ::clGetPlatformsIDs() )" );

		//	TODO: How should we determine what platform to choose?  We are just defaulting to the last one reported, as we
		//	print out the info
		for( unsigned int i=0; i < numPlatforms; ++i )
		{
			if( printclInfo )
			{
				std::cout << "OpenCL platform [ " << i << " ]:" << std::endl;
				prettyPrintPlatformInfo( platforms[i] );
			}

			platform = platforms[i];
		}
	}

	if( NULL == platform )
	{
		throw std::runtime_error( "No appropriate OpenCL platform could be found" );
	}

	/*
	 * If we could find our platform, use it. Otherwise use just available platform.
	 */

	//	Get the device list for this type.
	//
	cl_uint num_devices = 0;
	OPENCL_V_THROW( ::clGetDeviceIDs( platform, deviceType, 0, NULL, &num_devices ),
		"Getting OpenCL devices ( ::clGetDeviceIDs() )" );
	if( 0 == num_devices )
	{
		OPENCL_V_THROW( CLFFT_DEVICE_NOT_AVAILABLE, "No devices available");
	}

	std::vector< cl_device_id > deviceIDs( num_devices );
	OPENCL_V_THROW( ::clGetDeviceIDs( platform, deviceType, num_devices, &deviceIDs[0], NULL),
		"Getting OpenCL deviceIDs ( ::clGetDeviceIDs() )" );

	if( (CL_DEVICE_TYPE_GPU == deviceType) && (~cl_uint(0) != deviceGpuList) )
	{
		//	The command line options specify to user certain gpu(s)
		//
		for( unsigned u = (unsigned) deviceIDs.size(); u-- > 0; )
		{
			if( 0 != (deviceGpuList & (1<<u) ) )
				continue;

			//  Remove this GPU from the list
			deviceIDs[u] = deviceIDs.back();
			deviceIDs.pop_back();
		}
	}

	if( 0 == deviceIDs.size( ) )
	{
		OPENCL_V_THROW( CLFFT_DEVICE_NOT_AVAILABLE, "No devices available");
	}

	cl_context_properties cps[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	/////////////////////////////////////////////////////////////////
	// Create an OpenCL context
	/////////////////////////////////////////////////////////////////
	context = clCreateContext( cps,
							   (cl_uint) deviceIDs.size(),
							   & deviceIDs[0],
							   NULL,
							   NULL,
							   &status);
	OPENCL_V_THROW( status, "Creating Context ( ::clCreateContextFromType() )" );

	/* First, get the size of device list data */
	size_t deviceListSize;
	OPENCL_V_THROW( ::clGetContextInfo( context, CL_CONTEXT_DEVICES, 0, NULL, &deviceListSize ),
		"Getting device array size ( ::clGetContextInfo() )" );

	/////////////////////////////////////////////////////////////////
	// Detect OpenCL devices
	/////////////////////////////////////////////////////////////////
	std::vector< cl_device_id > devices( deviceListSize/sizeof( cl_device_id ) );

	/* Now, get the device list data */
	OPENCL_V_THROW( ::clGetContextInfo( context, CL_CONTEXT_DEVICES, deviceListSize, &devices[ 0 ], NULL ),
		"Getting device array ( ::clGetContextInfo() )" );

	if( printclInfo )
	{
		cl_uint cContextDevices	= 0;
		
		size_t deviceVersionSize	= 0;
		OPENCL_V_THROW( ::clGetDeviceInfo( devices[0], CL_DEVICE_VERSION, 0, NULL, &deviceVersionSize ),
			"Getting CL_DEVICE_VERSION Platform Info string size ( ::clGetDeviceInfo() )" );

		std::vector< char > szDeviceVersion( deviceVersionSize );
		OPENCL_V_THROW( ::clGetDeviceInfo( devices[0], CL_DEVICE_VERSION, deviceVersionSize, &szDeviceVersion[ 0 ], NULL ),
			"Getting CL_DEVICE_VERSION Platform Info string ( ::clGetDeviceInfo() )" );
	
		char openclstr[11]="OpenCL 1.0";

		if (!strncmp((const char*)&szDeviceVersion[ 0 ], openclstr, 10))
		{
			cContextDevices	= 1;
		}
		else
		{
			OPENCL_V_THROW( ::clGetContextInfo( context, CL_CONTEXT_NUM_DEVICES, sizeof( cContextDevices ), &cContextDevices, NULL ),
				"Getting number of context devices ( ::clGetContextInfo() )" );
		}

		for( cl_uint i = 0; i < cContextDevices; ++i )
		{
			std::cout << "OpenCL devices [ " << i << " ]:" << std::endl;
			prettyPrintDeviceInfo( devices[i] );
		}
	}

	return devices;
}

int cleanupCL( cl_context* context, cl_command_queue* commandQueue,
	const cl_uint numBuffersIn, cl_mem inputBuffer[], const cl_uint numBuffersOut, cl_mem outputBuffer[], cl_event* outEvent )
{
	if( *outEvent != NULL )
		OPENCL_V_THROW( clReleaseEvent( *outEvent ), "Error: In clReleaseEvent\n" );

	releaseOpenCLMemBuffer( numBuffersIn, inputBuffer);
	releaseOpenCLMemBuffer( numBuffersOut, outputBuffer);

	if( *commandQueue != NULL )
		OPENCL_V_THROW( clReleaseCommandQueue( *commandQueue ), "Error: In clReleaseCommandQueue\n" );

	if( *context != NULL )
		OPENCL_V_THROW( clReleaseContext( *context ), "Error: In clReleaseContext\n" );

	return 0;
}

int createOpenCLMemoryBuffer( cl_context& context, const size_t bufferSizeBytes, const cl_uint numBuffers, cl_mem buffer[], cl_mem_flags accessibility) {
	cl_int status = 0;

	for( cl_uint i = 0; i < numBuffers; ++i )
	{
		buffer[ i ] = ::clCreateBuffer( context, accessibility, bufferSizeBytes, NULL, &status);
		OPENCL_V_THROW( status, "Creating Buffer ( ::clCreateBuffer() )" );
	}

	return 0;
}

int releaseOpenCLMemBuffer( const cl_uint numBuffers, cl_mem buffer[])
{
	for( cl_uint i = 0; i < numBuffers; ++i )
	{
		if( buffer[ i ] != NULL )
			OPENCL_V_THROW( clReleaseMemObject( buffer[ i ] ), "Error: In clReleaseMemObject\n" );
	}

	return 0;
}

void createOpenCLCommandQueue( cl_context& context,
							   cl_uint commandQueueFlags,
							   cl_command_queue& commandQueue,
							   std::vector< cl_device_id > devices,
							   const size_t bufferSizeBytesIn,
							   const cl_uint numBuffersIn,
							   cl_mem clMemBufferIn[],
							   const size_t bufferSizeBytesOut,
							   const cl_uint numBuffersOut,
							   cl_mem clMemBufferOut[] )
{
	cl_int status = 0;
	commandQueue = ::clCreateCommandQueue( context, devices[0], commandQueueFlags, &status );
	OPENCL_V_THROW( status, "Creating Command Queue ( ::clCreateCommandQueue() )" );

	createOpenCLMemoryBuffer( context, bufferSizeBytesIn,  numBuffersIn,  clMemBufferIn,  CL_MEM_READ_WRITE);
	createOpenCLMemoryBuffer( context, bufferSizeBytesOut, numBuffersOut, clMemBufferOut, CL_MEM_READ_WRITE);
}

