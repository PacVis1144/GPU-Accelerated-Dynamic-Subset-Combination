#version 400

//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

out float fragColor[2];

in G2F{
    float valAll;
    float valComb;
}g2f;

void main(void)
{
    fragColor[ 0 ] = g2f.valAll;
    fragColor[ 1 ] = g2f.valComb;
}
