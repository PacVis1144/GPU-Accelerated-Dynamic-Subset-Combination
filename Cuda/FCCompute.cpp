//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

#include "Repo/Cuda/FCCompute.hpp"
#include "Expressions/GenerateBooleanExpressionProgram.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <streambuf>
#include <chrono>
#include <ctime>
#include "Algorithms/Standard/MyAlgorithms.hpp"

#include <nvrtc.h>
#include <cuda.h>
#include <cuda_runtime.h>

#define KERNEL_PATH  "./FCCompute.cu"

#define NVRTC_SAFE_CALL(x)                                        \
  do {                                                            \
    nvrtcResult nvrt_result = x;                                  \
    if (nvrt_result != NVRTC_SUCCESS) {                           \
      std::cerr << "\nerror: " #x " failed with error "           \
                << nvrtcGetErrorString( nvrt_result ) << '\n';    \
      exit(1);                                                    \
    }                                                             \
  } while(0)
#define CUDA_SAFE_CALL(x)                                         \
  do {                                                            \
    CUresult cu_result = x;                                       \
    if (cu_result != CUDA_SUCCESS) {                              \
      const char *msg;                                            \
      cuGetErrorName(cu_result, &msg);                            \
      std::cerr << "\nerror: " #x " failed with error "           \
                << msg << '\n';                                   \
      exit(1);                                                    \
    }                                                             \
  } while(0)

void FCCompute::initRuntime()
{
    CUDA_SAFE_CALL( cuInit( 0 ) );
    CUDA_SAFE_CALL( cuDeviceGet( & m_cuDevice, 0 ) );
    CUDA_SAFE_CALL( cuCtxCreate( & m_context, 0, m_cuDevice ) );

    char name[ 500 ];
    cuDeviceGetName ( name, 500, m_cuDevice );

    nvrtcProgram program;

    const char * options[] =
    {
        "--gpu-architecture=compute_52"
    };

    std::ifstream inFile( KERNEL_PATH );
    std::string source = std::string( std::istreambuf_iterator<char>( inFile ), std::istreambuf_iterator<char>() );
    inFile.close();
    
    NVRTC_SAFE_CALL( nvrtcCreateProgram(
        & program,
        source.c_str(),
        "computeStatistics.cu",
        0,
        NULL,
        NULL ) );

    NVRTC_SAFE_CALL( nvrtcCompileProgram( program,  1, options ) );
    size_t ptxSize;
    NVRTC_SAFE_CALL( nvrtcGetPTXSize( program, & ptxSize ) );
    char * ptx = new char[ ptxSize ];
    NVRTC_SAFE_CALL( nvrtcGetPTX( program, ptx ) );
    NVRTC_SAFE_CALL( nvrtcDestroyProgram( & program ) );
    CUDA_SAFE_CALL( cuModuleLoadDataEx(  & m_module, ptx, 0, 0, 0 ) );
    CUDA_SAFE_CALL( cuModuleGetFunction( & m_currentHistogramKernel,     m_module, "computeCurrentHistograms"               ) );
    CUDA_SAFE_CALL( cuModuleGetFunction( & m_sumAndCountKernel,          m_module, "computeTimeSeriesSumsAndCounts"         ) );
    CUDA_SAFE_CALL( cuModuleGetFunction( & m_divideKernel,               m_module, "divide"                                 ) );
}

FCCompute::FCCompute()
{
    m_useGLInterop = true;

    m_buffer = 0;
    m_ranges = 0;
    m_bitmasks = 0;
    m_sums = 0;
    m_counts = 0;

    m_glResources = 0;
    m_nResources = 0;
}

FCCompute::~FCCompute()
{
    CUDA_SAFE_CALL( cuModuleUnload(m_module ) );
    CUDA_SAFE_CALL( cuCtxDestroy( m_context ) );

    cleanBuffers();
    cleanMappedBuffers();
    cleanResultArray();
}

void FCCompute::cleanBuffers()
{
    if( ! m_useGLInterop )
    {
        CUDA_SAFE_CALL( cuMemFree( m_buffer                 ) );
        CUDA_SAFE_CALL( cuMemFree( m_bitmasks               ) );

        m_buffer = 0;
        m_bitmasks = 0;
    }

    CUDA_SAFE_CALL( cuMemFree( m_ranges                 ) );
    m_ranges = 0;
}

void FCCompute::cleanResultArray()
{
    CUDA_SAFE_CALL( cuMemFree( m_currentHistograms      ) );
    CUDA_SAFE_CALL( cuMemFree( m_timeAveragedHistograms ) );
    CUDA_SAFE_CALL( cuMemFree( m_sums                   ) );
    CUDA_SAFE_CALL( cuMemFree( m_counts                 ) );

    m_currentHistograms = 0;
    m_timeAveragedHistograms = 0;
    m_sums = 0;
    m_counts = 0;
}

