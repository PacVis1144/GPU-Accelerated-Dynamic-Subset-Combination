#version 400

//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

layout(location = 0) out float freq;

const float unit = 1.0 / 50.0;

void main(void)
{
    freq = unit;
}
