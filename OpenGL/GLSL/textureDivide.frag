#version 130

//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

in vec2 uv;
out float result[2];

uniform sampler2D sumAll;
uniform sampler2D countAll;
uniform sampler2D sumComb;
uniform sampler2D countComb;

void main(void)
{
    float vAll = texture(   sumAll, uv.st ).r;
    float cAll = texture( countAll, uv.st ).r;

    if( vAll == 0 || cAll == 0 )
    {
        result[ 0 ] = 0;
    }
    else
    {
        result[ 0 ] = vAll / cAll;
    }

    float vComb = texture(   sumComb, uv.st ).r;
    float cComb = texture( countComb, uv.st ).r;

    if( vComb == 0 || cComb == 0 )
    {
        result[ 1 ] = 0;
    }
    else
    {
        result[ 1 ] = vComb / cComb;
    }
}