void FCCompute::allocateResultArray(
    const size_t N_STEPS,
    const size_t N_VARS,
    const size_t N_BINS,
    const size_t N_SETS )
{
    cleanResultArray();
    CUDA_SAFE_CALL( cuMemAlloc( & m_currentHistograms, N_VARS * N_BINS * N_SETS * sizeof ( float ) ) );
    CUDA_SAFE_CALL( cuMemAlloc( & m_timeAveragedHistograms, N_VARS * N_BINS * N_SETS * sizeof ( float ) ) );
    CUDA_SAFE_CALL( cuMemAlloc( & m_sums,   N_STEPS * N_SETS * sizeof ( float ) ) );
    CUDA_SAFE_CALL( cuMemAlloc( & m_counts, N_STEPS * N_SETS * sizeof ( float ) ) );
}

void FCCompute::resizeBuffer(
    const size_t N_PARTICLES,
    const size_t N_STEPS,
    const size_t N_VARS  )
{
    cleanBuffers();
    CUDA_SAFE_CALL( cuMemAlloc( & m_buffer, N_STEPS * N_PARTICLES * N_VARS * sizeof ( float ) ) );
    CUDA_SAFE_CALL( cuMemAlloc( & m_ranges, N_VARS * 2 * sizeof ( float ) ) );
    CUDA_SAFE_CALL( cuMemAlloc( & m_bitmasks, N_STEPS * N_PARTICLES * sizeof ( int ) ) );
}

void FCCompute::cleanMappedBuffers()
{
    if( m_nResources <= 0 )
    {
        return;
    }

    CUDA_SAFE_CALL( cuGraphicsUnmapResources( m_nResources, m_glResources, 0 ) );

    for( int i = 0; i < m_nResources; ++i )
    {
         CUDA_SAFE_CALL( cuGraphicsUnregisterResource( m_glResources[ i ] ) );
    }
    delete [ ] m_glResources;
}

void FCCompute::mapBuffer(
    GLuint buffer,
    const size_t N_STEPS,
    const size_t N_PARTICLES,
    const size_t N_VARS )
{
    cleanMappedBuffers();

    m_nResources = 1;
    m_glResources = new CUgraphicsResource[ 1 ];

    CUDA_SAFE_CALL( cuGraphicsGLRegisterBuffer( & m_glResources[ 0 ], buffer, 0 ) );
    CUDA_SAFE_CALL( cuGraphicsMapResources( m_nResources, m_glResources, 0 ) );

    size_t sz;
    CUDA_SAFE_CALL( cuGraphicsResourceGetMappedPointer( & m_buffer, & sz, m_glResources[ 0 ] ) );

    m_bitmasks = ( CUdeviceptr )( ( ( int * ) m_buffer ) + ( N_STEPS * N_PARTICLES * N_VARS ) );
}

void FCCompute::allocateRanges(
   const size_t N_VARS )
{
    CUDA_SAFE_CALL( cuMemFree( m_ranges ) );
    CUDA_SAFE_CALL( cuMemAlloc( & m_ranges, N_VARS * 2 * sizeof ( float ) ) );
}

void FCCompute::bufferRanges(
   float * ranges,
   const size_t N_VARS )
{
   CUDA_SAFE_CALL( cuMemcpyHtoD( m_ranges, ranges, N_VARS * 2 * sizeof( float ) ) );
}

void FCCompute::bufferData(
   float * data,
   const size_t N )
{
    CUDA_SAFE_CALL( cuMemcpyHtoD( m_buffer, data, N * sizeof( float ) ) );
}

void FCCompute::bufferBitmasks(
   int * bitmasks,
   const size_t N_PARTICLES,
   const size_t N_STEPS )
{
    CUDA_SAFE_CALL(  cuMemcpyHtoD( m_bitmasks, bitmasks, N_PARTICLES * N_STEPS * sizeof( int ) ) );
}

void FCCompute::updateExpression( const std::string &expression )
{
    nvrtcProgram program;
    std::string source = TN::generateCombinationProgram( expression, TN::CUDA_COMBINATION_KERNEL_TEMPLATE );

    NVRTC_SAFE_CALL( nvrtcCreateProgram(
        & program,
        source.c_str(),
        "Combination.cu",
        0,
        NULL,
        NULL ) );

    const char * options[] =
    {
        "--gpu-architecture=compute_52"
    };

    NVRTC_SAFE_CALL( nvrtcCompileProgram( program, 1, options ) );

    size_t ptxSize;
    NVRTC_SAFE_CALL( nvrtcGetPTXSize( program, & ptxSize ) );
    char * ptx = new char[ ptxSize ];
    NVRTC_SAFE_CALL( nvrtcGetPTX( program, ptx ) );

    NVRTC_SAFE_CALL( nvrtcDestroyProgram( & program ) );
    CUDA_SAFE_CALL( cuModuleLoadDataEx(  & m_module, ptx, 0, 0, 0 ) );
    CUDA_SAFE_CALL( cuModuleGetFunction( & m_combinationKernel, m_module, "setCombination" ) );
}

