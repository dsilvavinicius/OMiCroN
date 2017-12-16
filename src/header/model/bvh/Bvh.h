#ifndef BVH_H
#define BVH_H

#include "BasicTypes.h"
#include "Point.h"
#include "PlyPointReader.h"

namespace model
{
	/** Axis-aligned bounding box. */
	class Aabb
	{
	public:
		struct Boundaries
		{
			Boundaries( const Vec3& origin, const Vec3& extension )
			: m_origin( origin ),
			m_extension( extension )
			{}
			
			Vec3 m_origin;
			Vec3 m_extension;
			
			friend ostream& operator<<( ostream& out, const Boundaries& boundaries )
			{
				out << "Origin: " << endl << boundaries.m_origin << endl << "Extension: " << endl << boundaries.m_extension;
				return out;
			}
		};
		
		using AabbPtr = shared_ptr< Aabb >;
		
		Aabb( const Vec3& p );
		
		/** Indicates if the node is leaf. */
		virtual bool isLeaf() const = 0;
		
		/** Recursively and hierarchically inserts a point into this Aabb. Changes origin and extension if needed. */
		virtual void traverseAndInsert( const Point& point ) = 0;
		
		/** Shrinks used memory to fit inserted points and children. */
		virtual void shrink() = 0;
		
		/** Inserts a point into the Aabb. Changes origin and extension if needed. */
		virtual void insert( const Point& point );
		
		/** Inserts a point into the Aabb. Changes origin and extension if needed. */
		void insert( const Vec3& point );
		
		/** Calculates the surface area for SAH. */
		float sahSurfaceArea( const Boundaries& boundaries ) const;
		
		/** Min point of this Aabb. */
		Vec3 origin() const { return m_boundaries.m_origin; }
		
		/** Calculates the maximum position this Aabb occupies. */
		Vec3 maxPoint() const;
		
		/** Calculates the center of this Aabb. */
		Vec3 center() const;
		
		const Boundaries& boundaries() const { return m_boundaries; }
		
	protected:
		/** Calculates the new boundaries of this Aabb when a point is inserted. */
		Boundaries calcNewBoundaries ( const Vec3& point ) const;
		
		Boundaries m_boundaries;
	};
	
	/** Inner Aabb. Contains left and right children. */
	class InnerAabb
	: public Aabb
	{
	public:
		using ChildrenVector = vector< AabbPtr >;
		
		/** Ctor. Receives both children and set the boundaries up. */
		InnerAabb( AabbPtr leftChild, AabbPtr rightChild );
		
		/** Hierarchical point insertion. Traverses the current node, fixing this Aabb's boundaries and recursively inserting the point in the best child, according to the SAH. */
		void traverseAndInsert( const Point& point ) override;
		
		void shrink() override;
		
		bool isLeaf() const override { return false; }
		
		const ChildrenVector& children() const { return m_children; }
		
	private:
		ChildrenVector m_children;
	};
	
	/** Leaf Aabb. Contains the actual model geometry. */
	class LeafAabb
	: public Aabb
	{
	public:
		LeafAabb( const Point& point );
		
		void insert( const Point& point ) override;
		
		void traverseAndInsert( const Point& point ) override;
		
		void shrink() override { m_points.shrink_to_fit(); }
		
		bool isLeaf() const override { true; }
		
		const vector< Point >& points() const { return m_points; }
		
	private:
		vector< Point > m_points;
	};
	
	/** Incremental bounding volume hierarchy for points. Insertion is performed using the Surface Area Heuristic (SAH). */
	class Bvh
	{
	public:
		struct Statistics
		{
			Statistics( const Aabb::Boundaries& boundaries )
			: m_boundaries( boundaries ),
			m_nNodes( 0ul ),
			m_nPoints( 0ul ),
			m_nLeaves( 0ul ),
			m_avgPointsPerLeaf( 0ul ),
			m_maxPointsPerLeaf( 0ul ),
			m_minPointsPerLeaf( 0ul ),
			m_recursionCount( 0ul ),
			m_maxDepth( 0ul )
			{}
			
