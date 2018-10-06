#version 400

//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

#define PIo2 1.57079632679

layout(lines) in;
layout(line_strip, max_vertices=2) out;

uniform sampler1D angleToNormalization;
uniform float width;
uniform float height;

in vData
{
    float weight;
    flat int select;
    flat int isReal;
} vertices[];


out G2F{
    float valAll;
    float valComb;
    float countAll;
    float countComb;
} g2f;

void main( void )
{
    int s = vertices[ 0 ].select * vertices[ 1 ].select;
    int r = vertices[ 0 ].isReal * vertices[ 1 ].isReal;

    if( bool( r ) )
    {
        float distX = abs( gl_in[1].gl_Position.x - gl_in[0].gl_Position.x ) * width;
        float distY = abs( gl_in[1].gl_Position.y - gl_in[0].gl_Position.y ) * height;

        vec2 a = normalize( vec2( distX, distY ) );
        vec2 b = vec2( 1, 0 );
        float angle = acos( dot( a, b ) );

        float norm = texture( angleToNormalization, angle / PIo2 ).r;
        float dist = max( length( vec2( distX, distY ) ), 1 );

        norm = 1.0 / ( dist * norm );

        float nmW1 = vertices[ 0 ].weight * norm;

        g2f.valAll    =  nmW1  * r;
        g2f.valComb   =  nmW1  * s * r;
        g2f.countAll  =  norm  * r;
        g2f.countComb =  norm  * s * r;

        gl_Position = gl_in[0].gl_Position;
        EmitVertex();

        float nmW2 = vertices[ 1 ].weight * norm;

        g2f.valAll    =   nmW2 * r;
        g2f.valComb   =   nmW2 * s * r;
        g2f.countAll  =   norm * r;
        g2f.countComb =   norm * s * r;

        gl_Position = gl_in[1].gl_Position;
        EmitVertex();

//        EndPrimitive();
    }
}
