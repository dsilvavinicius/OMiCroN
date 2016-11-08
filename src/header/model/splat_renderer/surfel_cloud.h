#ifndef SURFEL_CLOUD_H
#define SURFEL_CLOUD_H

#include "splat_renderer/surfel.hpp"
#include "Array.h"
#include "GpuAllocStatistics.h"
#include "OglUtils.h"

#define DEBUG

using namespace model;

/** Surfel cloud that supports async loading. Ctors can be called by a thread sharing Opengl context with a rendering
 * thread. The ctor will ensure VBO creation and data loading. VAO is created later at the first time the cloud is
 * rendered (since VAO cannot be shared among different threads). */
class SurfelCloud
{
public:
	SurfelCloud();
	SurfelCloud( const model::Array< Surfel >& surfels, const Eigen::Matrix4f& model = Eigen::Matrix4f::Identity() );
	SurfelCloud( const SurfelCloud& other ) = delete;
	SurfelCloud( SurfelCloud&& other );
	~SurfelCloud();
	
	SurfelCloud& operator=( const SurfelCloud& other ) = delete;
	SurfelCloud& operator=( SurfelCloud&& other );
	
	void render();
	uint numPoints() const { return m_numPts; }
	const Matrix4f& model() const { return m_model; }
	
	friend ostream& operator<<( ostream& out, const SurfelCloud& cloud );
	
private:
	void clean();
	void shallowClean();
	
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
: m_vao( 0 )
{
	if( !surfels.empty() )
	{
		m_model = model;
		m_numPts = static_cast< uint >( surfels.size() );
		
		GpuAllocStatistics::notifyAlloc( m_numPts * GpuAllocStatistics::pointSize() );
		
		glGenBuffers(1, &m_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Surfel) * m_numPts, surfels.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		#ifndef NDEBUG
			util::OglUtils::checkOglErrors();
		#endif
	}
	else
	{
		m_vbo = 0;
		m_numPts = 0;
		m_model = Eigen::Matrix4f::Identity();
	}
}

inline SurfelCloud::SurfelCloud( SurfelCloud&& other )
: m_vbo( other.m_vbo ),
m_vao( other.m_vao ),
m_numPts( other.m_numPts ),
m_model( other.m_model )
{
	other.shallowClean();
}

inline SurfelCloud& SurfelCloud::operator=( SurfelCloud&& other )
{
	clean();
	
	m_vbo = other.m_vbo;
	m_vao = other.m_vao;
	m_numPts = other.m_numPts;
	m_model = other.m_model;
	
	other.shallowClean();
	
	return *this;
}

inline SurfelCloud::~SurfelCloud()
{
	clean();
}

inline void SurfelCloud::render()
{
	if( !m_vao )
	{
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
	}
	else
	{
		glBindVertexArray( m_vao );
	}
	
	#ifndef DEBUG
		cout << "Rendering: " << endl << *this << endl << endl;
	#endif
	
	glDrawArrays( GL_POINTS, 0, m_numPts );
	glBindVertexArray( 0 );
	
	#ifndef NDEBUG
		util::OglUtils::checkOglErrors();
	#endif
}

inline void SurfelCloud::clean()
{
	if( m_vbo )
	{
		#ifdef DEBUG
		{
			cout << "Cleaning vbo" << endl << *this << endl << endl;
		}
		#endif
		
		glDeleteBuffers( 1, &m_vbo );
		GpuAllocStatistics::notifyDealloc( m_numPts * GpuAllocStatistics::pointSize() );
	}
	
	if( m_vao )
	{
		#ifdef DEBUG
		{
			cout << "Cleaning vao" << endl << *this << endl << endl;
		}
		#endif
		
		glDeleteVertexArrays( 1, &m_vao );
	}
}

inline void SurfelCloud::shallowClean()
{
	m_vbo = 0;
	m_vao = 0;
	m_numPts = 0;
	m_model = Matrix4f::Identity();
}

inline ostream& operator<<( ostream& out, const SurfelCloud& cloud )
{
	out << "Address: " << &cloud << ". vao: " << cloud.m_vao << " vbo: " << cloud.m_vbo << " nPoints: "
		<< cloud.m_numPts << " model matrix: " << endl << cloud.m_model;
	return out;
}

#undef DEBUG

#endif