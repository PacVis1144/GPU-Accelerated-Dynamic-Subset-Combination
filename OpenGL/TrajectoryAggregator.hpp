//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

#include "Utils/TNFrameBuffer.hpp"
#include "Utils/TNTextureLayer.hpp"
#include "Utils/TNGLBuffer.hpp"
#include "Utils/TNVAO.hpp"
#include "Utils/TNShaderProgram.hpp"

#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <cstdint>
#include <map>
#include <iostream>

#ifndef TN_TRAJECTORY_AGGREGATOR_HPP
#define TN_TRAJECTORY_AGGREGATOR_HPP

namespace TNR
{

class TrajectoryAggregator
{
    static const GLuint LINE_WIDTH = 1;

    QObject * m_parent;

    size_t m_numTrajectories;
    size_t m_numTimeSteps;
    std::map< std::string, int > m_vboIndices;

    TNR::FrameBuffer m_fb;

    std::unique_ptr<TNVAO> m_textureVAO;
    std::unique_ptr<TNGLBuffer> m_textureVBO;

    std::unique_ptr<TNVAO> m_dataVAO;
    std::unique_ptr<TNGLBuffer> m_dataVBO;

    TNR::TextureLayer m_countAll;
    TNR::TextureLayer m_countComb;

    TNR::TextureLayer m_sumAll;
    TNR::TextureLayer m_sumComb;

    TNR::Texture1D1 m_angleToPixelDensityTexture;

    std::unique_ptr<TNShaderProgram> m_densityProgram;
    std::unique_ptr<TNShaderProgram> m_densityProgramWeighted;
    std::unique_ptr<TNShaderProgram> m_minMaxProgram;
    std::unique_ptr<TNShaderProgram> m_summationProgram;
    std::unique_ptr<TNShaderProgram> m_textureDifferenceProgram;
    std::unique_ptr<TNShaderProgram> m_textureDivideProgram;
    std::unique_ptr<TNShaderProgram> m_varianceProgram;
    std::unique_ptr<TNShaderProgram> m_diagnosticProgram;

    // For plots where one or more axis wrap
    std::unique_ptr<TNShaderProgram> m_densityProgramWrapAround;
    std::unique_ptr<TNShaderProgram> m_densityProgramWeightedWrapAround;
    std::unique_ptr<TNShaderProgram> m_minMaxProgramWrapAround;
    std::unique_ptr<TNShaderProgram> m_summationProgramWrapAround;
    std::unique_ptr<TNShaderProgram> m_varianceProgramWrapAround;

    void compileProgram(
        const std::string & BASE_PATH,
        std::unique_ptr<TNShaderProgram> & program,
        const std::string & vs,
        const std::string & fs,
        const std::string & gs = "" )
    {
        program = std::unique_ptr<TNShaderProgram>( new TNShaderProgram );

        program->addShaderFromFile( BASE_PATH + vs, GL_VERTEX_SHADER );
        program->addShaderFromFile( BASE_PATH + fs, GL_FRAGMENT_SHADER );
        if( gs != "" )
        {
            program->addShaderFromFile( BASE_PATH + gs, GL_GEOMETRY_SHADER );
        }
        program->link();
    }

    void uniformTextures(
        std::unique_ptr<TNShaderProgram> & program,
        const std::vector< TNR::Texture2D1 * > textures2D,
        const std::vector< std::string > & textures2DNames,
        const std::vector< TNR::Texture1D1 * > textures1D,
        const std::vector< std::string > & textures1DNames )
    {
        m_textureVAO->bind();
        m_textureVBO->bind();

        const std::array< GLenum, 16 > TEX_ENUMS =
        {
            GL_TEXTURE0,
            GL_TEXTURE1,
            GL_TEXTURE2,
            GL_TEXTURE3,
            GL_TEXTURE4,
            GL_TEXTURE5,
            GL_TEXTURE6,
            GL_TEXTURE7,
            GL_TEXTURE8,
            GL_TEXTURE9,
            GL_TEXTURE10,
            GL_TEXTURE11,
            GL_TEXTURE12,
            GL_TEXTURE13,
            GL_TEXTURE14,
            GL_TEXTURE15
        };

        for( int i = 0; i < textures2D.size(); ++i )
        {
            glActiveTexture( TEX_ENUMS[ i ] );
            textures2D[ i ]->bind();
            glGenerateMipmap( GL_TEXTURE_2D );
        }
        for( int i = 0; i < textures1D.size(); ++i )
        {
            glActiveTexture( TEX_ENUMS[ i + textures2D.size() ] );
            textures1D[ i ]->bind();
        }
        for( int i = 0; i < textures2D.size(); ++i )
        {
            program->setUniformValue( textures2DNames[ i ].c_str(), i );
        }
        for( int i = 0; i < textures1D.size(); ++i )
        {
            program->setUniformValue( textures1DNames[ i ].c_str(), static_cast<GLuint>( i + textures2DNames.size() ) );
        }

        glActiveTexture( GL_TEXTURE0 );
    }

