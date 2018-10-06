//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

#include <GL/glew.h>
#include "TNTexture.hpp"

namespace TNR
{

Texture::Texture() : m_id( 0 )
{}

GLuint & Texture::id()
{
    return m_id;
}

void Texture::create()
{
    glGenTextures( 1, &m_id );
}

Texture::~Texture()
{
    if( m_id != 0 )
    {
        glDeleteTextures( 1, &m_id );
    }
}

///////////////////////////////

Texture2D::Texture2D() : Texture::Texture()
{}

void Texture2D::bind()
{
    glBindTexture( GL_TEXTURE_2D, m_id );
}

//

Texture1D::Texture1D() : Texture::Texture()
{}

void Texture1D::bind()
{
    glBindTexture( GL_TEXTURE_1D, m_id );
}

//////////////////////////////////////////////////

Texture1D3::Texture1D3() : Texture1D::Texture1D()
{}

void Texture1D3::load( std::vector< TNR::Vec3< float > > & data )
{
    glBindTexture( GL_TEXTURE_1D, m_id );
    glTexImage1D( GL_TEXTURE_1D, 0, GL_RGB32F, data.size(), 0, GL_RGB, GL_FLOAT, data.data() );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

//////////////////////////////////////////////////

Texture2D1::Texture2D1() : Texture2D::Texture2D()
{}

void Texture2D1::loadZeros( int width, int height )
{
    glBindTexture( GL_TEXTURE_2D, m_id );
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
}

void Texture2D1::load( std::vector< float > & data, int width, int height )
{
    glBindTexture( GL_TEXTURE_2D, m_id );
    glBindTexture( GL_TEXTURE_2D, m_id );
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, (void*) data.data() );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
}

Texture1D1::Texture1D1() : Texture1D::Texture1D()
{}

void Texture1D1::load( const std::vector< float > & data )
{
    glBindTexture( GL_TEXTURE_1D, m_id );
    glTexImage1D( GL_TEXTURE_1D, 0, GL_R32F, data.size(), 0, GL_RED, GL_FLOAT, data.data() );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}


}

