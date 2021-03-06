#include "compress.hpp"

#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>

#include "helper.hpp"

using namespace std;

/**
* Retrieve GPU device
*/
void gpu_find(cl_device_id &device, uint platform_select, uint device_select)
{

}

TextureCompressor::TextureCompressor() {

    uint platform_select = 0;
    uint device_select = 0;

    cout << "[DEBUG]Finding device for pair " << platform_select << ", " << device_select << endl;
    cl_platform_id platform;
    cl_uint platform_num = 0;
    cl_uint device_num = 0;

    size_t attr_size = 0;
    cl_char* attr_data = NULL;

    /* get num of available OpenCL platforms */
    CL_ERR( clGetPlatformIDs(0, NULL, &platform_num));
    this->platform_ids = new cl_platform_id[platform_num];
    DIE(this->platform_ids == NULL, "alloc platform_list");

    /* get all available OpenCL platforms */
    CL_ERR( clGetPlatformIDs(platform_num, this->platform_ids, NULL));
    cout << "Platforms found: " << platform_num << endl;

    /* list all platforms and VENDOR/VERSION properties */
    for(uint platf=0; platf<platform_num; platf++)
    {
        /* get attribute CL_PLATFORM_VENDOR */
        CL_ERR( clGetPlatformInfo(this->platform_ids[platf],
                                  CL_PLATFORM_VENDOR, 0, NULL, &attr_size));
        attr_data = new cl_char[attr_size];
        DIE(attr_data == NULL, "alloc attr_data");

        /* get data CL_PLATFORM_VENDOR */
        CL_ERR( clGetPlatformInfo(this->platform_ids[platf],
                                  CL_PLATFORM_VENDOR, attr_size, attr_data, NULL));
        cout << "Platform " << platf << " " << attr_data << " ";
        delete[] attr_data;

        /* get attribute size CL_PLATFORM_VERSION */
        CL_ERR( clGetPlatformInfo(this->platform_ids[platf],
                                  CL_PLATFORM_VERSION, 0, NULL, &attr_size));
        attr_data = new cl_char[attr_size];
        DIE(attr_data == NULL, "alloc attr_data");

        /* get data size CL_PLATFORM_VERSION */
        CL_ERR( clGetPlatformInfo(this->platform_ids[platf],
                                  CL_PLATFORM_VERSION, attr_size, attr_data, NULL));
        cout << attr_data << endl;
        delete[] attr_data;

        /* no valid platform found */
        platform = this->platform_ids[platf];
        DIE(platform == 0, "platform selection");

        /* get num of available OpenCL devices type ALL on the selected platform */
        if(clGetDeviceIDs(platform,
                          CL_DEVICE_TYPE_ALL, 0, NULL, &device_num) == CL_DEVICE_NOT_FOUND) {
            device_num = 0;
            cout << "[DEBUG]No devices found, continuing" << endl;
            continue;
        }
        cout << "[DEBUG]Devices found" << endl;
        this->device_ids = new cl_device_id[device_num];
        DIE(this->device_ids == NULL, "alloc devices");

        /* get all available OpenCL devices type ALL on the selected platform */
        CL_ERR( clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL,
                               device_num, this->device_ids, NULL));
        cout << "\tDevices found " << device_num  << endl;

        /* list all devices and TYPE/VERSION properties */
        for(uint dev=0; dev<device_num; dev++)
        {
            /* get attribute size */
            CL_ERR( clGetDeviceInfo(this->device_ids[dev], CL_DEVICE_NAME,
                                    0, NULL, &attr_size));
            attr_data = new cl_char[attr_size];
            DIE(attr_data == NULL, "alloc attr_data");

            /* get attribute CL_DEVICE_NAME */
            CL_ERR( clGetDeviceInfo(this->device_ids[dev], CL_DEVICE_NAME,
                                    attr_size, attr_data, NULL));
            cout << "\tDevice " << dev << " " << attr_data << " ";
            delete[] attr_data;

            /* get attribute size */
            CL_ERR( clGetDeviceInfo(this->device_ids[dev], CL_DEVICE_VERSION,
                                    0, NULL, &attr_size));
            attr_data = new cl_char[attr_size];
            DIE(attr_data == NULL, "alloc attr_data");

            /* get attribute CL_DEVICE_VERSION */
            CL_ERR( clGetDeviceInfo(this->device_ids[dev], CL_DEVICE_VERSION,
                                    attr_size, attr_data, NULL));
            cout << attr_data;
            delete[] attr_data;

            /* select device based on cli arguments */
            if((platf == platform_select) && (dev == device_select)){
                this->device = this->device_ids[dev];
                cout << " <--- SELECTED ";
            }

            cout << endl;
        }
    }

    DIE(this->device == 0, "check valid device");
}

TextureCompressor::~TextureCompressor() { }	// destructor


