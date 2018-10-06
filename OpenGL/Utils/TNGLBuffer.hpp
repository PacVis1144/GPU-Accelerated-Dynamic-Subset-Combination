//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

#ifndef TN_GL_BUFFER_HPP
#define TN_GL_BUFFER_HPP

#include <GL/glew.h>
#include <cstdint>

namespace TNR {

class TNGLBuffer
{
    GLuint m_id;
    std::uint64_t m_size;
    GLuint m_usagePattern;

public :

    TNGLBuffer() : m_id( 0 ), m_size( 0 ), m_usagePattern( GL_STATIC_DRAW )
    {}

    void setUsagePattern( int pattern )
    {
        m_usagePattern = pattern;
    }

    void create()
    {
        glGenBuffers( 1, & m_id );
    }

    void bind()
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_id );
    }

    void release()
    {
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
    }

    unsigned long long int size()
    {
        return m_size;
    }

    GLuint bufferId()
    {
        return m_id;
    }

    void allocate( long long int N_BYTES )
    {
        m_size = N_BYTES;
        glBufferData( GL_ARRAY_BUFFER, N_BYTES, NULL, m_usagePattern );
    }

    void write( unsigned long long int offset, const void *data, unsigned long long int count )
    {
        glBufferSubData( GL_ARRAY_BUFFER, offset, count, data );
    }

    void free()
    {
        glDeleteBuffers( 1, &m_id );
    }

    ~TNGLBuffer()
    {
        free();
    }
};

}

#endif
