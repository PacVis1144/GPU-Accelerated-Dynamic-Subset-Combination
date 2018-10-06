//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

#define ALL_MASK 1
#define COM_MASK 2
#define ALL_SEL_MASK 4
#define COM_SEL_MASK 8

__device__ __constant__
int lookup[ 31 ] = {
    1,2,4,8,16,32,64,128,256,512,1024,2048,4096,
    8192,16384,32768,65536,131072,262144,524288,
    1048576,2097152,4194304,8388608,16777216,
    33554432,67108864,134217728,268435456,536870912,
    1073741824
};

__device__
bool testSet( int s, int b )
{
    return ( ( b & lookup[ s ] ) && ( b & 1 ) );
}

extern "C" __global__
void divide(
    float * A,
    float * B,
    int N )
{
    const int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if( idx >= N )
    {
        return;
    }

    A[ idx ] /= B[ idx ];
}

extern "C" __global__
void computeTimeSeriesSumsAndCounts(
    float * data,
    int   * bitmasks,
    float * sums,
    float * counts,
    const unsigned long NPT,
    const unsigned long N_STEPS,
    const unsigned int N_SETS,
    const unsigned int V_OFF )
{
    const long int GLOBAL_IDX = blockIdx.x * blockDim.x + threadIdx.x;

    if( GLOBAL_IDX >= NPT )
    {
        return;
    }

    const int pIDX = GLOBAL_IDX / N_STEPS;
    const int tIDX = GLOBAL_IDX % N_STEPS;

    int b = bitmasks[ GLOBAL_IDX ];

    for( int sIDX = 0; sIDX < N_SETS; ++sIDX )
    {
        if( ( ( b & lookup[ sIDX ] ) && ( b & 1 ) ) )
        {
            const float value = data[ V_OFF + pIDX * N_STEPS + tIDX ];
            atomicAdd( & sums[   sIDX * N_STEPS + tIDX ], value );
            atomicAdd( & counts[ sIDX * N_STEPS + tIDX ], 1.0 );
        }
    }
}

extern "C" __global__
void computeCurrentHistograms(
    float * data,
    int   * bitmasks,
    float * ranges,
    float * histograms,
    const unsigned long N_PARTICLES,
    const unsigned long N_STEPS,
    const unsigned long STEP,
    const unsigned int N_VARS,
    const unsigned int N_BINS,
    const unsigned int N_SETS )
{
    const int pIDX = blockIdx.x * blockDim.x + threadIdx.x;

    if( pIDX >= N_PARTICLES )
    {
        return;
    }

    int b = bitmasks[ N_STEPS * pIDX + STEP ];

    for( unsigned int sIDX = 0; sIDX < N_SETS; ++sIDX )
    {
        if( ! testSet( sIDX, b ) )
        {
            continue;
        }
        for( unsigned int vIDX = 0; vIDX < N_VARS; ++vIDX )
        {
            const float MN = ranges[ vIDX*2     ];
            const float MX = ranges[ vIDX*2 + 1 ];
            const float WDTH = MX - MN;

            const float value = data[ vIDX * N_STEPS * N_PARTICLES + pIDX * N_STEPS + STEP ];

            if( value < MN || value > MX )
            {
                continue;
            }

            int bIDX = umin( ( unsigned int ) ( ( value - MN ) / WDTH * N_BINS ), ( unsigned int )( N_BINS - 1 ) );

            atomicAdd( & histograms[ sIDX * N_VARS * N_BINS + vIDX * N_BINS + bIDX ], 1.0 );
        }
    }
}
