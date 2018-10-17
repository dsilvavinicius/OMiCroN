#ifndef SURFEL_CLOUD_H
#define SURFEL_CLOUD_H

#include <future>
#include "omicron/renderer/splat_renderer/surfel.hpp"
#include "omicron/basic/array.h"
#include "omicron/hierarchy/gpu_alloc_statistics.h"
#include "omicron/renderer/ogl_utils.h"
#include "omicron/util/stack_trace.h"
#include "omicron/hierarchy/hierarchy_creation_log.h"
#include "omicron/memory/global_malloc.h"
#include "omicron/hierarchy/external_octree_data.h"

// #define DEBUG
// #define CTOR_DEBUG
// #define CLEANING_DEBUG
// #define RENDERING_DEBUG
// #define GL_ERROR_DEBUG
// #define COMPARISON_DEBUG

using namespace omicron::hierarchy;

/** Surfel cloud that supports async loading. */
class SurfelCloud
{
    
public:
	enum LoadStatus
	{
		UNLOADED,
		LOADING,
		LOADED,
	};
	
	void* operator new( size_t size );
	void operator delete( void* p );
	
	/** Ctor which maps a VBO GPU memory for the cloud and issue a async loading operation. The loading operation status
	 * can be evaluated with loadStatus(). */
	SurfelCloud( const ulong offset, const ulong size, const int hierarchyLvl );
	
	SurfelCloud( const SurfelCloud& other ) = delete;
	SurfelCloud( SurfelCloud&& other ) = delete;
	
	~SurfelCloud();
	
	SurfelCloud& operator=( const SurfelCloud& other ) = delete;
	SurfelCloud& operator=( SurfelCloud&& other ) = delete;
	
	bool operator==( const SurfelCloud& other ) const;
	bool operator!=( const SurfelCloud& other ) const;
	
	LoadStatus loadStatus();
	
	/** Prepares the rendering, unmapping the VBO case needed. Must be called before the  */
	bool prepareRendering();
	
	/** Renders this SurfelCloud. Must be called after */
	void render();
	
	uint numPoints() const { return m_numPts; }
	
	friend ostream& operator<<( ostream& out, const SurfelCloud& cloud );

private:
	void unmap();
	
	void clean();
	
	Surfel* m_bufferMap;
	
	future< void >* m_loadFuture;
	
	GLuint m_vbo, m_vao;
    uint m_numPts;
};

inline void* SurfelCloud::operator new( size_t size )
{
	return TbbAllocator< SurfelCloud >().allocate( 1 );
}

inline void SurfelCloud::operator delete( void* p )
{
	TbbAllocator< SurfelCloud >().deallocate( static_cast< SurfelCloud* >( p ) );
}

inline SurfelCloud::SurfelCloud( const ulong offset, const ulong size, const int hierarchyLvl )
: m_numPts( size )
{
	assert( size > 0 && "SurfelCloud size is expected to be greater than 0." );
	
	GpuAllocStatistics::notifyAlloc( m_numPts * GpuAllocStatistics::pointSize() );
	
	glGenVertexArrays( 1, &m_vao );
	glBindVertexArray( m_vao );

	glGenBuffers( 1, &m_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
	
	glBufferData( GL_ARRAY_BUFFER, sizeof(Surfel) * m_numPts, NULL, GL_STATIC_DRAW );
	m_bufferMap = ( Surfel* ) glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );
	
	// Center c.
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE,
		sizeof( Surfel ), reinterpret_cast< const GLfloat* >( 0 ) );

	// Tagent vector u.
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE,
		sizeof( Surfel ), reinterpret_cast< const GLfloat* >( 12 ) );

	// Tangent vector v.
	glEnableVertexAttribArray( 2 );
	glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE,
		sizeof( Surfel ), reinterpret_cast< const GLfloat* >( 24 ) );
	
	glBindVertexArray( 0 );
	
	#if defined GL_ERROR_DEBUG || !defined NDEBUG
		omicron::renderer::OglUtils::checkOglErrors();
	#endif
	
	m_loadFuture = new future< void >(
		async( launch::async,
			[ = ]
			{
				auto bufferIter = m_bufferMap;
				ExtOctreeData::copyFromExternal( bufferIter, offset, hierarchyLvl, m_numPts );
			}
		)
	);
		
	#ifdef CTOR_DEBUG
	{
		stringstream ss; ss << "Ctor: " << *this << endl << endl;
		HierarchyCreationLog::logDebugMsg( ss.str() );
	}
	#endif
}

