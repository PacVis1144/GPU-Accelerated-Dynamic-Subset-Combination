#version 400

//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

#define PIo2 1.57079632679
#define PI2 6.28318530718

layout(lines) in;
layout(line_strip, max_vertices=2) out;

uniform float width;
uniform float height;

in vData
{
    flat int select;
    flat int isReal;
} vertices[];


out G2F{
    float nm;
    float value;
    flat int select;
    flat int isReal;
} g2f;

void main( void )
{
    vec4 pos1 = gl_in[0].gl_Position;
    vec4 pos2 = gl_in[1].gl_Position;

    float distX = abs( pos2.x - pos1.x ) * width;
    float distY = abs( pos2.y - pos1.y ) * height;

    vec2 a = normalize( vec2( distX, distY ) );
    vec2 b = vec2( 1, 0 );

    float value = acos( dot( a, b ) );

    g2f.select = vertices[ 0 ].select * vertices[ 1 ].select;
    g2f.isReal = vertices[ 0 ].isReal * vertices[ 1 ].isReal;
    g2f.value = value;

    gl_Position = pos1;
    EmitVertex();

    g2f.select = vertices[ 0 ].select * vertices[ 1 ].select;
    g2f.isReal = vertices[ 0 ].isReal * vertices[ 1 ].isReal;
    g2f.value = value;

    gl_Position = pos2;
    EmitVertex();

    EndPrimitive();
}
