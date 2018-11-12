#ifndef EXTERNAL_OCTREE_DATA_H
#define EXTERNAL_OCTREE_DATA_H

#include <stxxl/vector>
#include "omicron/renderer/splat_renderer/surfel.hpp"
#include "omicron/hierarchy/octree_dimensions.h"

namespace omicron::hierarchy
{
    /** All external datastructures of the octree nodes and methods to operate them. */
    class ExtOctreeData
    {
    public:
        using SurfelIter = Surfel*;
        using IndexVector = std::vector< ulong, TbbAllocator< ulong > >;

        static void pushSurfel( const Surfel& s );
        
        template< typename Morton >
		static Morton calcMorton( const ulong index, const OctreeDimensions< Morton >& octreeDim );
        static const Surfel& getSurfel( const ulong index );
        static const ulong getIndex( const ulong offset );

        static ulong reserveIndices( const uint nIndices );
        static void copy2External( const IndexVector& indices, const ulong pos );
        static void copyFromExternal( SurfelIter& iter, const ulong pos, const int hierarchyLvl, const uint size );
    
    private:
        #ifdef LAB
            static constexpr ulong EXT_SURFEL_MEM_BUDGET = 1ul * 1024ul * 1024ul * 1024ul;
            static constexpr ulong EXT_INDEX_MEM_BUDGET = 1ul * 1024ul * 1024ul * 1024ul;
        #else
            static constexpr ulong EXT_SURFEL_MEM_BUDGET = 6ul * 1024ul * 1024ul * 1024ul;
            static constexpr ulong EXT_INDEX_MEM_BUDGET = 6ul * 1024ul * 1024ul * 1024ul;
        #endif
        static constexpr ulong EXT_PAGE_SIZE = 4ul;
        static constexpr ulong EXT_BLOCK_SIZE = 2ul * 1024ul * 1024ul;

        using ExtSurfelVector = stxxl::VECTOR_GENERATOR< Surfel, EXT_PAGE_SIZE, EXT_SURFEL_MEM_BUDGET / ( EXT_BLOCK_SIZE * EXT_PAGE_SIZE ), EXT_BLOCK_SIZE >::result;
        using ExtIndexVector = stxxl::VECTOR_GENERATOR< ulong, EXT_PAGE_SIZE, EXT_INDEX_MEM_BUDGET / ( EXT_BLOCK_SIZE * EXT_PAGE_SIZE ), EXT_BLOCK_SIZE >::result;
        
        static ExtSurfelVector m_surfels;
		static ExtIndexVector m_indices;
        static mutex m_indexMutex;
        static mutex m_surfelMutex;
    };

    inline void ExtOctreeData::pushSurfel( const Surfel& s )
    {
        lock_guard< mutex > lock( m_surfelMutex );
        m_surfels.push_back( s );
    }

    template< typename Morton >
    inline Morton ExtOctreeData::calcMorton( const ulong index, const OctreeDimensions< Morton >& octreeDim )
    {
        Surfel s;
        {
            lock_guard< mutex > lock( m_surfelMutex );
            s = m_surfels[ index ];
        }
        return octreeDim.calcMorton( s );
    }

    inline const Surfel& ExtOctreeData::getSurfel( const ulong index )
    {
        lock_guard< mutex > lock( m_surfelMutex );
        return m_surfels[ index ];
    }
    
    inline const ulong ExtOctreeData::getIndex( const ulong offset )
    {
        lock_guard< mutex > lock( m_indexMutex );
        return m_indices[ offset ];
    }

    inline ulong ExtOctreeData::reserveIndices( const uint nIndices )
    {
        ulong sizeBefore;
        {
            lock_guard< mutex > lock( m_indexMutex );
            sizeBefore = m_indices.size();
            m_indices.resize( sizeBefore + nIndices );
        }
        return sizeBefore;
    }
    
    inline void ExtOctreeData::copy2External( const IndexVector& indices, ulong pos )
    {
        for( const ulong i : indices )
        {
            lock_guard< mutex > lock( m_indexMutex );
            
            assert( !( Vec3( 0.f, 0.f, 0.f ) == m_surfels[ i ].c && Vec3( 0.f, 0.f, 0.f ) == m_surfels[ i ].u && Vec3( 0.f, 0.f, 0.f ) == m_surfels[ i ].v ) );
            
            m_indices[ pos++ ] = i;
        }
    }

    inline void ExtOctreeData::copyFromExternal( SurfelIter& iter, const ulong pos, const int hierarchyLvl, const uint size )
    {
        Vector2f multipliers = ReconstructionParams::calcMultipliers( hierarchyLvl );

        for( ulong i = pos; i < pos + size; ++i )
        {
            ulong index;
            {
                lock_guard< mutex > lock( m_indexMutex );
                index = m_indices[ i ];
            }
            Surfel s;
            {
                lock_guard< mutex > lock( m_surfelMutex );
                s = m_surfels[ index ];
            }
            s.multiplyTangents( multipliers );
            *iter++ = s;
        }
    }
}

#endif