void FCCompute::computeCombination(
    int N_STEPS,
    int N_PARTICLES,
    std::vector< int > & result )
{
    size_t N = N_STEPS * N_PARTICLES;

    void *args[] = {
        & m_bitmasks,
        & N
    };

    CUDA_SAFE_CALL(  cuLaunchKernel(
        m_combinationKernel,
        512 * std::ceil( ( float ) N / float( 512 ) ), 1, 1,
        512, 1, 1,
        0, NULL,
        args, 0 ) );

    CUDA_SAFE_CALL(  cuCtxSynchronize() );
    CUDA_SAFE_CALL( cuMemcpyDtoH( result.data(), m_bitmasks, N * sizeof( int ) ) );
}

void FCCompute::computeSumsAndCounts(
    unsigned long N_PARTICLES,
    unsigned long N_STEPS,
    unsigned  int N_SETS,
    unsigned int V_IDX,
    bool absolute )
{
    unsigned long N = N_STEPS * N_PARTICLES;
    unsigned long V_OFF = V_IDX * N_STEPS * N_PARTICLES;

    CUDA_SAFE_CALL( cuMemsetD32(   m_sums, 0, N_STEPS * N_SETS ) );
    CUDA_SAFE_CALL( cuMemsetD32( m_counts, 0, N_STEPS * N_SETS ) );

    void *args[] = {
        & m_buffer,
        & m_bitmasks,
        & m_sums,
        & m_counts,
        & N,
        & N_STEPS,
        & N_SETS,
        & V_OFF
    };

    CUDA_SAFE_CALL( cuLaunchKernel(
        m_sumAndCountKernel,
        512 * std::ceil( ( float ) N / float( 512 ) ), 1, 1,
        512, 1, 1,
        0, NULL,
        args, 0 ) );

    CUDA_SAFE_CALL( cuCtxSynchronize() );
}

void FCCompute::computeSampleMean(
    unsigned long N_PARTICLES,
    unsigned long N_STEPS,
    unsigned  int N_SETS,
    unsigned int V_IDX,
    bool absolute,
    std::vector<float> & result )
{
    computeSumsAndCounts(
        N_PARTICLES,
        N_STEPS,
        N_SETS,
        V_IDX,
       absolute );

    long int N = N_STEPS * N_SETS;

    void *args[] = {
        & m_sums,
        & m_counts,
        & N
    };

    CUDA_SAFE_CALL(  cuLaunchKernel(
        m_divideKernel,
        512 * std::ceil( ( float ) N / float( 512 ) ), 1, 1,
        512, 1, 1,
        0, NULL,
        args, 0 ) );

    CUDA_SAFE_CALL(  cuCtxSynchronize() );
    CUDA_SAFE_CALL( cuMemcpyDtoH( result.data(), m_sums, N * sizeof( float ) ) );
}

void FCCompute::computeHistograms(
    unsigned long N_PARTICLES,
    unsigned long N_STEPS,
    unsigned long STEP,
    unsigned int N_VARS,
    unsigned int N_BINS,
    unsigned int N_SETS,
    std::vector< float > & result,
    bool probability )
{
    const size_t RESULT_SIZE = N_VARS * N_BINS * N_SETS;

    CUDA_SAFE_CALL( cuMemsetD32( m_currentHistograms, 0, RESULT_SIZE ) );

    void *args[] = {
        & m_buffer,
        & m_bitmasks,
        & m_ranges,
        & m_currentHistograms,
        & N_PARTICLES,
        & N_STEPS,
        & STEP,
        & N_VARS,
        & N_BINS,
        & N_SETS
    };

    CUDA_SAFE_CALL( cuLaunchKernel(
        m_currentHistogramKernel,
        512 * std::ceil( ( float ) N_PARTICLES / float( 512 ) ), 1, 1,
        512, 1, 1,
        0, NULL,
        args, 0 ) );

    CUDA_SAFE_CALL( cuCtxSynchronize() );
    CUDA_SAFE_CALL( cuMemcpyDtoH( result.data(), m_currentHistograms, RESULT_SIZE * sizeof( float ) ) );

    if( probability )
    {
        for( int s = 0; s < N_SETS; ++s )
        {
            for( int v = 0; v < N_VARS; ++v )
            {
                float sum = 0;
                for( int b = 0; b < N_BINS; ++b )
                {
                    sum += result[ s * N_VARS * N_BINS + v * N_BINS + b ];
                }
                if( sum > 0 )
                {
                    for( int b = 0; b < N_BINS; ++b )
                    {
                        result[ s * N_VARS * N_BINS + v * N_BINS + b  ] /= sum;
                    }
                }
            }
        }
    }
}
