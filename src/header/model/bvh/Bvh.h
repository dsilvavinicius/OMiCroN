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
		using ChildrenVector = vector< AabbPtr >;
		
		Aabb( const Vec3& p );
		
		/** Indicates if the node is leaf. */
		virtual bool isLeaf() const = 0;
		
		/** Recursively and hierarchically inserts a point into this Aabb. Changes origin and extension if needed. */
		virtual void traverseAndInsert( const Point& point, int level, int maxLevel ) = 0;
		
		/** Shrinks used memory to fit inserted points and children. */
		virtual void shrink() = 0;
		
		/** Inserts a point into the Aabb. Changes origin and extension if needed. */
		virtual void insert( const Point& point );
		
		virtual const ChildrenVector& children() const = 0;
		
		virtual ChildrenVector& children() = 0;
		
		/** Inserts a point into the Aabb. Changes origin and extension if needed. */
		void insert( const Vec3& point );
		
		/** Calculates the surface area for SAH. */
		static float sahSurfaceArea( const Boundaries& boundaries );
		
		/** Calculates the new boundaries of this Aabb when a point is inserted. */
		Boundaries calcNewBoundaries ( const Vec3& point ) const;
		
		/** Min point of this Aabb. */
		Vec3 origin() const { return m_boundaries.m_origin; }
		
		Vec3 extension() const { return m_boundaries.m_extension; }
		
		/** Calculates the maximum position this Aabb occupies. */
		Vec3 maxPoint() const;
		
		/** Calculates the center of this Aabb. */
		Vec3 center() const;
		
		const Boundaries& boundaries() const { return m_boundaries; }
		
		friend ostream& operator<<( ostream& out, const Aabb& aabb );
	
	protected:
		
		Boundaries m_boundaries;
	};
	
	/** Inner Aabb. Contains left and right children. */
	class InnerAabb
	: public Aabb
	{
	public:
		/** Ctor. Receives both children and set the boundaries up. */
		InnerAabb( AabbPtr leftChild, AabbPtr rightChild );
		
		/** Hierarchical point insertion. Traverses the current node, fixing this Aabb's boundaries and recursively inserting the point in the best child, according to the SAH. */
		void traverseAndInsert( const Point& point, int level, int maxLevel ) override;
		
		void shrink() override;
		
		bool isLeaf() const override { return false; }
		
		const ChildrenVector& children() const override { return m_children; }
		
		ChildrenVector& children() override { return m_children; }
		
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
		
		void traverseAndInsert( const Point& point, int level, int maxLevel ) override;
		
		void shrink() override { m_points.shrink_to_fit(); }
		
		bool isLeaf() const override { return true; }
		
		const ChildrenVector& children() const override { throw logic_error( "LeafAabb does not have child." ); }
		
		ChildrenVector& children() override { throw logic_error( "LeafAabb does not have child." ); }
		
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
			
			friend ostream& operator<<( ostream& out, const Statistics& stats );
		};
		
		/** Ctor for an empty Bvh. Points can be inserted using insert(). */
		Bvh( int maxLevel );
		
		/** @param plyFilename the path for a .ply file with the input. */
		Bvh( const string& plyFilename, int maxLevel );
		
		const Aabb& root() const { return *m_root; }
		
		/** Inserts the point into the Bvh, selecting the insertion path in the hierarchy using SAH. Internal nodes' Aabbs are fixed and geometry is inserted at leaves. New nodes can be created in the process. */
		void insert( const Point& point );
		
		Statistics statistics() const;
		
		void shrink();
		
		/** Sanity test. Throws exceptions when insane.*/
		void isSane( bool print = false ) const;
		
	private:
		void isSane( const Aabb& aabb, int level, bool print ) const;
		
		void statistics( const Aabb& aabb, Statistics& stats ) const;
		
		typename Aabb::AabbPtr m_root;
		int m_maxLevel;
	};
	
	// ==== Aabb implementation ====
	
	Aabb::Aabb( const Vec3& p )
	: m_boundaries( p, Vec3( 0.f, 0.f, 0.f ) )
	{}
	
	void Aabb::insert( const Point& point )
	{
		// Debug
// 		{
// 			cout << "Before: " << *this << endl << endl;
// 		}
		//
		
		insert( point.getPos() );
		
		// Debug
// 		{
// 			cout << "After: " << *this << endl << endl;
// 		}
		//
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
	
	float Aabb::sahSurfaceArea( const Boundaries& boundaries )
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

	ostream& operator<<( ostream& out, const Aabb& aabb )
	{
		out << "Addr: " << &aabb << endl << "Is leaf: " << ( ( aabb.isLeaf() ) ? "true" : "false" ) << endl << aabb.m_boundaries;
		return out;
	}
	
	// ==== InnerAabb implementation ====
	
	InnerAabb::InnerAabb( Aabb::AabbPtr leftChild, Aabb::AabbPtr rightChild )
	: Aabb::Aabb( leftChild->boundaries().m_origin )
	{
		m_children.push_back( leftChild );
		m_children.push_back( rightChild );
		
		// Set boundaries.
		insert( m_children[ 0 ]->origin() );
		insert( m_children[ 0 ]->maxPoint() );
		insert( m_children[ 1 ]->origin() );
		insert( m_children[ 1 ]->maxPoint() );
	}

	void InnerAabb::traverseAndInsert( const Point& point, int level, int maxLevel )
	{
		// Case 1: The object and a leaf node are paired under a new node.
		// Case 2: The object is added as a new child to an existing node.
		// Case 3: The object is added lower in the tree by means of recursing into a child volume.
		float epsilon = 1.e-10;
		
		// Case 1 cost.
		float cost1 = std::numeric_limits< float >::max();
		AabbPtr bestCase1NewChild = nullptr;
		typename ChildrenVector::iterator itToPairedLeaf = m_children.end();
		
		AabbPtr bestCase3Inner = nullptr;
		
		bool hasInnerChild = false;
		
		// Debug
// 		{
// 			cout << "Traversing:" << endl << *this << endl << endl;
// 		}
		
		AabbPtr newLeaf( new LeafAabb( point ) );
		
		{
			float bestCase3InnerCost = std::numeric_limits< float >::max();
			
			for( typename ChildrenVector::iterator child = m_children.begin(); child != m_children.end(); child++ )
			{
				if( ( *child )->isLeaf() )
				{
					// Select the best leaf node to pair with a new leaf node in case 1.
					AabbPtr newNode( new InnerAabb( *child, newLeaf ) );
					float cost = 2 * newNode->sahSurfaceArea( newNode->boundaries() );
					
					bool currentIsBetterInTie = false;
					
					if( abs( cost - cost1 ) < epsilon )
					{
						// Cost tie. Choose the Aabb with nearest center.
						Vec3 distanceToCurrent = point.getPos() - newNode->center();
						Vec3 distanceToBest = point.getPos() - bestCase1NewChild->center();
						
						if( distanceToCurrent.squaredNorm() < distanceToBest.squaredNorm() )
						{
							cost1 = cost;
							bestCase1NewChild = newNode;
							itToPairedLeaf = child;
						}
					}
					else if( cost < cost1 )
					{
						cost1 = cost;
						bestCase1NewChild = newNode;
						itToPairedLeaf = child;
					}
				}
				else
				{
					// Select the best inner node to recurse in case 3.
					hasInnerChild = true;
					
					float childOldArea = sahSurfaceArea( ( *child )->boundaries() );
					float childNewArea = sahSurfaceArea( ( *child )->calcNewBoundaries( point.getPos() ) );
					float cost = ( ( ( *child )->isLeaf() ) ? 0 : ( *child )->children().size() ) * ( childNewArea - childOldArea );
					
					if( abs( cost - bestCase3InnerCost ) < epsilon )
					{
						// Cost tie. Choose the Aabb with nearest center.
						Vec3 distanceToCurrent = point.getPos() - ( *child )->center();
						Vec3 distanceToBest = point.getPos() - bestCase3Inner->center();
						
						if( distanceToCurrent.squaredNorm() < distanceToBest.squaredNorm() )
						{
							bestCase3InnerCost = cost;
							bestCase3Inner = *child;
						}
					}
					else if( cost < bestCase3InnerCost )
					{
						bestCase3InnerCost = cost;
						bestCase3Inner = *child;
					}
				}
			}
			
			// Debug
// 			{
// 				if( itToPairedLeaf != m_children.end() )
// 				{
// 					cout << "Best leaf:" << endl << **itToPairedLeaf << endl << endl
// 						 << "New paired inner:" << endl << *bestCase1NewChild << endl << endl;
// 				}
// 				else
// 				{
// 					cout << "No leaves." << endl << endl;
// 				}
// 				
// 				if( bestCase3Inner != nullptr )
// 				{
// 					cout << "Best inner:" << endl << *bestCase3Inner << endl << endl;
// 				}
// 				else
// 				{
// 					cout << "No inners." << endl << endl;
// 				}
// 			}
			//
		}
		
		// Case 2 cost.
		float oldArea = sahSurfaceArea( m_boundaries );
		float newArea = sahSurfaceArea( calcNewBoundaries( point.getPos() ) );
		
		float cost2 = m_children.size() * ( newArea - oldArea ) + newArea;
		
		// Case 3 cost.
		float cost3 = ( hasInnerChild ) ? m_children.size() * ( newArea - oldArea ) : std::numeric_limits< float >::max(); 
		
		float bestCost = min( min( cost1, cost2 ), cost3 );
		
		// Debug
// 		{
// 			cout << "cost 1: " << cost1 << endl << "cost 2: " << cost2 << endl << "cost 3: " << cost3 << endl << endl;
// 		}
		//
		
		// Fix bounding box.
		insert( point );
		
		// Debug
// 		{
// 			cout << "Fixed: " << *this << endl << endl;
// 		}
		//
		
		if( abs( bestCost - cost1 ) < epsilon )
		{
			if( level == maxLevel - 1 )
			{
				// Debug 
// 				{
// 					cout << "Case: max level. Inserting at best leaf." << endl << endl;
// 				}
				
				// Cannot increase level, so creating a new inner node is not allowed. Inserting directly at leaf instead.
				( *itToPairedLeaf )->insert( point );
			}
			else
			{
				// Debug 
// 				{
// 					cout << "Case: paired with best child" << endl << endl;
// 				}
				
				// Case 1 is better
				*itToPairedLeaf = bestCase1NewChild;
			}
		}
		else if( abs( bestCost - cost2 ) < epsilon )
		{
			// Debug 
// 			{
// 				cout << "Case: added as new child" << endl << endl;
// 			}
			
			// Case 2 is better
			m_children.push_back( newLeaf );
		}
		else
		{
			// Debug 
// 			{
// 				cout << "Case: recursion" << endl << endl;
// 			}
			
			// Case 3 is better
			bestCase3Inner->traverseAndInsert( point, level + 1, maxLevel );
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

	void LeafAabb::traverseAndInsert( const Point& point, int level, int maxLevel )
	{
		insert( point );
	}

	// ==== Bvh implementation ====
	
	Bvh::Bvh( int maxLevel = std::numeric_limits< int >::max() )
	: m_root( nullptr ),
	m_maxLevel( maxLevel )
	{}
	
	Bvh::Bvh( const string& plyFilename, int maxLevel = std::numeric_limits< int >::max() )
	: m_root( nullptr ),
	m_maxLevel( maxLevel )
	{
		using namespace util;
		
		PlyPointReader reader( plyFilename );
		reader.read(
			[ & ]( const Point& p )
			{
				insert( p );
				
				// DEBUG
// 				{
// 					isSane();
// 				}
				//
			}
		);
		
		shrink();
	}
	
	void Bvh::insert( const Point& point )
	{
		// Debug
// 		{
// 			cout << "===== INSERTING =====" << endl << point << endl;
// 		}
		//
		
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
			m_root->traverseAndInsert( point, 1, m_maxLevel );
		}
	}
	
	void Bvh::shrink()
	{
		m_root->shrink();
	}
	
	void Bvh::isSane( bool print ) const
	{
		isSane( *m_root, 1, print );
	}
	
	void Bvh::isSane( const Aabb& aabb, int level, bool print ) const
	{
		if( print )
		{
			cout << "Level: " << level << "." << endl << aabb << endl << endl;
		}
		
		if( !aabb.isLeaf() )
		{
			using ChildrenVector = InnerAabb::ChildrenVector;
			
			const ChildrenVector& children = aabb.children();
			
			float sah = aabb.sahSurfaceArea( aabb.boundaries() );
			
			for( ChildrenVector::const_iterator it = children.begin(); it != children.end(); it++ )
			{
				const Aabb& child = ( **it );
				float childSah = child.sahSurfaceArea( child.boundaries() );
				
				
				{
					float epsilon = 1.e-5;
					// Checking surface area.
					if( childSah >= sah + epsilon )
					{
						stringstream ss; ss << "Child surface area is expected to be less than parent." << "Parents SA: " << sah << " Child SA: " << childSah << endl << endl
						<< "Parent:" << endl << aabb << endl << endl << "Child:" << endl << child << endl << endl; 
						throw runtime_error( ss.str() );
					}
					
					// Checking inclusion.
					for( int i = 0; i < 3; ++i )
					{
						if( child.origin()[ i ] < aabb.origin()[ i ] - epsilon || child.maxPoint()[ i ] > aabb.maxPoint()[ i ] + epsilon )
						{
							stringstream ss; ss << "Child is expected to be contained in parent." << endl << endl
							<< "Parent:" << endl << aabb << endl << endl << "Child:" << endl << child << endl << endl;
							throw runtime_error( ss.str() );
						}
					}
				}
				
// 				for( ChildrenVector::const_iterator it2 = std::next( it, 1 ); it2 != children.end(); it2++ )
// 				{
// 					float collisionPercentage = 0.9f;
// 					
// 					const Aabb& sibling = ( **it2 );
// 					
// 					// Checking no intersection between children.
// 					bool intersecting = true;
// 					
// 					// Debug
// // 					{
// // 						cout << "Intersection test: " << endl << child << endl << " XXX " << endl << sibling << endl << endl;
// // 					}
// 					//
// 					
// 					for( int i = 0; i < 3; ++i )
// 					{
// 						
// 						if( child.origin()[ i ] > sibling.maxPoint()[ i ] - ( sibling.extension()[ i ] * collisionPercentage )
// 							|| sibling.origin()[ i ] > child.maxPoint()[ i ] - ( child.extension()[ i ] * collisionPercentage ) )
// 						{
// 							// Debug
// // 							{
// // 								cout << "Not intersecting at " << i << endl << endl;
// // 							}
// 							//
// 							
// 							intersecting = false;
// 							break;
// 						}
// 					}
// 					
// 					// Test
// 					if( intersecting )
// 					{
// 						stringstream ss; ss << "Intersecting: " << endl << child << endl << "AND " << endl << sibling << endl << endl;
// 						throw runtime_error( ss.str() );
// 					}
// 				}
			}
			
			for( const Aabb::AabbPtr child : children )
			{
					isSane( *child, level + 1, print );
			}
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
	
	ostream& operator<<( ostream& out, const Bvh::Statistics& stats )
	{
		out << "Boundaries: " << endl << "origin: " << endl << stats.m_boundaries.m_origin << endl << "extension" << endl << stats.m_boundaries.m_extension << endl
			<< "Max depth: " << stats.m_maxDepth << endl
			<< "Number of nodes: " << stats.m_nNodes << endl
			<< "Number of points: " << stats.m_nPoints << endl
			<< "Number of leaves: " << stats.m_nLeaves << endl
			<< "Avg points per leaf: " << stats.m_avgPointsPerLeaf << endl
			<< "Min points in a leaf: " << stats.m_minPointsPerLeaf << endl
			<< "Max points in a leaf: " << stats.m_maxPointsPerLeaf << endl
			<< "Recursion count: " << stats.m_recursionCount;
			
		return out;
	}
}

#endif