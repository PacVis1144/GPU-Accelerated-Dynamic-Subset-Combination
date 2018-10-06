//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================
#ifndef TN_FRAME_BUFFER_HPP
#define TN_FRAME_BUFFER_HPP

#include <GL/glew.h>

#include "TNTextureLayer.hpp"
#include "TNTexture.hpp"

namespace TNR
{

struct FrameBuffer
{
    int width;
    int height;

    GLuint frameBufferId;
    TNR::Texture1D1 angleToPixelDensityTexture;

    FrameBuffer() : frameBufferId( 0 )
    {}

    ~FrameBuffer()
    {
        if( frameBufferId != 0 )
        {
            glDeleteFramebuffers( 1, &frameBufferId );
        }
    }

    void setAngleToPixelDensityTexture( const std::vector< float > & data )
    {
        angleToPixelDensityTexture.load( data );
    }

    // must be called after openGL has already been initialized
    void init()
    {
        glGenFramebuffers( 1, & frameBufferId );
        angleToPixelDensityTexture.create();
    }

    void release()
    {
        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    }

    void bind()
    {
        glBindFramebuffer( GL_FRAMEBUFFER, frameBufferId );
    }

    void setTextures( std::vector< TextureLayer * > & layers )
    {
        GLint maxDrawBuf = 0;
        glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuf );

        GLint maxAttach = 0;
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttach);

        const unsigned int SUPPORTED_NUM_ATTACHMENTS = 16;

        if( layers.size() > maxAttach 
         || layers.size() > SUPPORTED_NUM_ATTACHMENTS 
         || layers.size() > maxDrawBuf )
        {
            std::cerr << "Too many texures to attach\n";
        }

        const GLenum COLOR_ATTACHMENTS[] = 
        {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3,
            GL_COLOR_ATTACHMENT4,
            GL_COLOR_ATTACHMENT5,
            GL_COLOR_ATTACHMENT6,
            GL_COLOR_ATTACHMENT7,  
            GL_COLOR_ATTACHMENT8,
            GL_COLOR_ATTACHMENT9,
            GL_COLOR_ATTACHMENT10,
            GL_COLOR_ATTACHMENT11,
            GL_COLOR_ATTACHMENT12,
            GL_COLOR_ATTACHMENT13,
            GL_COLOR_ATTACHMENT14,
            GL_COLOR_ATTACHMENT15,                                                            
        };
 
        glDeleteFramebuffers( 1, &frameBufferId );
        glGenFramebuffers( 1, &frameBufferId );

        bind();

        if( layers.size() > 0 )
        {
            for( size_t i = 0, end = layers.size(); i < end; ++i )
            {
                layers[ i ]->tex.bind();
                glFramebufferTexture2D(GL_FRAMEBUFFER, COLOR_ATTACHMENTS[ i ], GL_TEXTURE_2D, layers[ i ]->tex.id(), 0);
                if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
                {
                    std::cerr << "frame buffer incomplete";
                }
            }
            glDrawBuffers( layers.size(), COLOR_ATTACHMENTS );
        }

        if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
        {
            std::cout << "Frame buffer incomplete\n";
        }

        release();
    }

    void setSize( int _width, int _height )
    {
        width = _width;
        height = _height;
    }
};

}

#endif
