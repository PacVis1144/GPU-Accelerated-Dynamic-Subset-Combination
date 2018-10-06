//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

#ifndef TN_TEXTURE_LAYER_HPP
#define TN_TEXTURE_LAYER_HPP

#include "TNTexture.hpp"
#include "Types/Vec.hpp"

#include <GL/glew.h>

#include <omp.h>
#include <vector>
#include <utility>
#include <limits>

namespace TNR
{

struct TextureLayer
{
    TNR::Texture2D1 tex;
    int width;
    int height;

    float minValue;
    float maxValue;

    std::vector< float > m_storage;

    std::vector< float > & get( )
    {
        m_storage.resize( width*height );

        tex.bind();
        glGetTexImage(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            GL_FLOAT,
            ( void * ) m_storage.data() );

        return m_storage;
    }

    TN::Vec2< float > getRangeInRadius( TN::Vec2< float > & center,  TN::Vec2< float >  & radius )
    {
        float mx = 0;
        float mn = std::numeric_limits< float >::max();

        if( m_storage.size() < width*height )
        {
            return { 0, 0 };
        }

        tex.bind();
        glGetTexImage(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            GL_FLOAT,
            ( void * ) m_storage.data() );

        const int END = width * height;

        for( int i = 0; i < END; ++i )
        {
            int r = i / width;
            int c = i % width;

            float rn = 2.f * ( (float ) r / height ) - 1;
            float cn = 2.f * ( (float ) c / width  ) - 1;

            const float relativeRadius =
                sqrt(
                    pow( ( cn - center.x() ) / radius.x(), 2.0 ) +
                    pow( ( rn - center.y() ) / radius.y(), 2.0 ) );

            if( relativeRadius <= 1.0 )
            {
                if( m_storage[ i ] > mx )
                {
                    mx = m_storage[ i ];
                }
                if( m_storage[ i ] < mn && m_storage[ i ] > 0 )
                {
                    mn = m_storage[ i ];
                }
            }
        }

        return { mn, mx };
    }

    void minMask( float m )
    {
        if( m_storage.size() < width * height )
        {
            m_storage.resize( width * height );
        }

        tex.bind();
        glGetTexImage(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            GL_FLOAT,
            ( void * ) m_storage.data() );

        const int END = width * height;

        #pragma omp parallel for
        for( int i = 0; i < END; ++i )
        {
            if( m_storage[ i ] <= m*0.9999 )
            {
                m_storage[ i ] = 0;
            }
        }

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_R32F,
            width,
            height,
            0,
            GL_RED,
            GL_FLOAT,
            (void*) m_storage.data() );
    }

    void maxMask( float m )
    {
        if( m_storage.size() < width * height )
        {
            m_storage.resize( width * height );
        }

        tex.bind();
        glGetTexImage(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            GL_FLOAT,
            ( void * ) m_storage.data() );

        const int END = width * height;

        #pragma omp parallel for
        for( int i = 0; i < END; ++i )
        {
            if( m_storage[ i ] >= m * 1.0001 )
            {
                m_storage[ i ] = 0;
            }
        }

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_R32F,
            width,
            height,
            0,
            GL_RED,
            GL_FLOAT,
            (void*) m_storage.data() );
    }

    void computeRange()
    {
        if( m_storage.size() < width * height )
        {
            m_storage.resize( width * height );
        }

        tex.bind();
        glGetTexImage(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            GL_FLOAT,
            ( void * ) m_storage.data() );

        const int END = width * height;

        float mx = m_storage[ 0 ];
        float mn = std::numeric_limits< float >::max();

        for( int i = 0; i < END; ++i )
        {
            if( m_storage[ i ] > mx )
            {
                mx = m_storage[ i ];
            }
            if( m_storage[ i ] < mn/* &&  m_storage[ i ] > 0*/ )
            {
                mn = m_storage[ i ];
            }
        }

        maxValue = mx;
        minValue = mn;
    }

    void loadMaxes( int w, int h )
    {
        std::vector< float > maxes( width*height, std::numeric_limits< float >::max() );

        width = w;
        height = h;

        if( tex.id() == 0 )
        {
            tex.create();
            tex.loadZeros( width, height );
        }

        tex.load( maxes, width, height );
    }

    void load( std::vector< float > & data, int w, int h )
    {
        if( tex.id() == 0 )
        {
            tex.create();
        }

        width = w;
        height = h;

        tex.load( data, width, height );

        float mx = data[ 0 ];
        float mn = std::numeric_limits< float >::max();

        for( size_t i = 0, END = data.size(); i < END; ++i )
        {
            if( data[ i ] > mx )
            {
                mx = data[ i ];
            }
            if( data[ i ] < mn )
            {
                mn = data[ i ];
            }
        }

        maxValue = mx;
        minValue = mn;
    }

    void resize( int w, int h )
    {
        if( tex.id() == 0 )
        {
            tex.create();
        }

        width = w;
        height = h;

        tex.loadZeros( width, height );
    }
};

}

#endif
