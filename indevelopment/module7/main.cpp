#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <omp.h>
#include <cuda_profiler_api.h>

#include "imageWrapper.h"

extern "C" {
  void blur5(unsigned char*, unsigned char*, long, long, long);
  void blur5_serial(unsigned char*, unsigned char*, long, long, long);
}

int main(int argc, char** argv)
{
  // Excludes warm-up from profile
  cudaProfilerStop();
  if (argc < 3) {
    fprintf(stderr,"Usage: %s inFilename outFilename\n",argv[0]);
    return -1;
  }

  long w, h, ch;
  unsigned char* data = readImage(argv[1], w, h, ch);

  unsigned char* output1 = new unsigned char[w*h*ch];
  unsigned char* output2 = new unsigned char[w*h*ch];

  // Warm-up
  blur5(data, output1, w, h, ch);

  cudaProfilerStart();
  double st = omp_get_wtime();
  blur5(data, output1, w, h, ch);
  printf("Time taken for parallel blur5: %.4f seconds\n", omp_get_wtime()-st);
  printf("Running serial for correctness comparison...\n");
  st = omp_get_wtime();
  blur5_serial(data, output2, w, h, ch);
  printf("Time taken for serial blur5: %.4f seconds\n", omp_get_wtime()-st);

  bool success = true;
  for(int i = 0; i < w*h*ch; i++) {
    if(output1[i] != output2[i]) {
      success = false;
      break;
    }
  }

  if(success) {
    printf("Results are equal.\n");
  } else {
    printf("Results are not equal.\n");
  }

  memcpy(data, output1, w*h*ch);
  writeImage(argv[2]);

  delete[] output1;
  delete[] output2;

  return 0;
}