			ulong m_nNodes;
			ulong m_nPoints;
			ulong m_nLeaves;
			ulong m_avgPointsPerLeaf;
			ulong m_maxPointsPerLeaf;
			ulong m_minPointsPerLeaf;
			ulong m_recursionCount;
			ulong m_maxDepth;
			
			Aabb::Boundaries m_boundaries;
		};
		
		/** @param plyFilename the path for a .ply file with the input. */
		Bvh( const string& plyFilename );
		
		const Aabb& root() const { return *m_root; }
		
		Statistics statistics() const;
		
	private:
		/** Inserts the point into the Bvh, selecting the insertion path in the hierarchy using SAH. Internal nodes' Aabbs are fixed and geometry is inserted at leaves. New nodes can be created in the process. */
		void insert( const Point& point );
		
		void statistics( const Aabb& aabb, Statistics& stats ) const;
		
		typename Aabb::AabbPtr m_root;
	};
	
	// ==== Aabb implementation ====
	
	Aabb::Aabb( const Vec3& p )
	: m_boundaries( p, Vec3( 0.f, 0.f, 0.f ) )
	{}
	
	void Aabb::insert( const Point& point )
	{
		insert( point.getPos() );
	}
	
	void Aabb::insert( const Vec3& point )
	{
		m_boundaries = calcNewBoundaries( point );
	}
	
	Aabb::Boundaries Aabb::calcNewBoundaries( const Vec3& point ) const
	{
		Vec3 newOrigin;
		Vec3 newExtension;
		
		Vec3 prevMax = maxPoint();
		
		for( int i = 0; i < 3; ++i )
		{
			newOrigin[ i ] = min( m_boundaries.m_origin[ i ], point[ i ] );
			float newMaxCoord = max( prevMax[ i ], point[ i ] );
			newExtension[ i ] = newMaxCoord - newOrigin[ i ];
		}
		
		return Boundaries( newOrigin, newExtension ); 
	}
	
	float Aabb::sahSurfaceArea( const Boundaries& boundaries ) const
	{
		const Vec3& extension = boundaries.m_extension;
		
		return extension.x() * ( extension.y() + extension.z() ) + extension.y() * extension.z();
	}
	
	Vec3 Aabb::maxPoint() const
	{
		return m_boundaries.m_origin + m_boundaries.m_extension;
	}
	
	Vec3 Aabb::center() const
	{
		return m_boundaries.m_origin + m_boundaries.m_extension * 0.5f;
	}

	
	// ==== InnerAabb implementation ====
	
	InnerAabb::InnerAabb( Aabb::AabbPtr leftChild, Aabb::AabbPtr rightChild )
	: Aabb::Aabb( leftChild->boundaries().m_origin )
	{
		m_children.push_back( leftChild );
		m_children.push_back( rightChild );
		
		// Set boundaries.
		insert( m_children[ 0 ]->maxPoint() );
		insert( m_children[ 1 ]->boundaries().m_origin );
		insert( m_children[ 1 ]->maxPoint() );
	}

	void InnerAabb::traverseAndInsert( const Point& point )
	{
		// Case 1: The object and a leaf node are paired under a new node
		float cost1 = std::numeric_limits< float >::max();
		AabbPtr bestCase1NewChild = nullptr;
		typename ChildrenVector::iterator itToPairedLeaf;
		
		AabbPtr newLeaf( new LeafAabb( point ) );
		
		{
			for( typename ChildrenVector::iterator child = m_children.begin(); child != m_children.end(); child++ )
			{
				if( ( *child )->isLeaf() )
				{
					// Create a new inner node to pair the new leaf and the child node in order to simumlate case 1.
					AabbPtr newNode( new InnerAabb( *child, newLeaf ) );
					float cost = 2 * newNode->sahSurfaceArea( newNode->boundaries() );
					
					if( cost < cost1 )
					{
						cost1 = cost;
						bestCase1NewChild = newNode;
						itToPairedLeaf = child;
					}
				}
			}
		}
		
		// Case 2: The object is added as a new child to an existing node
		float oldArea = sahSurfaceArea( m_boundaries );
		float newArea = sahSurfaceArea( calcNewBoundaries( point.getPos() ) );
		
		float cost2 = m_children.size() * ( newArea - oldArea ) + newArea;
		
		float epsilon = 1.0e-8;
		
		// Fix bounding box.
		insert( point );
		
		if( abs( cost1 - cost2 ) < epsilon )
		{
			// Debug 
// 			{
// 				cout << "t" << endl;
// 			}
			
			// Cost tie. Insertion is done into the child with nearest center.
			AabbPtr closerChild;
			float inf = std::numeric_limits<float>::max();
			Vec3 minDistance( inf, inf, inf );
			
			for( AabbPtr child : m_children )
			{
				Vec3 distance = point.getPos() - child->center();
				if( distance.squaredNorm() < minDistance.squaredNorm() )
				{
					minDistance = distance;
					AabbPtr closerChild = child;
				}
			}
			closerChild->traverseAndInsert( point );
		}
		else if( cost1 < cost2 )
		{
			// Debug 
// 			{
// 				cout << "c1" << endl;
// 			}
			
			// Case 1 is better
			*itToPairedLeaf = bestCase1NewChild;
		}
		else
		{
			// Debug 
// 			{
// 				cout << "c2" << endl;
// 			}
			
			// Case 2 is better
			m_children.push_back( newLeaf );
		}
	}

