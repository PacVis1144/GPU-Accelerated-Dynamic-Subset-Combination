#version 400

//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

out float fragColor[ 2 ];

in G2F{
    float valAll;
    float valComb;
    float norm;
} g2f;

uniform sampler2D tex1;
uniform sampler2D tex2;

uniform float width;
uniform float height;

void main(void)
{
    vec2 uv = vec2( gl_FragCoord.x / width, gl_FragCoord.y / height );

    float term1 =  g2f.valAll  - texture( tex1, uv.st ).r;
    float term2 =  g2f.valComb - texture( tex2, uv.st ).r;

    fragColor[ 0 ] = term1 * term1;// * g2f.norm;
    fragColor[ 1 ] = term2 * term2;// * g2f.norm;
}
