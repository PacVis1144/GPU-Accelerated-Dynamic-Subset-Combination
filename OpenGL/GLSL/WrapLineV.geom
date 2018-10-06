#version 400

//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

#define PIo2 1.57079632679

layout(lines) in;
layout(line_strip, max_vertices=8) out;

uniform sampler1D angleToNormalization;
uniform float width;
uniform float height;
uniform bool YWRAPS;
uniform bool XWRAPS;

const float W1 = -1;
const float W2 =  1;

in vData
{
    float weight;
    flat int select;
    flat int isReal;
} vertices[];


out G2F{
    float valAll;
    float valComb;
    float norm;
} g2f;

void outputSubLine( vec4 p1, vec4 p2, float wt1, float wt2, float portion, float totalDist, int s )
{
    float distX = abs( p2.x - p1.x ) * width;
    float distY = abs( p2.y - p1.y ) * height;
    vec2 a = normalize( vec2( distX, distY ) );
    vec2 b = vec2( 1, 0 );
    float angle = acos( dot( a, b ) );
    float norm = portion / ( texture( angleToNormalization, angle / PIo2 ).r * totalDist );

    gl_Position = p1;
    g2f.valAll   =  wt1;
    g2f.valComb  =  wt1 * s;
    g2f.norm =  norm;
    EmitVertex();

    gl_Position = p2;
    g2f.valAll   = wt2;
    g2f.valComb  = wt2 * s;
    g2f.norm =  norm;
    EmitVertex();
}

void outputSplit( vec4 p1, vec4 p2, vec4 p3, vec4 p4, float wt1, float wt2, float wt3, float wt4, int s )
{
    float dist1X = abs( p1.x - p2.x ) * width;
    float dist1Y = abs( p1.y - p2.y ) * height;

    float dist2X = abs( p3.x - p4.x ) * width;
    float dist2Y = abs( p3.y - p4.y ) * height;

    float dist1 = max( length( vec2( dist1X, dist1Y ) ), 1 );
    float dist2 = max( length( vec2( dist2X, dist2Y ) ), 1 );

    float total = dist1 + dist2;

    outputSubLine( p1, p2, wt1, wt2, dist1 / total, total, s );
    EndPrimitive();

    outputSubLine( p3, p4, wt3, wt4, dist2 / total, total, s );
    EndPrimitive();
}

void wrapLineY( vec4 p1, vec4 p2, float wt1, float wt2, int s )
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

    float wWrap = wt2 + r * ( wt2 - wt1 );

    outputSplit( p2, tmp, tmp2, p1, wt2, wWrap, wWrap, wt1, s );
}

void wrapLine( vec4 p1, vec4 p2, float wt1, float wt2, int s )
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
            float wWrap = wt2 + r * ( wt2 - wt1 );

            outputSplit( p2, tmp, tmp2, p1, wt2, wWrap, wWrap, wt1, s );
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

            float lng1 = length( p2 - tmp );
            float lng2 = length( tmp2 - p1 );
            float totalLng = lng1 + lng2;
            float r = lng2 / totalLng;
            float wWrap = wt2 + r * ( wt2 - wt1 );

            outputSplit( p2, tmp, tmp2, p1, wt2, wWrap, wWrap, wt1, s );
        }
    }
    else if( wrapY )
    {
        wrapLineY( p1, p2, wt1, wt2, s );
    }
    else
    {
        float distX = abs( p1.x - p2.x ) * width;
        float distY = abs( p1.y - p2.y ) * height;
        float dist = max( length( vec2( distX, distY ) ), 1 );
        outputSubLine( p1, p2, wt1, wt2, 1, dist, s );
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
        float wt1 = vertices[ 0 ].weight;
        float wt2 = vertices[ 1 ].weight;

        wrapLine( p1, p2, wt1, wt2, s );
    }
}


//#version 400

////============================================================================
////  Copyright (c) 2016 University of California Davis
////  All rights reserved.
////  See LICENSE.txt for details.
////  This software is distributed WITHOUT ANY WARRANTY; without even
////  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
////  PURPOSE.  See the above copyright notice for more information
////============================================================================

//#define PIo2 1.57079632679

//layout(lines) in;
//layout(line_strip, max_vertices=8) out;

//uniform sampler1D angleToNormalization;
//uniform float width;
//uniform float height;
//uniform bool YWRAPS;
//uniform bool XWRAPS;