inline SurfelCloud::~SurfelCloud()
{
	#ifdef DEBUG
	{
		cout << "Dtor:" << endl << *this << endl << endl;
	}
	#endif
	
	clean();
}

inline bool SurfelCloud::operator==( const SurfelCloud& other ) const
{
	#ifdef COMPARISON_DEBUG
	{
		stringstream ss; ss << "Comparing vbo: " << m_vbo << " with vbo: " << other.m_vbo << endl << endl;
		HierarchyCreationLog::logDebugMsg( ss.str() );
	}
	#endif
	
	return m_vbo == other.m_vbo;
}

inline bool SurfelCloud::operator!=( const SurfelCloud& other ) const
{
	return !( *this == other );
}

inline void SurfelCloud::render()
{
	#ifdef RENDERING_DEBUG
	{
		stringstream ss; ss << "Rendering" << endl << *this << endl << endl;
		HierarchyCreationLog::logDebugMsg( ss.str() );
		HierarchyCreationLog::flush();
	}
	#endif
	
	glBindVertexArray( m_vao );
	glDrawArrays( GL_POINTS, 0, m_numPts );
	glBindVertexArray( 0 );
	
	#if defined GL_ERROR_DEBUG || !defined NDEBUG
		omicron::renderer::OglUtils::checkOglErrors();
	#endif
}

inline SurfelCloud::LoadStatus SurfelCloud::loadStatus()
{
	if( m_loadFuture->valid() )
	{
		// get() was not called yet.
		future_status status = m_loadFuture->wait_for( chrono::seconds( 0 ) );
		
		if( status == future_status::timeout )
		{
			// Loading still processing.
			return LOADING;
		}
		else
		{
			// Loading finished. Call get() to release shared resources, unmap VBO and report loaded successfully.
			m_loadFuture->get();
			unmap();
			return LOADED;
		}
	}
	else
	{
		// get() was already called. Loading was done.
		return LOADED;
	}
}

inline void SurfelCloud::unmap()
{
	glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
	glUnmapBuffer( GL_ARRAY_BUFFER );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
		
	#if defined GL_ERROR_DEBUG || !defined NDEBUG
		omicron::renderer::OglUtils::checkOglErrors();
	#endif
		
	m_bufferMap = nullptr;
}

inline void SurfelCloud::clean()
{
	if( m_loadFuture->valid() )
	{
		m_loadFuture->get();
		unmap();
	}
	delete m_loadFuture;
	m_loadFuture = nullptr;
	
	if( m_vbo )
	{
		#ifdef CLEANING_DEBUG
		{
			stringstream ss; ss << "Cleaning: " << endl << *this << endl << endl;
			HierarchyCreationLog::logDebugMsg( ss.str() );
		}
		#endif
		
		GpuAllocStatistics::notifyDealloc( m_numPts * GpuAllocStatistics::pointSize() );
		
		glDeleteBuffers( 1, &m_vbo );
		glDeleteVertexArrays( 1, &m_vao );
	}
}

inline ostream& operator<<( ostream& out, const SurfelCloud& cloud )
{
	out << "Address: " << &cloud << ". vao: " << cloud.m_vao << " vbo: " << cloud.m_vbo << " nPoints: "
		<< cloud.m_numPts;
	return out;
}

#undef DEBUG
#undef CTOR_DEBUG
#undef CLEANING_DEBUG
#undef RENDERING_DEBUG
#undef GL_ERROR_DEBUG
#undef COMPARISON_DEBUG

#endif
