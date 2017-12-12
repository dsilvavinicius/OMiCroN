#ifndef BVH_H
#define BVH_H

#include "BasicTypes.h"
#include "Point.h"

namespace model
{
	/** Axis-aligned bounding box. */
	class Aabb
	{
	public:
		using AabbPtr = shared_ptr< Aabb >;
		
		Aabb();
		
		/** Indicates if the node is leaf. */
		virtual bool isLeaf() const = 0;
		
		/** Recursively and hierarchically inserts a point into this Aabb. Changes origin and extension if needed. */
		void traserveAndInsert( const Point& point ) = 0;
		
		/** Shrinks used memory to fit inserted points and children. */
		void shrink() = 0;
		
		/** Inserts a point into the Aabb. Changes origin and extension if needed. */
		virtual void insert( const Point& point );
		
		/** Inserts a point into the Aabb. Changes origin and extension if needed. */
		void insert( const Vec3& point );
		
		/** Calculates the surface area for SAH. */
		float sahSurfaceArea( const Boundaries& boundaries = m_boundaries ) const;
		
		/** Calculates the maximum position this Aabb occupies. */
		Vec3 maxPoint() const;
		
		/** Calculates the center of this Aabb. */
		Vec3 center() const;
		
	protected:
		struct Boundaries
		{
			Vec3 m_origin;
			Vec3 m_extension;
		};
		
		/** Calculates the new boundaries of this Aabb when a point is inserted. */
		Boundaries calcNewBoundaries ( const Point& point ) const;
		
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
		
		void shrink() override { m_children.shrink_to_fit(); }
		
		bool isLeaf() const override { return false; }
		
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
		
	private:
		vector< Point > m_points;
	};
	
	/** Incremental bounding volume hierarchy for points. Insertion is performed using the Surface Area Heuristic (SAH). */
	class Bvh
	{
	public:
		Bvh();
		
		/** Inserts the point into the Bvh, selecting the insertion path in the hierarchy using SAH. Internal nodes' Aabbs are fixed and geometry is inserted at leaves. New nodes can be created in the process. */
		void insert( const Point& point );
		
	private:
		typename Aabb::AabbPtr m_root;
	};
	
	// ==== Aabb implementation ====
	
	Aabb::Aabb()
	{
		m_boundaries.m_origin = Vec3( 0.f, 0.f, 0.f );
		m_boundaries.m_extension = Vec3( 0.f, 0.f, 0.f );
	}
	
	void Aabb::insert( const Point& point )
	{
		insert( point.getPos() );
	}
	
	void Aabb::insert( const Vec3& point )
	{
		m_boundaries = calcNewBoundaries( point );
	}
	
	Aabb::Boundaries Aabb::calcNewBoundaries( const Point& point ) const
	{
		Boundaries newBoundaries;
		
		for( int i = 0; i < 3; ++i )
		{
			newBoundaries.m_origin[ i ] = min( m_boundaries.m_origin[ i ], point[ i ] );
			newBoundaries.m_extension[ i ] = max( m_boundaries.m_extension[ i ], point[ i ] - m_boundaries.m_origin[ i ] );
		}
		
		return newBoundaries; 
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
	{
		m_children.push_back( leftChild );
		m_children.push_back( rightChild );
		
		// Set boundaries.
		insert( m_children[ 0 ]->m_boundaries.m_origin );
		insert( m_children[ 0 ]->maxPoint() );
		insert( m_children[ 1 ]->m_boundaries.m_origin );
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
					float cost = 2 * newNode->sahSurfaceArea();
					
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
		float oldArea = sahSurfaceArea();
		float newArea = sahSurfaceArea( calcNewBoundaries( point ) );
		
		float cost2 = m_children.size() * ( newArea - oldArea ) + newArea;
		
		float epsilon = 1.0e-8;
		
		// Fix bounding box.
		insert( point );
		
		if( abs( cost1 - cost2 ) < epsilon )
		{
			// Cost tie. Insertion is done into the child with nearest center.
			AabbPtr closerChild;
			float inf = std::numeric_limits<float>::max();
			Vec3 minDistance( inf, inf, inf );
			
			for( AabbPtr child : m_children )
			{
				Vec3 distance = point.getPos() - child()->center();
				if( distance.squaredNorm() < minDistance.squaredNorm() )
				{
					minDistance = distace;
					AabbPtr closerChild = child;
				}
			}
			closerChild->traverseAndInsert( point );
		}
		else if( cost1 < cost2 )
		{
			// Case 1 is better
			*itToPairedLeaf = bestCase1NewChild;
		}
		else
		{
			// Case 2 is better
			m_children.push_back( newLeaf );
		}
	}

	
	// ==== LeafAabb implementation ====
	LeafAabb::LeafAabb( const Point& point )
	{
		insert( point );
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
	
	Bvh::Bvh()
	: m_root( nullptr )
	{}

	
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
			m_root->traserveAndInsert( point );
		}
	}

}

#endif