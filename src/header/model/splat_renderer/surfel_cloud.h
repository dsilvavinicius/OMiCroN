#ifndef SURFEL_CLOUD_H
#define SURFEL_CLOUD_H

#include "splat_renderer/surfel.hpp"
#include "Array.h"

class SurfelCloud
{
public:
	SurfelCloud();
	SurfelCloud( const model::Array< Surfel >& surfels, const Eigen::Matrix4f& model = Eigen::Matrix4f::Identity() );
	~SurfelCloud();
	void render() const;
	uint numPoints() const { return m_numPts; }
	const Matrix4f& model() const { return m_model; }
	
private:
	GLuint m_vbo, m_vao;
    uint m_numPts;
	Matrix4f m_model;
};

inline SurfelCloud::SurfelCloud()
: m_vbo( 0 ),
m_vao( 0 ),
m_numPts( 0 ),
m_model( Eigen::Matrix4f::Identity() )
{}

inline SurfelCloud::SurfelCloud( const model::Array< Surfel >& surfels, const Matrix4f& model )
: m_model( model )
{
	glGenBuffers(1, &m_vbo);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // Center c.
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Surfel), reinterpret_cast<const GLfloat*>(0));

    // Tagent vector u.
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(Surfel), reinterpret_cast<const GLfloat*>(12));

    // Tangent vector v.
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
        sizeof(Surfel), reinterpret_cast<const GLfloat*>(24));
	
	m_numPts = static_cast< uint >( surfels.size() );
	
	glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Surfel) * m_numPts, surfels.data(), GL_STATIC_DRAW);
	
	glBindVertexArray(0);
}

inline SurfelCloud::~SurfelCloud()
{
	glDeleteVertexArrays( 1, &m_vao );
    glDeleteBuffers( 1, &m_vbo );
}

inline void SurfelCloud::render() const
{
	glBindVertexArray( m_vao );
	glDrawArrays( GL_POINTS, 0, m_numPts );
	glBindVertexArray( 0 );
}

#endif