    void renderQuadTexture( std::unique_ptr<TNShaderProgram> & program )
    {
        m_textureVBO->bind();
        m_textureVAO->bind();

        GLuint posAttr = program->attributeLocation( "ps" );
        GLuint uvAttr  = program->attributeLocation( "tc" );

        program->enableAttributeArray( posAttr );
        program->enableAttributeArray( uvAttr );

        program->setAttributeBuffer( posAttr, GL_FLOAT, 0, 2 );
        program->setAttributeBuffer( uvAttr,  GL_FLOAT, 12 * sizeof( float ), 2 );

        glDrawArrays( GL_TRIANGLES, 0, 6 );

        program->disableAttributeArray( posAttr );
        program->disableAttributeArray( uvAttr );

        program->release();

        m_textureVBO->release();
        m_textureVAO->release();
    }

    void finishAndRestoreGLState( std::vector< TNR::TextureLayer * > layers )
    {
        for( auto & layer : layers )
        {
            layer->computeRange();
        }
        m_fb.release();
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }

    void prepareFBO( int width, int height, std::vector< TNR::TextureLayer * > layers )
    {
        m_fb.setSize( width, height );
        for( auto layer : layers )
        {
            layer->resize( width, height );
        }
        m_fb.setTextures( layers );
        m_fb.bind();
    }

    void prepareViewPort( int width, int height, GLenum equation, std::unique_ptr<TNShaderProgram> & program )
    {
        glEnable( GL_BLEND );
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation( equation );
        glLineWidth( LINE_WIDTH );
        glClearColor( 0.0, 0.0, 0.0, 1.0 );
        glViewport( 0, 0, width, height );
        glClear( GL_COLOR_BUFFER_BIT );
        program->bind();
    }

    void prepareVertexBuffersAndRender(
        std::unique_ptr<TNShaderProgram> & program,
        const std::vector< std::string > & programAttrNames,
        const std::vector< int > & vboOffsets,
        const size_t CBMSK_OFFSET,
        const size_t NPT )
    {
        m_dataVAO->bind();
        m_dataVBO->bind();

        for( int i = 0; i < programAttrNames.size(); ++i )
        {
            program->enableAttributeArray(  program->attributeLocation( programAttrNames[ i ].c_str() ) );
        }

        GLuint c  = program->attributeLocation(  "c" );
        program->enableAttributeArray( c );

        for( int i = 0; i < programAttrNames.size(); ++i )
        {
            program->setAttributeBuffer(
                program->attributeLocation( programAttrNames[ i ].c_str() ),
                GL_FLOAT,
                vboOffsets[ i ] * NPT * sizeof( float ), 1 );
        }

        glVertexAttribIPointer( c, 1, GL_INT, sizeof( int ), (void*)( CBMSK_OFFSET*NPT*sizeof( float ) ) );

        glDrawArrays( GL_LINE_STRIP, 0, NPT );

        for( int i = 0; i < programAttrNames.size(); ++i )
        {
            program->disableAttributeArray( program->attributeLocation( programAttrNames[ i ].c_str() ) );
        }

        program->disableAttributeArray( c );

        m_dataVBO->release();
        m_dataVAO->release();
    }

public:

    TrajectoryAggregator( QObject *parent = Q_NULLPTR ) : m_parent( parent ) {}
    ~TrajectoryAggregator() {}

