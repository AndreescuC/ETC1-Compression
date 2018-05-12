#include "compress.hpp"

#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>

#include "helper.hpp"

using namespace std;

TextureCompressor::TextureCompressor() { } 	// constructor/Users/grigore.lupescu/Desktop/RESEARCH/asc/teme/tema3/2018/Tema3-schelet/src/compress.cpp
TextureCompressor::~TextureCompressor() { }	// destructor

/**
* Retrieve GPU device
*/
void gpu_find(cl_device_id &device, uint platform_select, uint device_select)
{
	cl_platform_id platform;
	cl_uint platform_num = 0;
	cl_platform_id* platform_list = NULL;

	cl_uint device_num = 0;
	cl_device_id* device_list = NULL;

	size_t attr_size = 0;
	cl_char* attr_data = NULL;

	/* get num of available OpenCL platforms */
	CL_ERR( clGetPlatformIDs(0, NULL, &platform_num));
	platform_list = new cl_platform_id[platform_num];
	DIE(platform_list == NULL, "alloc platform_list");

	/* get all available OpenCL platforms */
	CL_ERR( clGetPlatformIDs(platform_num, platform_list, NULL));
	cout << "Platforms found: " << platform_num << endl;

	/* list all platforms and VENDOR/VERSION properties */
	for(uint platf=0; platf<platform_num; platf++)
	{
		/* get attribute CL_PLATFORM_VENDOR */
		CL_ERR( clGetPlatformInfo(platform_list[platf],
								  CL_PLATFORM_VENDOR, 0, NULL, &attr_size));
		attr_data = new cl_char[attr_size];
		DIE(attr_data == NULL, "alloc attr_data");

		/* get data CL_PLATFORM_VENDOR */
		CL_ERR( clGetPlatformInfo(platform_list[platf],
								  CL_PLATFORM_VENDOR, attr_size, attr_data, NULL));
		cout << "Platform " << platf << " " << attr_data << " ";
		delete[] attr_data;

		/* get attribute size CL_PLATFORM_VERSION */
		CL_ERR( clGetPlatformInfo(platform_list[platf],
								  CL_PLATFORM_VERSION, 0, NULL, &attr_size));
		attr_data = new cl_char[attr_size];
		DIE(attr_data == NULL, "alloc attr_data");

		/* get data size CL_PLATFORM_VERSION */
		CL_ERR( clGetPlatformInfo(platform_list[platf],
								  CL_PLATFORM_VERSION, attr_size, attr_data, NULL));
		cout << attr_data << endl;
		delete[] attr_data;

		/* no valid platform found */
		platform = platform_list[platf];
		DIE(platform == 0, "platform selection");

		/* get num of available OpenCL devices type ALL on the selected platform */
		if(clGetDeviceIDs(platform,
						  CL_DEVICE_TYPE_ALL, 0, NULL, &device_num) == NULL) {
			device_num = 0;
			continue;
		}

		device_list = new cl_device_id[device_num];
		DIE(device_list == NULL, "alloc devices");

		/* get all available OpenCL devices type ALL on the selected platform */
		CL_ERR( clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL,
							   device_num, device_list, NULL));
		cout << "\tDevices found " << device_num  << endl;

		/* list all devices and TYPE/VERSION properties */
		for(uint dev=0; dev<device_num; dev++)
		{
			/* get attribute size */
			CL_ERR( clGetDeviceInfo(device_list[dev], CL_DEVICE_NAME,
									0, NULL, &attr_size));
			attr_data = new cl_char[attr_size];
			DIE(attr_data == NULL, "alloc attr_data");

			/* get attribute CL_DEVICE_NAME */
			CL_ERR( clGetDeviceInfo(device_list[dev], CL_DEVICE_NAME,
									attr_size, attr_data, NULL));
			cout << "\tDevice " << dev << " " << attr_data << " ";
			delete[] attr_data;

			/* get attribute size */
			CL_ERR( clGetDeviceInfo(device_list[dev], CL_DEVICE_VERSION,
									0, NULL, &attr_size));
			attr_data = new cl_char[attr_size];
			DIE(attr_data == NULL, "alloc attr_data");

			/* get attribute CL_DEVICE_VERSION */
			CL_ERR( clGetDeviceInfo(device_list[dev], CL_DEVICE_VERSION,
									attr_size, attr_data, NULL));
			cout << attr_data;
			delete[] attr_data;

			/* select device based on cli arguments */
			if((platf == platform_select) && (dev == device_select)){
				device = device_list[dev];
				cout << " <--- SELECTED ";
			}

			cout << endl;
		}
	}

	delete[] platform_list;
	delete[] device_list;
}

