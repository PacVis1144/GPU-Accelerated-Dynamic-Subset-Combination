#version 400

//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

#define PIo2 1.57079632679

layout(lines) in;
layout(line_strip, max_vertices=2) out;

in vData
{
    float weight;
    flat int select;
    flat int isReal;
} vertices[];


out G2F{
    float value;
    flat int select;
} g2f;

void main( void )
{
    int s = vertices[ 0 ].select * vertices[ 1 ].select;
    int r = vertices[ 0 ].isReal * vertices[ 1 ].isReal;

    if( bool( r ) )
    {
        g2f.value  = vertices[ 0 ].weight;
        g2f.select = s;

        gl_Position = gl_in[0].gl_Position;
        EmitVertex();

        g2f.value  = vertices[ 1 ].weight;
        g2f.select = s;

        gl_Position = gl_in[1].gl_Position;
        EmitVertex();

        EndPrimitive();
    }
}
