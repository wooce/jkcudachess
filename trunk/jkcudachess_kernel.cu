#ifndef _jkcudachess_KERNEL_H_
#define _jkcudachess_KERNEL_H_

#include <stdio.h>

#define SDATA( index)      CUT_BANK_CHECKER(sdata, index)



__global__ void testKernel( float* g_idata, float* g_odata) 
{
  // shared memory
  // the size is determined by the host application
  extern  __shared__  float sdata[];

  // access thread id
  const unsigned int tid = threadIdx.x;
  // access number of threads in this block
  const unsigned int num_threads = blockDim.x;

  // read in input data from global memory
  // use the bank checker macro to check for bank conflicts during host
  // emulation
  SDATA(tid) = g_idata[tid];
  __syncthreads();

  // perform some computations
  SDATA(tid) = (float) num_threads * SDATA( tid);
  __syncthreads();

  // write data to global memory
  g_odata[tid] = SDATA(tid);
}

__global__ void vecAdd(float* A,float* B,float* C)
{
	int i =blockIdx.x * blockDim.x + threadIdx.x;
	C[i] = A[i] + B [i];
}
//測試把2D記憶體copy過去+1
__global__ void test2Darray(unsigned char KingMoves[256][8])
{
    KingMoves[blockIdx.x][threadIdx.x]+=1;
}
//測試把3D記憶體copy過去+1
__global__ void test3Darray(unsigned char xRookMoves[12][512][12])
{
    for(int i=0;i<12;i++)
    {
        xRookMoves[i][blockIdx.x][threadIdx.x]+=1;
    }
}
#endif // #ifndef _jkcudachess_KERNEL_H_