/**
* Profile kernel execution using the selected device
*/
void gpu_profile_kernel(cl_device_id device, const uint8_t* src, uint8_t* dst, int width, int height);
{
	cl_int ret;

	cl_context context;
	cl_command_queue cmdQueue;
	cl_program program;
	cl_kernel kernel;

	string kernel_src;

	double timeDiff = 0;

	/* create a context for the device */
	context = clCreateContext(0, 1, &device, NULL, NULL, &ret);
	CL_ERR( ret );

	/* create a command queue for the device in the context */
	cmdQueue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &ret);
	CL_ERR( ret );

	/* allocate 1 buffer of BUF_128 float elements on the CPU (HOST) */
    int srcSize = width * height;
    int dstSize = width * height;
    cl_uchar *srcBuf = new cl_uchar[srcSize];
    cl_uchar *dstBuf = new cl_uchar[dstSize];

	/* assert HOST (CPU/RAM) allocation */
    DIE ( srcBuf == NULL, "src alloc failed" );
    DIE ( dstBuf == NULL, "dst alloc failed" );

	/* fill data */
	for (int i = 0; i < srcSize; i++) {
        srcBuf[i] = src[i];
	}

    /* allocate buffer on the DEVICE (GPU/VRAM) */
    cl_mem devSrcBuf = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_uchar) * srcSize, NULL, &ret);
    CL_ERR( ret );
    cl_mem devDstBuf = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_uchar) * dstSize, NULL, &ret);
    CL_ERR( ret );

    /* copy the buffers back */
    CL_ERR( clEnqueueWriteBuffer(cmdQueue, devSrcBuf, CL_TRUE, 0,
                                 sizeof(uint8_t) * srcSize, srcBuf, 0, NULL, NULL));

	/* retrieve kernel source */
	read_kernel("compress.cl", kernel_src);
	const char* kernel_c_str = kernel_src.c_str();

	/* create kernel program from source */
	program = clCreateProgramWithSource(context, 1, &kernel_c_str, NULL, &ret);
	CL_ERR( ret );

	/* compile the program for the given set of devices */
	ret = clBuildProgram(program, 1, &device, "-cl-fast-relaxed-math", NULL, NULL);
	CL_COMPILE_ERR( ret, program, device );

	/* create kernel associated to compiled source kernel */
	kernel = clCreateKernel(program, "compress_func", &ret);
	CL_ERR( ret );

	/* set OpenCL kernel argument */
	CL_ERR( clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&devSrcBuf) );
	CL_ERR( clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&devDstBuf) );
	CL_ERR( clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *)&width) );
	CL_ERR( clSetKernelArg(kernel, 3, sizeof(cl_uint), (void *)&height) );

	/* TIME - start */
	auto start = std::chrono::system_clock::now();

	/* profile execution of OpenCL kernel */
	cl_event event;
    //TODO: dwqe
	size_t globalSize[2] = {(size_t)srcSize, (size_t)srcSize};
	ret = clEnqueueNDRangeKernel(cmdQueue, kernel, 2, NULL,
								 globalSize, 0, 0, NULL, &event);
	CL_ERR( ret );
	CL_ERR( clWaitForEvents(1, &event));

	/* TIME - end */
	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
	std::cout << "Kernel exec time (chrono, ms): " << elapsed.count() / 1000.f << '\n';
    //TODO: dwqe
	//std::cout << "Gflops: " << (double) bufMatSize * matSize * 2 / elapsed.count() << endl;

	/* copy the buffers back */
	CL_ERR( clEnqueueReadBuffer(cmdQueue, devDstBuf, CL_TRUE, 0,
                                sizeof(uint8_t) * dstSize, dstBuf, 0, NULL, NULL));

	/* wait for all enqueued operations to finish */
	CL_ERR( clFinish(cmdQueue) );

	/* compute kernel execution time */
	cl_ulong timeStart, timeEnd;
	CL_ERR( clGetEventProfilingInfo(event,
									CL_PROFILING_COMMAND_START, sizeof(timeStart), &timeStart, NULL));
	CL_ERR( clGetEventProfilingInfo(event,
									CL_PROFILING_COMMAND_END, sizeof(timeEnd), &timeEnd, NULL));
	timeDiff = timeEnd - timeStart;
	cout << "Kernel exec time (opencl, ms): " << timeDiff / 1000.f << endl;

	/* free all resources related to GPU */
	CL_ERR( clReleaseMemObject(devSrcBuf) );
	CL_ERR( clReleaseMemObject(devDstBuf) );
	CL_ERR( clReleaseCommandQueue(cmdQueue) );
	CL_ERR( clReleaseContext(context) );

	/* free all resources on CPU */
	delete[] srcBuf;
	delete[] dstBuf;
}

unsigned long TextureCompressor::compress(const uint8_t* src,
									  uint8_t* dst,
									  int width,
									  int height)
{
	cl_device_id device;
	int platform_select = 0;
	int device_select = 0;

	if(argc == 3){
		platform_select = atoi(argv[1]);
		device_select = atoi(argv[2]);
	} else {
		cout << "./bin <platform> <device>" << endl << endl;
	}

	/* list selected platform and devices */
	cout << "SELECT(platform = " << platform_select
		 << ", device = " << device_select << ")" << endl;

	/* search and select platform/devices in OpenCL */
	gpu_find(device, platform_select, device_select);

	/* perform kernel profile using selected device (GPU NVIDIA TESLA) */
	gpu_profile_kernel(device, src, dst, width, height);

	return 0;
	return 0;
}
