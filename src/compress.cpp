#include "compress.hpp"

#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <chrono>

using namespace std;

#define BGRA

int readCompressWrite(const char *input, 
	const char *output,
	TextureCompressor *compressor)
{
	char line[256];
	int red, green, blue;
	int max;
	int width, height;
	long data_start;
	int stride;
	int compressed_size;
	uint8_t *src, *dst;
	auto begin_timer = std::chrono::high_resolution_clock::now();

	FILE *f = fopen(input, "r");
	if (!f) {
		cout << "File not found: " << input << endl;
		return EXIT_FAILURE;
	}

#if defined(BGRA)
	red = 2;
	green = 1;
	blue = 0;
	#else
	red = 0;
	green = 1;
	blue = 2;
#endif

	fgets(line, sizeof(line), f);
	data_start = ftell(f);

	while (fgets(line, sizeof(line), f)) {
		if (line[0] == '#') {
			data_start = ftell(f);
		}
		else {
			fseek(f, data_start, SEEK_SET);
			break;
		}
	}

	fscanf(f, "%d %d %d\n", &width, &height, &max);
	src = (uint8_t*)malloc( width * height * 4);

	if (!src) {
		fclose(f);
		return EXIT_FAILURE;
	}

	stride = width * 4;

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < stride; j += 4) {
			fread(&src[stride * i + j + red], 1, 1, f);
			fread(&src[stride * i + j + green], 1, 1, f);
			fread(&src[stride * i + j + blue], 1, 1, f);
			src[stride * i + j + 3] = 0;
		}
	}

	fclose(f);
	
	compressed_size = width * height * 4 / 8;
	dst = (uint8_t*)malloc( compressed_size);

	if (!dst) {
		free(src);
		return EXIT_FAILURE;
	}

	memset(dst, 0, compressed_size);
	
	// actual compression
	begin_timer = std::chrono::high_resolution_clock::now();
	compressor->compress(src, dst, width, height);

	// end timer compressor
	cout << "FILE = " << input << ", \tTIME.ms = " <<
	std::chrono::duration_cast<std::chrono::milliseconds>
	(std::chrono::high_resolution_clock::now() - begin_timer).count()
	<< endl;

	// write file	
	f = fopen(output, "wb");
	if (f) {
		uint8_t header[16];
		memcpy(header, "PKM ", 4);
		memcpy(&header[4], "20", 2);
		header[6] = 0;
		header[7] = 1;
		header[8] = header[12] = (width >> 8) & 0xFF;
		header[9] = header[13] = width & 0xFF;
		header[10] = header[14] = (height >> 8) & 0xFF;
		header[11] = header[15] = height & 0xFF;

		fwrite(header, 16, 1, f);
		fwrite(dst, compressed_size, 1, f);
		fclose(f);
	}

	free(dst);
	free(src);
	return EXIT_SUCCESS;
}

struct CompressedFile
{
	string in;
	string out;
};

int main(int argc, const char **argv)
{
	int ret = 0;
	vector<CompressedFile> todoFiles;

	auto begin_timer = std::chrono::high_resolution_clock::now();
	
	// create compressor
	TextureCompressor *compressor = new TextureCompressor();
	if (!compressor) {
		cout << "ERR allocate compressor" << endl;
		return EXIT_FAILURE;
	}
	
	// end timer contructor
	cout << "INIT TIME.ms = " <<
	chrono::duration_cast<chrono::milliseconds>
	(chrono::high_resolution_clock::now() - begin_timer).count()
	<< endl;
	
	if(compressor->device == 0) {
		cout << "DEVICE INVALID. Is compressor.device set ?" << endl;
	}
	else {
		char devName[512];
		clGetDeviceInfo(compressor->device, CL_DEVICE_NAME,
				512, devName, NULL);
		cout << "DEVICE = " << devName << endl;
		cl_device_type devType;
		clGetDeviceInfo(compressor->device, CL_DEVICE_TYPE,
            sizeof(cl_device_type), &devType, NULL);
		if(devType == CL_DEVICE_TYPE_GPU) {
			cout << "GPU = YES" << endl;
		} else {
			cout << "GPU = NO!!" << endl;	
		}
	}

	// cycle through list of files
	for(int i = 1; i < argc; i++) {
		CompressedFile cFile;
		cFile.in = argv[i];
		cFile.out = cFile.in + ".pkm";
		todoFiles.push_back(cFile);
	}
	for(auto cFile : todoFiles) {
		ret |= readCompressWrite(cFile.in.c_str(), 
			cFile.out.c_str(), compressor);
	}
	
	delete compressor;
	return ret;
}