	void InnerAabb::shrink()
	{
		m_children.shrink_to_fit();
		
		for( AabbPtr child : m_children )
		{
			child->shrink();
		}
	}

	
	// ==== LeafAabb implementation ====
	LeafAabb::LeafAabb( const Point& point )
	: Aabb::Aabb( point.getPos() )
	{
		m_points.push_back( point );
	}
	
	void LeafAabb::insert( const Point& point )
	{
		Aabb::insert(point);
		m_points.push_back( point );
	}

	void LeafAabb::traverseAndInsert( const Point& point )
	{
		insert( point );
	}

	// ==== Bvh implementation ====
	
	Bvh::Bvh( const string& plyFilename )
	: m_root( nullptr )
	{
		using namespace util;
		
		PlyPointReader reader( plyFilename );
		reader.read(
			[ & ]( const Point& p )
			{
				insert( p );
			}
		);
		
		m_root->shrink();
	}
	
	void Bvh::insert( const Point& point )
	{
		if( m_root == nullptr )
		{
			m_root = Aabb::AabbPtr( new LeafAabb( point ) );
		}
		else if( m_root->isLeaf() )
		{
			Aabb::AabbPtr newLeaf( new LeafAabb( point ) );
			m_root = Aabb::AabbPtr( new InnerAabb( m_root, newLeaf ) );
		}
		else
		{
			m_root->traverseAndInsert( point );
		}
	}
	
	Bvh::Statistics Bvh::statistics() const
	{
		Statistics stats( m_root->boundaries() );
		stats.m_maxPointsPerLeaf = numeric_limits< ulong >::min();
		stats.m_minPointsPerLeaf = numeric_limits< ulong >::max();
		
		statistics( *m_root, stats );
		
		stats.m_avgPointsPerLeaf = stats.m_nPoints / stats.m_nLeaves;
		
		return stats;
	}

	void Bvh::statistics( const Aabb& aabb, Statistics& stats ) const
	{
		++stats.m_nNodes;
		++stats.m_recursionCount;
		
		if( stats.m_recursionCount > stats.m_maxDepth )
		{
			stats.m_maxDepth = stats.m_recursionCount;
		}
		
		if( aabb.isLeaf() )
		{
			auto leafAabb = dynamic_cast< const LeafAabb* >( &aabb );
			ulong pointsInLeaf = leafAabb->points().size();
			stats.m_nPoints += pointsInLeaf;
			++stats.m_nLeaves;
		
			if( pointsInLeaf > stats.m_maxPointsPerLeaf )
			{
				stats.m_maxPointsPerLeaf = pointsInLeaf;
			}
			if( pointsInLeaf < stats.m_minPointsPerLeaf )
			{
				stats.m_minPointsPerLeaf = pointsInLeaf;
			}
		}
		else
		{
			auto innerAabb = dynamic_cast< const InnerAabb* >( &aabb );
			for( const Aabb::AabbPtr child : innerAabb->children() )
			{
				statistics( *child, stats );
			}
		}
		
		--stats.m_recursionCount;
	}
}

#endif