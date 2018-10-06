//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

#ifndef TN_SHADER_PROGRAM_HPP
#define TN_SHADER_PROGRAM_HPP

#include <GL/glew.h>
#include <list>
#include <memory>
#include <vector>
#include <iostream>
#include <fstream>

struct TNShaderProgram
{

private:

    std::string getSourceFromFile( const std::string & path )
    {
        std::ifstream inFile( path, std::ios::in );
        if( ! inFile.is_open() )
        {
            std::cerr << "Failed to open: " << path << "\n";
        }

        std::string source;
        std::string line;
        while( std::getline( inFile, line) ) 
        {
            source.append(line + "\n");
        }
        inFile.close();
        return source;
    }

    struct TNShader
    {
        GLuint id;
        GLenum type;

            TNShader( GLenum t ) : id( 0 ), type( t ) {}
        ~TNShader() 
        {
            glDeleteShader( id );
        }

        void compile( const std::string & source )
        {
            if( ! id )
            {
                id = glCreateShader( type );
            }

                const GLchar * data = reinterpret_cast< const GLchar * >( source.c_str() );
                glShaderSource( id, 1, &data, 0 );
                glCompileShader( id );
        }
    };

    GLuint m_id;
    std::list< std::unique_ptr< TNShader > > m_shaders;

public:

    void bind()
    {
        glUseProgram( m_id );
    }

    void release()
    {
        glUseProgram( 0 );   
    }

    void enableAttributeArray( GLuint loc )
    {
        glEnableVertexAttribArray( loc );
    }

    void disableAttributeArray( GLuint loc )
    {
        glDisableVertexAttribArray( loc );
    }

    GLuint attributeLocation( const std::string & name )
    {
        return glGetAttribLocation( m_id, name.c_str() );
    }

    void setAttributeBuffer( GLuint index, int type, int offset, int tupleSize, GLboolean normalize = GL_TRUE , int stride = 0 )
    {
        glVertexAttribPointer( index, tupleSize, type, GL_TRUE, stride, reinterpret_cast<const void *>( offset ) );
    }

    void setUniformValue( const std::string & name, GLuint value )
    {
        glUniform1ui( glGetUniformLocation( m_id, name.c_str() ), value );
    }

    void setUniformValue( const std::string & name, GLint value )
    {
        glUniform1i( glGetUniformLocation( m_id, name.c_str() ), value );
    }

    void setUniformValue( const std::string & name, GLfloat value )
    {
        glUniform1fv( glGetUniformLocation( m_id, name.c_str() ), 1, &value );
    }

    void setUniformValue( const std::string & name, bool value )
    {
        setUniformValue( name, ( int ) value );
    }

    void setUniformMat4( const std::string & name, const float * data )
    {
        glUniformMatrix4fv( glGetUniformLocation( m_id, name.c_str() ), 1, GL_FALSE, data );
    }

    bool link()
    {
        glLinkProgram( m_id );

        GLint success = 0;
        glGetProgramiv( m_id, GL_LINK_STATUS, &success );

        if( success == GL_FALSE )
        {
            GLint messageLength = 0;
            glGetProgramiv( m_id, GL_INFO_LOG_LENGTH, &messageLength );

            std::vector<GLchar> errorMessage( messageLength );
            glGetProgramInfoLog( m_id, messageLength, &messageLength, errorMessage.data() );
        
            std::cerr << ( (char *) errorMessage.data() ) << "\n";

            return false;
        }

        for( auto & shader : m_shaders )
        {
            glDetachShader( m_id, shader->id );
        }

        return true;
    }

    TNShaderProgram() : m_id( 0 ) {}
    ~TNShaderProgram() 
    {
        glDeleteProgram( m_id );
    }

    void addShaderFromSource( const std::string & source, GLenum type )
    {
        if( ! m_id )
        {
            m_id = glCreateProgram();
        }

        std::unique_ptr< TNShader > shader = std::unique_ptr< TNShader >( new TNShader( type ) );
        shader->compile( source.c_str() );
        glAttachShader( m_id, shader->id );
        m_shaders.push_back( std::move( shader ) );
    }   

    void addShaderFromFile( const std::string & source, GLenum type )
    {
        addShaderFromSource( getSourceFromFile( source ), type );
    }
};

#endif