    // must be called after OpenGL has been initialized
    void init()
    {
        compilePrograms( std::string( "../Code/Shared/VIDI/Tyson/Repo/OpenGL/GLSL/" ) );

        m_fb.init();

        m_textureVAO = std::unique_ptr<TNVAO>( new TNVAO  );
        m_textureVBO = std::unique_ptr<TNGLBuffer>( new TNGLBuffer  );
        m_textureVBO->setUsagePattern( GL_STATIC_DRAW );
        m_textureVAO->create();
        m_textureVBO->create();

        const float quad[] =
        {
            -1.f,  1.f,
            -1.f, -1.f,
            1.f, -1.f,
            -1.f,  1.f,
            1.f, -1.f,
            1.f,  1.f,
        };

        const float texcoords[] =
        {
            0.f, 1.f,
            0.f, 0.f,
            1.f, 0.f,
            0.f, 1.f,
            1.f, 0.f,
            1.f, 1.f
        };

        m_textureVAO->bind();
        m_textureVBO->bind();
        m_textureVBO->allocate( 24 * sizeof( float ) );
        m_textureVBO->write( 0, quad, 12 * sizeof( float ) );
        m_textureVBO->write( 12*sizeof(float), texcoords, 12 * sizeof( float ) );
        m_textureVBO->release();
        m_textureVAO->release();

        m_dataVAO = std::unique_ptr<TNVAO>( new TNVAO  );
        m_dataVBO = std::unique_ptr<TNGLBuffer>( new TNGLBuffer );
        m_dataVBO->setUsagePattern( GL_STATIC_DRAW );
        m_dataVAO->create();
        m_dataVBO->create();

        angleToPixelDensityTexture.create();
        computePixelDensityByAngle();
    }

    void setData(
        float * data,
        const size_t num_trajectories,
        const size_t num_timeSteps,
        const std::map< std::string, int > & indexMap )
    {
        const std::uint64_t N_BYTES =
            indexMap.size() * num_trajectories * num_timeSteps * sizeof( float );

        m_numTrajectories = num_trajectories;
        m_numTimeSteps = num_timeSteps;
        m_vboIndices = indexMap;

        m_dataVAO->bind();
        m_dataVBO->bind();
        m_dataVBO->allocate( N_BYTES );
        m_dataVBO->write( 0, data, N_BYTES );
        m_dataVBO->release();
        m_dataVAO->release();
    }

    void compilePrograms( const std::string & BASE_PATH )
    {
        compileProgram( BASE_PATH, m_densityProgram,                 "CombVS.vert",         "LineMap2.frag", "LineMap.geom" );
        compileProgram( BASE_PATH, m_densityProgramWeighted,        "CombVS2.vert",        "LineMap2.frag", "LineMap2W2.geom" );
        compileProgram( BASE_PATH, m_minMaxProgram,           "MinMaxLineMap.vert",    "MinMaxLineMap.frag", "MinMaxLineMap.geom" );
        compileProgram( BASE_PATH, m_summationProgram,              "CombVS2.vert", "LineMapSummation.frag", "LineMap2W.geom" );
        compileProgram( BASE_PATH, m_varianceProgram,               "CombVS2.vert",  "LineMapVariance.frag", "LineMapVariance.geom" );
        compileProgram( BASE_PATH, m_textureDivideProgram,          "quadTex.vert",    "textureDivide.frag" );
        compileProgram( BASE_PATH, m_textureDifferenceProgram,      "quadTex.vert",       "difference.frag" );
        compileProgram( BASE_PATH, m_diagnosticProgram,       "binDiagnostic.vert",    "binDiagnostic.frag" );

        compileProgram( BASE_PATH, m_densityProgramWrapAround,                "CombVS.vert",          "LineMap2.frag", "WrapLine1.geom" );
        compileProgram( BASE_PATH, m_densityProgramWeightedWrapAround,       "CombVS2.vert" ,        "LineMap2.frag", "WrapLine2.geom" );
        compileProgram( BASE_PATH, m_minMaxProgramWrapAround,          "MinMaxLineMap.vert",    "MinMaxLineMap.frag", "WrapLine4.geom" );
        compileProgram( BASE_PATH, m_summationProgramWrapAround,             "CombVS2.vert", "LineMapSummation.frag", "WrapLine3.geom" );
        compileProgram( BASE_PATH, m_varianceProgramWrapAround,              "CombVS2.vert",  "LineMapVariance.frag", "WrapLineV.geom" );     
    }

