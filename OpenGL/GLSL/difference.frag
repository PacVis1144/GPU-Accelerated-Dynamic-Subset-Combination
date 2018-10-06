#version 130

//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

in vec2 uv;
out float result[2];

uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;
uniform sampler2D tex4;

void main(void)
{
    result[ 0 ] = texture( tex1, uv.st ).r - texture( tex2, uv.st ).r;
    result[ 1 ] = texture( tex3, uv.st ).r - texture( tex4, uv.st ).r;
}
