#version 400

//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

#define PIo2 1.57079632679

layout(lines) in;
layout(line_strip, max_vertices=8) out;

uniform sampler1D angleToNormalization;
uniform bool distanceWeighted;
uniform float width;
uniform float height;

uniform bool YWRAPS;
uniform bool XWRAPS;

in vData
{
    flat int select;
    flat int isReal;
} vertices[];

out G2F{
    float valAll;
    float valComb;
} g2f;

const float W1 = -1;
const float W2 =  1;

void outputSubLine( vec4 p1, vec4 p2, float portion, float totalDist, int s )
{
    float distX = abs( p2.x - p1.x ) * width;
    float distY = abs( p2.y - p1.y ) * height;
    vec2 a = normalize( vec2( distX, distY ) );
    vec2 b = vec2( 1, 0 );
    float angle = acos( dot( a, b ) );
    float norm = texture( angleToNormalization, angle / PIo2 ).r;

    if( distanceWeighted )
    {
        norm *= totalDist;
    }

    norm = portion / norm;

    gl_Position = p1;
    g2f.valAll    =  norm;
    g2f.valComb   =  norm * s;
    EmitVertex();

    gl_Position = p2;
    g2f.valAll    =  norm;
    g2f.valComb   =  norm * s;
    EmitVertex();
}

void outputSplit( vec4 p1, vec4 p2, vec4 p3, vec4 p4, int s )
{
    float dist1X = abs( p1.x - p2.x ) * width;
    float dist1Y = abs( p1.y - p2.y ) * height;

    float dist2X = abs( p3.x - p4.x ) * width;
    float dist2Y = abs( p3.y - p4.y ) * height;

    float dist1 = max( length( vec2( dist1X, dist1Y ) ), 1 );
    float dist2 = max( length( vec2( dist2X, dist2Y ) ), 1 );

    float total = dist1 + dist2;

    outputSubLine( p1, p2, dist1 / total, total, s );
    EndPrimitive();

    outputSubLine( p3, p4, dist2 / total, total, s );
    EndPrimitive();
}

void wrapLineY( vec4 p1, vec4 p2, int s )
{
    if( p2.y < p1.y )
    {
        vec4 temp2 = p2;
        p2 = p1;
        p1 = temp2;
    }

    float r = ( W2 - p2.y ) / ( ( p1.y - W1 ) + ( W2 - p2.y ) );
    vec4 tmp = p2;
    tmp.y = W2;
    tmp.x = p2.x + r * ( p1.x - p2.x );
    vec4 tmp2 = tmp;
    tmp2.y = W1;

    outputSplit( p2, tmp, tmp2, p1, s );
}

void wrapLine( vec4 p1, vec4 p2, int s )
{
    if( p2.x < p1.x )
    {
        vec4 temp2 = p2;
        p2 = p1;
        p1 = temp2;
    }
    bool wrapX = XWRAPS && ( ( min( p1.x, p2.x ) - W1 ) + ( W2 - max( p1.x, p2.x ) ) ) < abs( p1.x - p2.x );
    bool wrapY = YWRAPS && ( ( min( p1.y, p2.y ) - W1 ) + ( W2 - max( p1.y, p2.y ) ) ) < abs( p1.y - p2.y );
    if( wrapX )
    {
        float r = ( W2 - p2.x ) / ( ( p1.x - W1 ) + ( W2 - p2.x ) );

        vec4 tmp = p2;
        tmp.x = W2;

        if( ! wrapY )
        {
            tmp.y = p2.y + r * abs( p1.y - p2.y );
            vec4 tmp2 = tmp;
            tmp2.x = W1;

            outputSplit( p2, tmp, tmp2, p1, s );
        }
        else
        {
            float slope;

            if( p2.y > p1.y )
            {
                slope = ( ( p1.y - W1 ) + ( W2 - p2.y ) )
                      / ( ( p1.x - W1 ) + ( W2 - p2.x ) );
            }
            else
            {
                slope = ( ( p2.y - W1 ) + ( W2 - p1.y ) )
                      / ( ( p1.x - W1 ) + ( W2 - p2.x ) );
            }

            float x2Cross = W2;
            float x1Cross = W1;

            float y2Cross = p2.y + slope * ( W2 - p2.x );
            float y1Cross = p1.y - slope * ( p1.x - W1 );

            tmp.x = x2Cross;
            tmp.y = y2Cross;

            vec4 tmp2 = tmp;
            tmp2.x = x1Cross;
            tmp2.y = y1Cross;

            outputSplit( p2, tmp, tmp2, p1, s );
        }
    }
    else if( wrapY )
    {
        wrapLineY( p1, p2, s );
    }
    else
    {
        float distX = abs( p1.x - p2.x ) * width;
        float distY = abs( p1.y - p2.y ) * height;
        float dist = max( length( vec2( distX, distY ) ), 1 );
        outputSubLine( p1, p2, 1, dist, s );
    }
}

void main( void )
{
    int s = vertices[ 0 ].select * vertices[ 1 ].select;
    int r = vertices[ 0 ].isReal * vertices[ 1 ].isReal;

    if( bool( r ) )
    {
        vec4 p1 = gl_in[0].gl_Position;
        vec4 p2 = gl_in[1].gl_Position;

        wrapLine( p1, p2, s );
    }
}