    void computePixelDensityByAngle()
    {
        std::vector< float > result;
        const int DIM = 500;

        m_fb.setSize( DIM, DIM );
        TNR::TextureLayer tex;
        tex.resize( DIM, DIM );
        std::vector< TNR::TextureLayer * > layers = { &tex };
        m_fb.setTextures( layers );
        m_fb.bind();

        glViewport( 0, 0, DIM, DIM );
        glEnable( GL_BLEND );
        glBlendFunc( GL_ONE, GL_ONE );
        glBlendEquation( GL_FUNC_ADD );
        glDisable( GL_LINE_SMOOTH );
        glLineWidth( LINE_WIDTH );

        std::vector< float > x( 2 );
        std::vector< float > y( 2 );

        std::vector< float > im( DIM*DIM, 0.f );

        TNVAO tmpVAO;
        TNR::TNGLBuffer tmpVBO;
        tmpVBO.setUsagePattern( GL_DYNAMIC_DRAW );
        tmpVAO.create();
        tmpVBO.create();
        tmpVAO.bind();
        tmpVBO.bind();
        tmpVBO.allocate( x.size() * 2 * sizeof( float ) );

        m_diagnosticProgram->bind();
        GLuint xAttr = m_diagnosticProgram->attributeLocation(    "xAttr"    );
        GLuint yAttr = m_diagnosticProgram->attributeLocation(    "yAttr"    );
        m_diagnosticProgram->enableAttributeArray( xAttr );
        m_diagnosticProgram->enableAttributeArray( yAttr );
        m_diagnosticProgram->setAttributeBuffer( xAttr, GL_FLOAT, 0, 1 );
        m_diagnosticProgram->setAttributeBuffer( yAttr, GL_FLOAT, x.size()*sizeof( float ), 1 );

        for( double a = 0.0; a < 3.14159265359 / 2.0; a += 0.001 )
        {
            std::fill( im.begin(), im.end(), 0.f );

            x[ 0 ] = 0;
            y[ 0 ] = 0;

            x[ 1 ] = std::cos( a )*.9;
            y[ 1 ] = std::sin( a )*.9;

            glClearColor( 0.0, 0.0, 0.0, 1.0 );
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            tmpVBO.write( 0, x.data(), x.size() * sizeof( float ) );
            tmpVBO.write( x.size() *  sizeof( float ), y.data(), x.size() * sizeof( float ) );

            glDrawArrays( GL_LINES, 0, x.size() );
            glReadPixels( 0, 0, DIM, DIM, GL_RED, GL_FLOAT, im.data() );

            float sum = 0;
            for ( int p = 0; p < DIM*DIM; ++p )
            {
                sum += im[ p ];
            }
            result.push_back( sum );
        }

        m_diagnosticProgram->disableAttributeArray( xAttr );
        m_diagnosticProgram->disableAttributeArray( yAttr );
        tmpVBO.release();
        tmpVAO.release();
        m_diagnosticProgram->release();
        m_fb.release();
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glDisable( GL_LINE_SMOOTH );

        m_angleToPixelDensityTexture.load( result );
    }

    void computeDensity(
        std::pair< TNR::TextureLayer, TNR::TextureLayer > & layers,
        const std::string & x,
        const std::string & y,
        const int width,
        const int height,
        const QMatrix4x4 & M,
        bool distanceWeighted,
        bool wrapX,
        bool wrapY )
    {
        auto & program = ( wrapX || wrapY ) ? m_densityProgramWrapAround : m_densityProgram;
        prepareFBO( width, height, { &( layers.first ), &( layers.second ) } );
        prepareViewPort( width, height, GL_FUNC_ADD, program );

        glActiveTexture( GL_TEXTURE0 );
        m_fb.angleToPixelDensityTexture.bind();
        glActiveTexture( GL_TEXTURE0 );

        program->setUniformValue( "angleToNormalization", 0 );
        program->setUniformValue( "distanceWeighted", distanceWeighted );
        program->setUniformMat4( "M", M.constData() );
        program->setUniformValue( "width", static_cast< float >( width ) );
        program->setUniformValue( "height", static_cast< float >( height ) );
        program->setUniformValue( "XWRAPS", wrapX );
        program->setUniformValue( "YWRAPS", wrapY );

        prepareVertexBuffersAndRender(
            program,
        { "x1", "x2" },
        { m_vboIndices.at( x ), m_vboIndices.at( y ) },
        m_vboIndices.size(),
        m_numTimeSteps * m_numTrajectories );

        program->release();
        finishAndRestoreGLState( { &( layers.first ), &( layers.second ) } );
    }