//in vData
//{
//    float weight;
//    flat int select;
//    flat int isReal;
//} vertices[];


//out G2F{
//    float valAll;
//    float valComb;
//    float norm;
//} g2f;

//void main( void )
//{
//    float wrap1 = -1;
//    float wrap2 =  1;

//    int s = vertices[ 0 ].select * vertices[ 1 ].select;
//    int r = vertices[ 0 ].isReal * vertices[ 1 ].isReal;

//    if( bool( r ) )
//    {
//        vec4 pos1 = gl_in[0].gl_Position;
//        vec4 pos2 = gl_in[1].gl_Position;

//        float distX = abs( pos2.x - pos1.x ) * width;
//        float distY = abs( pos2.y - pos1.y ) * height;
//        vec2 a = normalize( vec2( distX, distY ) );
//        vec2 b = vec2( 1, 0 );
//        float angle = acos( dot( a, b ) );
//        float aNorm = texture( angleToNormalization, angle / PIo2 ).r;
//        float dist = max( length( vec2( distX, distY ) ), 1 );
//        float norm;

//        /////////////////////////////////////////////////////////////////////

//        float x1 = gl_in[0].gl_Position.x;
//        float x2 = gl_in[1].gl_Position.x;

//        vec4 p1 = gl_in[0].gl_Position;
//        vec4 p2 = gl_in[1].gl_Position;

//        float w1 = vertices[ 0 ].weight;
//        float w2 = vertices[ 1 ].weight;

//        if( x2 < x1 )
//        {
//            float temp = x2;
//            x2 = x1;
//            x1 = temp;

//            vec4 temp2 = p2;
//            p2 = p1;
//            p1 = temp2;

//            float wTemp = w2;
//            w2 = w1;
//            w1 = wTemp;
//        }

//        float wrapDist = ( x1 - wrap1 ) + ( wrap2 - x2 );
//        float strtDist =   x2 - x1;

//        if( wrapDist < strtDist )
//        {
//            float r = ( wrap2 - x2 ) / wrapDist;
//            float yWrap = p2.y + r * ( p1.y - p2.y );
//            float wWrap = w2   + r * ( w2 - w1 );

//            // 0 -------|___0

//            distX = abs( wrap2 - p2.x ) * width;
//            distY = abs( yWrap - p2.y ) * height;
//            dist = max( length( vec2( distX, distY ) ), 1 );
//            norm = r / ( dist * aNorm );

//            g2f.valAll    = w2;
//            g2f.valComb   = w2 * s;
//            g2f.norm =  norm;
//            gl_Position = p2;
//            EmitVertex();

//            vec4 tmp = p2;
//            tmp.x = wrap2;
//            tmp.y = yWrap;

//            g2f.valAll    =  wWrap;
//            g2f.valComb   =  wWrap * s;
//            g2f.norm =  norm;
//            gl_Position = tmp;
//            EmitVertex();

//            EndPrimitive();

//            // 0 __|--------0

//            distX = abs( p1.x - wrap1 ) * width;
//            distY = abs( p1.y - yWrap ) * height;
//            dist = max( length( vec2( distX, distY ) ), 1 );
//            norm = ( 1.0 - r ) / ( dist * aNorm  );

//            tmp.y = yWrap;
//            tmp.x = wrap1;

//            g2f.valAll    = wWrap;
//            g2f.valComb   = wWrap * s;
//            g2f.norm =  norm;
//            gl_Position = tmp;
//            EmitVertex();

//            g2f.valAll    = w1;
//            g2f.valComb   = w1 * s;
//            g2f.norm =  norm;
//            gl_Position = p1;
//            EmitVertex();

//            EndPrimitive();
//        }
//        else
//        {
//            norm = 1.0 / ( dist * aNorm );

//            g2f.valAll    =  vertices[ 0 ].weight;
//            g2f.valComb   =  vertices[ 0 ].weight  * s;
//            g2f.norm =  norm;
//            gl_Position = gl_in[0].gl_Position;
//            EmitVertex();


//            g2f.valAll    =   vertices[ 1 ].weight;
//            g2f.valComb   =   vertices[ 1 ].weight * s;
//            g2f.norm =  norm;
//            gl_Position = gl_in[1].gl_Position;
//            EmitVertex();

//            //EndPrimitive();
//        }
//    }
//}
