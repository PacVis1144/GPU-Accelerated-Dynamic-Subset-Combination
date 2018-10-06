//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================

#ifndef TN_TEXTURE_HPP_R
#define TN_TEXTURE_HPP_R

#include <GL/glew.h>
#include <vector>

namespace TNR
{

template < typename T >
struct Vec3
{
    T x;
    T y;
    T z;
};

class Texture
{

protected :

GLuint m_id;

public:

Texture();

GLuint & id();
void create();

virtual ~Texture();

virtual void bind() = 0;

};

///////////////////

class Texture2D : public Texture
{

public:

Texture2D();
virtual void bind();

};

class Texture1D : public Texture
{

public:

Texture1D();
virtual void bind();

};

/////////////////

class Texture2D1 : public Texture2D
{

public:

    Texture2D1();

    void loadZeros( int width, int height );
    void load( std::vector< float > & data, int width, int height );
};

class Texture1D1 : public Texture1D
{

public:

    Texture1D1();
    void load( const std::vector< float > & data );
};

class Texture1D3 : public Texture1D
{

public:

    Texture1D3();
    void load( std::vector< TN::Vec3< float > > & data );
};

}


#endif // TEXTURE2D_H


