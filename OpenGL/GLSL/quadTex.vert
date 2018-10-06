#version 130

//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

in vec2 ps;
in vec2 tc;

out vec2 uv;

void main(void)
{
    uv = tc;
    gl_Position = vec4( ps.x, ps.y, 0, 1 );
}

