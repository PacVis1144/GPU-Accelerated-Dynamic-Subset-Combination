#version 400

//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

#define PIo2 1.57079632679

layout(lines) in;
layout(line_strip, max_vertices=2) out;

uniform sampler1D angleToNormalization;
uniform bool distanceWeighted;
uniform float width;
uniform float height;

in vData
{
    flat int select;
    flat int isReal;
} vertices[];

out G2F{
    float valAll;
    float valComb;
} g2f;

void main( void )
{
    int s = vertices[ 0 ].select * vertices[ 1 ].select;
    int r = vertices[ 0 ].isReal * vertices[ 1 ].isReal;

    if( bool( r ) )
    {

        vec4 pos1 = gl_in[0].gl_Position;
        vec4 pos2 = gl_in[1].gl_Position;

        float distX = abs( pos2.x - pos1.x ) * width;
        float distY = abs( pos2.y - pos1.y ) * height;

        vec2 a = normalize( vec2( distX, distY ) );
        vec2 b = vec2( 1, 0 );
        float angle = acos( dot( a, b ) );

        float norm = texture( angleToNormalization, angle / PIo2 ).r;

        if( distanceWeighted )
        {
            float dist = max( length( vec2( distX, distY ) ), 1 );
            norm *= dist;
        }

        norm = 1.0 / norm;

        g2f.valAll  = norm * r;
        g2f.valComb = norm * r * s;

        gl_Position = pos1;
        EmitVertex();

        g2f.valAll  = norm * r;
        g2f.valComb = norm * r * s;

        gl_Position = pos2;
        EmitVertex();

        //EndPrimitive();
    }
}
