// MP 1
#include <wb.h>

__global__ void vecAdd(float *in1, float *in2, float *out, int len) {
  //@@ Insert code to implement vector addition here

  int i = blockIdx.x * blockDim.x + threadIdx.x;

  if ( i < len ) out[i] = in1[i] + in2[i];
  //printf("blockIdx is: %d\n blockDim is: %d\n threadIdx is: %d\n idx is: %d\n out %f\n in1 %f\n in2 %f\n -------\n", blockIdx.x, blockDim.x, threadIdx.x, i,out[i],in1[i],in2[i]);
}

int main(int argc, char **argv) {
  wbArg_t args;
  int inputLength;
  float *hostInput1;
  float *hostInput2;
  float *hostOutput;
  float *deviceInput1;
  float *deviceInput2;
  float *deviceOutput;

  args = wbArg_read(argc, argv);

  wbTime_start(Generic, "Importing data and creating memory on host");
  hostInput1 = ( float * )wbImport(wbArg_getInputFile(args, 0), &inputLength);
  hostInput2 = ( float * )wbImport(wbArg_getInputFile(args, 1), &inputLength);
  hostOutput = ( float * )malloc(inputLength * sizeof(float));
  wbTime_stop(Generic, "Importing data and creating memory on host");

  wbLog(TRACE, "The input length is ", inputLength);

  wbTime_start(GPU, "Allocating GPU memory.");
  //@@ Allocate GPU memory here
  int size = inputLength * sizeof(float);
  cudaError_t err;
  err = cudaMalloc((void **) &deviceInput1, size);
  if (err!=cudaSuccess){
    printf("%s in %s at line %d\n",
        cudaGetErrorString(err),__FILE__,__LINE__);
    exit(EXIT_FAILURE);
  }
  err = cudaMalloc((void **) &deviceInput2, size);
  if (err!=cudaSuccess){
    printf("%s in %s at line %d\n",
        cudaGetErrorString(err),__FILE__,__LINE__);
    exit(EXIT_FAILURE);
  }
  err = cudaMalloc((void **) &deviceOutput, size);
  if (err!=cudaSuccess){
    printf("%s in %s at line %d\n",
        cudaGetErrorString(err),__FILE__,__LINE__);
    exit(EXIT_FAILURE);
  }

  wbTime_stop(GPU, "Allocating GPU memory.");

  wbTime_start(GPU, "Copying input memory to the GPU.");
  //@@ Copy memory to the GPU here
  err = cudaMemcpy(deviceInput1,hostInput1,size, cudaMemcpyHostToDevice);
  if (err!=cudaSuccess){
    printf("%s in %s at line %d\n",
        cudaGetErrorString(err),__FILE__,__LINE__);
    exit(EXIT_FAILURE);
  }
  err = cudaMemcpy(deviceInput2,hostInput2, size, cudaMemcpyHostToDevice);
  if (err!=cudaSuccess){
    printf("%s in %s at line %d\n",
        cudaGetErrorString(err),__FILE__,__LINE__);
    exit(EXIT_FAILURE);
  }

  wbTime_stop(GPU, "Copying input memory to the GPU.");

  //@@ Initialize the grid and block dimensions here
  dim3 DimGrid((inputLength-1)/32 + 1, 1, 1);
  dim3 DimBlock(32,1,1);

  wbTime_start(Compute, "Performing CUDA computation");
  //@@ Launch the GPU Kernel here
  vecAdd<<<DimGrid,DimBlock>>>(deviceInput1, deviceInput2, deviceOutput, inputLength);

  cudaDeviceSynchronize();
  wbTime_stop(Compute, "Performing CUDA computation");

  wbTime_start(Copy, "Copying output memory to the CPU");


  //@@ Copy the GPU memory back to the CPU here
  err = cudaMemcpy(hostOutput,deviceOutput,size,cudaMemcpyDeviceToHost);
  if (err!=cudaSuccess){
    printf("%s in %s at line %d\n",
        cudaGetErrorString(err),__FILE__,__LINE__);
    exit(EXIT_FAILURE);
  }
  for (int i = 0; i < 64; i++){
    wbLog(TRACE, "i: ", i);
    wbLog(TRACE, "host1 input ", hostInput1[i]);
    wbLog(TRACE, "host2 input ", hostInput2[i]);
  }

  wbTime_stop(Copy, "Copying output memory to the CPU");

  wbTime_start(GPU, "Freeing GPU Memory");
  //@@ Free the GPU memory here
  cudaFree(deviceInput1);
  cudaFree(deviceInput2);
  cudaFree(deviceOutput);

  wbTime_stop(GPU, "Freeing GPU Memory");

  wbSolution(args, hostOutput, inputLength);

  free(hostInput1);
  free(hostInput2);
  free(hostOutput);

  return 0;
}

