//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

#ifndef TN_VAO_HPP
#define TN_VAO_HPP

#include <GL/glew.h>

struct TNVAO
{
    GLuint m_id;
    
    void bind()
    {
        glBindVertexArray( m_id );
    }

    void release()
    {
        glBindVertexArray( 0 );
    }

    void create()
    {
        if( ! m_id )
        {
            glGenVertexArrays( 1, & m_id );
        }
    }

    void free()
    {
        glDeleteVertexArrays( 1, &m_id );
    }

    TNVAO() : m_id( 0 ) {}
    ~TNVAO()
    {
        free();
    }


};

#endif
