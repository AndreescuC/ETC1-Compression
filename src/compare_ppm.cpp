#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <stdint.h>
#include <math.h>

using namespace std;

int main(int argc, char*args[]) {

	int width, height, max;
	char buffer[256];
	FILE *f[argc - 1];
	
	for (int i = 0; i< argc - 1; i++) {
		char *file = args[1 + i]; 
		f[i] = fopen(file, "rb");
		fgets(buffer, sizeof(buffer), f[i]);
		long data_start = ftell(f[i]);

		while (fgets(buffer, sizeof(buffer), f[i])) {
			if (buffer[0] == '#') {
				data_start = ftell(f[i]);
			}
			else {
				fseek(f[i], data_start, SEEK_SET);
				break;
			}
		}
		fscanf(f[i], "%d %d %d\n", &width, &height, &max);
	}

	uint8_t *src = (uint8_t*)calloc(width * height * 4, 1);
	uint8_t *src1 = (uint8_t*)calloc(width * height * 4, 1);
	unsigned long *error = (unsigned long*)calloc(argc - 2, sizeof(unsigned long));

	int stride = width * 4;
	int red, green, blue, alpha = 3;

	red = 2;
	green = 1;
	blue = 0;
	
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < stride; j += 4) {
			fread(&src[stride * i + j + red], 1, 1, f[0]);
			fread(&src[stride * i + j + green], 1, 1, f[0]);
			fread(&src[stride * i + j + blue], 1, 1, f[0]);
			src[stride * i + j + alpha] = 0;
		}
	}

	for (int k = 1; k < argc - 1; k++) {
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < stride; j += 4) {
				fread(&src1[stride * i + j + red], 1, 1, f[k]);
				fread(&src1[stride * i + j + green], 1, 1, f[k]);
				fread(&src1[stride * i + j + blue], 1, 1, f[k]);
				src1[stride * i + j + alpha] = 0;
				for (int l = 0; l < 4; l++) {
					unsigned long temp = (src1[stride * i + j + l] - src[stride * i + j + l]);
					error[k - 1] += (unsigned long)(temp * temp);
				}
			}
		}
	}

	for (int i = 0; i < argc - 2; i++) {
		double PSNR = 10 * log10(3 * 255.0 * 255.0 / (error[i] / (double)(width * height)));
		double RMSE = sqrt(error[i] / (double)(width * height));
		printf("FILE = %s, \tPSNR = %.6lf, \tRMSE = %.6lf\n", args[i + 2], PSNR, RMSE);
	}
}
