#version 400

//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

layout(location = 0) in float x1;
layout(location = 1) in float x2;
layout(location = 2) in int   c;
out vData
{
    flat int select;
    flat int isReal;
} vertex;
uniform mat4 M;
void main(void)
{
    vertex.select = int( ( c & 2 ) != 0 );
    vertex.isReal = int( ( c & 1 ) != 0 );
    gl_Position = M * vec4( x1, x2, 0.0, 1.0 );
};
