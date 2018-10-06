//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

#ifndef TN_COMPUTE_STATS_HPP
#define TN_COMPUTE_STATS_HPP

#include <vector>
#include <string>

#include <GL/glew.h>
#include <nvrtc.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <cudaGL.h>

class FCCompute
{
    bool m_useGLInterop;

    CUdeviceptr m_timeAveragedHistograms;
    CUdeviceptr m_currentHistograms;
    CUdeviceptr m_sums;
    CUdeviceptr m_counts;

    CUdeviceptr m_buffer;
    CUdeviceptr m_ranges;
    CUdeviceptr m_bitmasks;

    CUdevice   m_cuDevice;
    CUcontext  m_context;
    CUmodule   m_module;
    CUmodule   m_module2;


    CUfunction m_timeAverageHistogramKernel;
    CUfunction m_currentHistogramKernel;
    CUfunction m_sumAndCountKernel;
    CUfunction m_absoluteSumAndCountKernel;
    CUfunction m_radialSelectionKernel;
    CUfunction m_combinationKernel; 
    CUfunction m_divideKernel;


    std::string template_kernel;
    std::string template_kernel2;

    CUgraphicsResource * m_glResources;
    int m_nResources;

    void clean();

public :

    FCCompute();

    void cleanMappedBuffers();

    void mapBuffer(
        GLuint buffer,
        const size_t N_STEPS,
        const size_t N_PARTICLES,
        const size_t N_VARS );

    void allocateRanges(
        const size_t N_VARS );

    void bufferRanges(
        float * ranges,
        const size_t N_VARS );

    void bufferData(
        float * data,
        const size_t N );

    void bufferBitmasks(
        int * bitmasks,
        const size_t N_PARTICLES,
        const size_t N_STEPS );

    void allocateResultArray(
        const size_t N_STEPS,
        const size_t N_VARS,
        const size_t N_BINS,
        const size_t N_SETS );

    void cleanResultArray();
    void cleanBuffers();

    void computeCombination(
        int N_STEPS,
        int N_PARTICLES,
        std::vector< int > & result );

    void computeSampleMean( 
        unsigned long N_PARTICLES,
        unsigned long N_STEPS,
        unsigned  int N_SETS,
        unsigned int V_IDX,
        bool absolute,
        std::vector<float> & result );

    void computeSumsAndCounts(
        unsigned long N_PARTICLES,
        unsigned long N_STEPS,     
        unsigned int N_SETS,
        unsigned int V_IDX,
        bool absolute );

    void computeHistograms(
        unsigned long N_PARTICLES,
        unsigned long N_STEPS,
        unsigned long STEP,
        unsigned int N_VARS,
        unsigned int N_BINS,
        unsigned int N_SETS,
        std::vector< float > & result,
        bool probability = false );

    void resizeBuffer(
        const size_t NUM_PARTICLES,
        const size_t N_STEPS,
        const size_t N_VARS );

    void updateExpression( const std::string & expression );
    void initRuntime();

    ~FCCompute();
};

#endif