/**
* Profile kernel execution using the selected device
*/
void gpu_profile_kernel(cl_device_id device, const uint8_t* src, uint8_t* dst, int width, int height)
{

	cout << "[DEBUG]Profiling kernel" << endl;
	cl_int ret;

	cl_context context;
	cl_command_queue cmdQueue;
	cl_program program;
	cl_kernel kernel;

	string kernel_src;

	double timeDiff = 0;
	cout << "[DEBUG]Creating context for device " << endl;
	/* create a context for the device */
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &ret);
	cout << "[DEBUG]Creating context[END]" << endl;
	CL_ERR( ret );

	cout << "[DEBUG]Creating cmd" << endl;
	/* create a command queue for the device in the context */
	cmdQueue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &ret);
	CL_ERR( ret );

	cout << "[DEBUG]Allocating buff on CPU" << endl;
	/* allocate 1 buffer of BUF_128 float elements on the CPU (HOST) */
    int srcSize = width * height;
    int dstSize = width * height;
    cl_uchar *srcBuf = new cl_uchar[srcSize];
    cl_uchar *dstBuf = new cl_uchar[dstSize];

	/* assert HOST (CPU/RAM) allocation */
    DIE ( srcBuf == NULL, "src alloc failed" );
    DIE ( dstBuf == NULL, "dst alloc failed" );

	cout << "[DEBUG]Filling data" << endl;
	/* fill data */
	for (int i = 0; i < srcSize; i++) {
        srcBuf[i] = src[i];
	}

	cout << "[DEBUG]Allocating buffers on device" << endl;
    /* allocate buffer on the DEVICE (GPU/VRAM) */
    cl_mem devSrcBuf = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_uchar) * srcSize, NULL, &ret);
    CL_ERR( ret );
    cl_mem devDstBuf = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_uchar) * dstSize, NULL, &ret);
    CL_ERR( ret );

	cout << "[DEBUG]Writing buffer src" << endl;
    /* copy the buffers back */
    CL_ERR( clEnqueueWriteBuffer(cmdQueue, devSrcBuf, CL_TRUE, 0,
                                 sizeof(uint8_t) * srcSize, srcBuf, 0, NULL, NULL));

	cout << "[DEBUG]Reading kernel" << endl;
	/* retrieve kernel source */
	read_kernel("compress.cl", kernel_src);
	const char* kernel_c_str = kernel_src.c_str();

	cout << "[DEBUG]Creating Program" << endl;
	/* create kernel program from source */
	program = clCreateProgramWithSource(context, 1, &kernel_c_str, NULL, &ret);
	CL_ERR( ret );

	/* compile the program for the given set of devices */
	ret = clBuildProgram(program, 1, &device, "-cl-fast-relaxed-math", NULL, NULL);
	CL_COMPILE_ERR( ret, program, device );

	/* create kernel associated to compiled source kernel */
	kernel = clCreateKernel(program, "compress_func", &ret);
	CL_ERR( ret );

	cout << "[DEBUG]Setting arguments" << endl;
	/* set OpenCL kernel argument */
	CL_ERR( clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&devSrcBuf) );
	CL_ERR( clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&devDstBuf) );
	CL_ERR( clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *)&width) );
	CL_ERR( clSetKernelArg(kernel, 3, sizeof(cl_uint), (void *)&height) );

	/* TIME - start */
	auto start = std::chrono::system_clock::now();

	cout << "[DEBUG]Profile exec" << endl;
	/* profile execution of OpenCL kernel */
	cl_event event;
	size_t globalSize[2] = {(size_t)height, (size_t)width};
	ret = clEnqueueNDRangeKernel(cmdQueue, kernel, 2, NULL,
								 globalSize, 0, 0, NULL, &event);
	CL_ERR( ret );
	CL_ERR( clWaitForEvents(1, &event));

	/* TIME - end */
	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
	std::cout << "Kernel exec time (chrono, ms): " << elapsed.count() / 1000.f << '\n';
	//std::cout << "Gflops: " << (double) bufMatSize * matSize * 2 / elapsed.count() << endl;

	/* copy the buffers back */
	CL_ERR( clEnqueueReadBuffer(cmdQueue, devDstBuf, CL_TRUE, 0,
                                sizeof(uint8_t) * dstSize, dstBuf, 0, NULL, NULL));

	cout << "[DEBUG]Waiting for enqueued" << endl;
	/* wait for all enqueued operations to finish */
	CL_ERR( clFinish(cmdQueue) );

    /*for (int i=0; i<5; i++) {
        cout << "Element " << devDstBuf[i] << endl;
    }*/

	/* compute kernel execution time */
	cl_ulong timeStart, timeEnd;
	CL_ERR( clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(timeStart), &timeStart, NULL));
	CL_ERR( clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(timeEnd), &timeEnd, NULL));
	timeDiff = timeEnd - timeStart;
	cout << "Kernel exec time (opencl, ms): " << timeDiff / 1000.f << endl;

	cout << "[DEBUG]Freeing resources" << endl;
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

	/* perform kernel profile using selected device (GPU NVIDIA TESLA) */
	gpu_profile_kernel(device, src, dst, width, height);

	return 0;
}