    void computeWeightedDensity(
        std::pair< TNR::TextureLayer, TNR::TextureLayer > & layers,
        const std::string & x,
        const std::string & y,
        const std::string & ws,
        const int width,
        const int height,
        const QMatrix4x4 & M,
        bool wrapX,
        bool wrapY )
    {
        auto & program = ( wrapX || wrapY ) ? m_densityProgramWeightedWrapAround : m_densityProgramWeighted;
        prepareFBO( width, height, { &( layers.first ), &( layers.second ) } );
        prepareViewPort( width, height, GL_FUNC_ADD, program );

        glActiveTexture( GL_TEXTURE0 );
        m_fb.angleToPixelDensityTexture.bind();
        glActiveTexture( GL_TEXTURE0 );

        program->setUniformValue( "angleToNormalization", 0 );
        program->setUniformMat4( "M", M.constData() );
        program->setUniformValue( "width",  static_cast< float >( layers.second.width ) );
        program->setUniformValue( "height", static_cast< float >( layers.second.height ) );
        program->setUniformValue( "XWRAPS", wrapX );
        program->setUniformValue( "YWRAPS", wrapY );

        prepareVertexBuffersAndRender(
            program,
        { "x1", "x2", "weight" },
        { m_vboIndices.at( x ), m_vboIndices.at( y ), m_vboIndices.at( ws ) },
        m_vboIndices.size(),
        m_numTimeSteps * m_numTrajectories );

        program->release();
        finishAndRestoreGLState( { &( layers.first ), &( layers.second ) } );
    }

    void computeVariance(
        std::pair< TNR::TextureLayer, TNR::TextureLayer > & layers,
        const std::string & x,
        const std::string & y,
        const std::string & ws,
        const int width,
        const int height,
        const QMatrix4x4 & M,
        bool wrapX,
        bool wrapY )
    {
        computeMean( layers, x, y, ws, width, height, M, wrapX, wrapY );

        auto & program = ( wrapX || wrapY ) ? m_varianceProgramWrapAround : m_varianceProgram;
        prepareFBO( width, height, { &( m_sumAll ), &( m_sumComb ) } );
        prepareViewPort( width, height, GL_FUNC_ADD, program );

        uniformTextures(
            program,
        { &( layers.first.tex ), &( layers.second.tex ) }, { "tex1", "tex2" },
        { &( m_fb.angleToPixelDensityTexture ) },
        { "angleToNormalization" } );

        program->setUniformMat4( "M", M.constData() );
        program->setUniformValue( "width", static_cast< float >( layers.second.width ) );
        program->setUniformValue( "height", static_cast< float >( layers.second.height ) );
        program->setUniformValue( "XWRAPS", wrapX );
        program->setUniformValue( "YWRAPS", wrapY );

        prepareVertexBuffersAndRender(
            program,
        { "x1", "x2", "weight" },
        { m_vboIndices.at( x ), m_vboIndices.at( y ), m_vboIndices.at( ws ) },
        m_vboIndices.size(),
        m_numTimeSteps * m_numTrajectories );

        program->release();

        prepareFBO( width, height, { &( layers.first ), &( layers.second ) } );
        prepareViewPort( width, height, GL_FUNC_ADD, m_textureDivideProgram );

        uniformTextures(
            m_textureDivideProgram,
        { &( m_sumAll.tex ), &( m_countAll.tex ), &( m_sumComb.tex ), &( m_countComb.tex ) },
        { "sumAll", "countAll", "sumComb", "countComb" },
        {}, {} );

        renderQuadTexture( m_textureDivideProgram );
        finishAndRestoreGLState( { &( layers.first ), &( layers.second ) } );
    }

