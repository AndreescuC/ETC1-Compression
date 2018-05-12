// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_RASTER_TEXTURE_COMPRESSOR_H_
#define CC_RASTER_TEXTURE_COMPRESSOR_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#if __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define NOTREACHED()

union Color {
	struct BgraColorType {
		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t a;
	} channels;
	uint8_t components[4];
	uint32_t bits;
};

// Defining the following macro will cause the error metric function to weigh
// each color channel differently depending on how the human eye can perceive
// them. This can give a slight improvement in image quality at the cost of a
// performance hit.
// #define USE_PERCEIVED_ERROR_METRIC

#define ALIGNAS(X)	__attribute__((aligned(X)))

class TextureCompressor {
	cl_context        context;
	cl_program        program;
	cl_command_queue  command_queue;
	cl_kernel         kernel;
	cl_device_id      *device_ids;
	cl_platform_id    *platform_ids;
	
public:
	cl_device_id 	device;	// used to inspect device type
	TextureCompressor();
	~TextureCompressor();
	
	// Compress a texture using ETC1. Note that the |quality| parameter is
	// ignored. The current implementation does not support different quality
	// settings.
	unsigned long compress(const uint8_t* src,
						   uint8_t* dst,
						   int width,
						   int height);
	
protected:
	unsigned long compressBlock(uint8_t* dst,
								const Color* ver_src,
								const Color* hor_src,
								unsigned long threshold);
};

#endif  // CC_RASTER_TEXTURE_COMPRESSOR_H_
