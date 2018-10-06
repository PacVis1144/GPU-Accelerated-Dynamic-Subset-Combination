#version 400

//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

in float xAttr;
in float yAttr;

void main(void)
{
    gl_Position = vec4( xAttr, yAttr, 0, 1.0 );
}