    void computeMean(
        std::pair< TNR::TextureLayer, TNR::TextureLayer > & layers,
        const std::string & x,
        const std::string & y,
        const std::string & ws,
        const int width,
        const int height,
        const QMatrix4x4 & M,
        bool wrapX,
        bool wrapY )
    {
        auto & program = ( wrapX || wrapY ) ? m_summationProgramWrapAround : m_summationProgram;
        prepareFBO( width, height, { &( m_sumAll ), &( m_sumComb ), &( m_countAll ), &( m_countComb ) } );
        prepareViewPort( width, height, GL_FUNC_ADD, program );

        glActiveTexture( GL_TEXTURE0 );
        m_fb.angleToPixelDensityTexture.bind();
        glActiveTexture( GL_TEXTURE0 );

        program->setUniformValue( "angleToNormalization", 0 );
        program->setUniformMat4( "M", M.constData() );
        program->setUniformValue( "width", static_cast< float >( width ) );
        program->setUniformValue( "height", static_cast< float >( height ) );
        program->setUniformValue( "XWRAPS", wrapX );
        program->setUniformValue( "YWRAPS", wrapY );

        prepareVertexBuffersAndRender(
            program,
        { "x1", "x2", "weight" },
        { m_vboIndices.at( x ), m_vboIndices.at( y ), m_vboIndices.at( ws ) },
        m_vboIndices.size(),
        m_numTimeSteps * m_numTrajectories );

        program->release();

        prepareFBO( width, height, { &( layers.first ), &( layers.second ) } );
        prepareViewPort( width, height, GL_FUNC_ADD, m_textureDivideProgram );

        uniformTextures(
            m_textureDivideProgram,
        { &( m_sumAll.tex ), &( m_countAll.tex ), &( m_sumComb.tex ), &( m_countComb.tex ) },
        { "sumAll", "countAll", "sumComb", "countComb" },
        {}, {} );

        renderQuadTexture( m_textureDivideProgram );
        finishAndRestoreGLState( { &( layers.first ), &( layers.second ) } );
    }

    void computeMinOrMax(
        std::pair< TNR::TextureLayer, TNR::TextureLayer > & layers,
        const std::string & x,
        const std::string & y,
        const std::string & ws,
        const int width,
        const int height,
        const QMatrix4x4 & M,
        bool cMin,
        bool wrapX,
        bool wrapY )
    {
        auto & program = ( wrapX || wrapY ) ? m_minMaxProgramWrapAround : m_minMaxProgram;
        prepareFBO( width, height, { &( layers.first ), &( layers.second ) } );
        prepareViewPort( width, height, ( cMin ? GL_MIN : GL_MAX ), program );

        if( cMin )
        {
            glClearColor( std::numeric_limits< float >::max() * 0.95, 0, 0, 1 );
            glClear( GL_COLOR_BUFFER_BIT );
        }
        else
        {
            glClearColor(-std::numeric_limits< float >::max() * 0.95, 0, 0, 1 );
            glClear( GL_COLOR_BUFFER_BIT );
        }

        program->setUniformMat4( "M", M.constData() );
        program->setUniformValue( "XWRAPS", wrapX );
        program->setUniformValue( "YWRAPS", wrapY );

        prepareVertexBuffersAndRender(
            program,
        { "x1", "x2", "weight" },
        { m_vboIndices.at( x ), m_vboIndices.at( y ), m_vboIndices.at( ws ) },
        m_vboIndices.size(),
        m_numTimeSteps * m_numTrajectories );

        program->release();

        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glBlendEquation( GL_FUNC_ADD );

        if( cMin )
        {
            layers.first.maxMask(  std::numeric_limits< float >::max() * 0.9 );
            layers.second.maxMask( std::numeric_limits< float >::max() * 0.9 );
        }
        else
        {
            layers.first.minMask(  -std::numeric_limits< float >::max() * 0.9 );
            layers.second.minMask( -std::numeric_limits< float >::max() * 0.9 );
        }

        finishAndRestoreGLState( { &( layers.first ), &( layers.second ) } );
    }

    void computeRange(
        std::pair< TNR::TextureLayer, TNR::TextureLayer > & layers,
        const std::string & x,
        const std::string & y,
        const std::string & ws,
        const int width,
        const int height,
        const QMatrix4x4 & M,
        bool wrapX,
        bool wrapY )
    {
        std::pair< TNR::TextureLayer, TNR::TextureLayer > p1;
        std::pair< TNR::TextureLayer, TNR::TextureLayer > p2;

        computeMinOrMax( p2, x, y, ws, width, height, M,  true, wrapX, wrapY );
        computeMinOrMax( p1, x, y, ws, width, height, M, false, wrapX, wrapY );

        prepareFBO( width, height, { &( layers.first ), &( layers.second ) } );
        prepareViewPort( width, height, GL_FUNC_ADD, m_textureDifferenceProgram );

        uniformTextures(
            m_textureDifferenceProgram,
        { &( p1.first.tex ), &( p2.first.tex ), &( p1.second.tex ), &( p2.second.tex ) },
        { "tex1", "tex2", "tex3", "tex4" },
        {}, {} );

        renderQuadTexture( m_textureDifferenceProgram );
        finishAndRestoreGLState( { &( layers.first ), &( layers.second ) } );
    }
};

}

#endif
