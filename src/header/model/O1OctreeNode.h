#ifndef O1_OCTREE_NODE_H
#define O1_OCTREE_NODE_H

#include <memory>
#include "OctreeMapTypes.h"
#include "ConcurrentOctreeMapTypes.h"

using namespace std;

namespace model
{
	/** Base for octree node that has O(1) complexity for all operations.
	 * @param Contents is the content type of the node. */
	template< typename Contents >
	class O1OctreeNodeBase
	{
	public:
		Contents& getContents();
		void setContents( const Contents& contents );
	
	protected:
		Contents m_contents;
	};
	
	/** O1OctreeNode that uses an OctreeMap as hierarchy.
	 * @param Contents is the content type of the node.
	 * @param Morton is the morton code used in the OctreeMap. */
	template< typename Contents, typename Morton >
	class O1OctreeNode
	: O1OctreeNodeBase< Contents >
	{
		using Hierarchy = OctreeMap< Morton, O1OctreeNode >;
		using Accessor = typename Hierarchy::iterator;
	
	public:
		O1OctreeNode( const Hierarchy& hierarchy );
		
		Accessor getParent() const;
		Accessor getSibling( const Hierarchy& hierarchy ) const;
		Accessor getChild() const;
		
		/** Set parent, given its morton code and the hierarchy. */
		void setParent( const Morton& morton, const Hierarchy& hierarchy );
		
		/** Set child, given the Acessor for this OctreeNode inside hierarchy and a flag indicating
		 * if the Acessor should be advanced or not. */
		void setChild( const Hierarchy& hierarchy, const bool& advance );
		
		/** @returns true if there is no m_parent set, false otherwise */
		bool isRoot( const Hierarchy& hierarchy ) const;
		
		/** @returns true if there is no m_firstChild set, false otherwise */
		bool isLeaf( const Hierarchy& hierarchy ) const;
		
		template< typename C, typename M >
		friend ostream& operator<<( ostream& out, const O1OctreeNode< C, M >& node );
		
		size_t serialize( byte** serialization ) const;
		
		static O1OctreeNode deserialize( byte* serialization, const Accessor& parent, const Accessor& sibling );
		
	private:
		Accessor m_parent; // parent.
		Accessor m_child; // first child.
	};
	
	/** O1OctreeNode that uses an OctreeMap as hierarchy.
	 * @param Contents is the content type of the node.
	 * @param Morton is the morton code used in the OctreeMap. */
	template< typename Contents, typename Morton >
	class ParallelO1OctreeNode
	: O1OctreeNodeBase< Contents >
	{
		using Hierarchy = ConcurrentOctreeMap< Morton, ParallelO1OctreeNode >;
		using Accessor = typename Hierarchy::accessor;
	
	public:
		ParallelO1OctreeNode( const Hierarchy& hierarchy );
		
		Accessor getParent() const;
		Accessor getSibling() const;
		Accessor getChild() const;
		
		void setParent( const Accessor& parent );
		void setSibling( const Accessor& sibling );
		void setChild( const Accessor& child );
		
		/** @returns true if there is no m_parent set, false otherwise */
		bool isRoot( const Hierarchy& hierarchy ) const;
		
		/** @returns true if there is no m_firstChild set, false otherwise */
		bool isLeaf( const Hierarchy& hierarchy ) const;
		
		template< typename C, typename M >
		friend ostream& operator<<( ostream& out, const ParallelO1OctreeNode< C, M >& node );
		
		size_t serialize( byte** serialization ) const;
		
		static ParallelO1OctreeNode deserialize( byte* serialization, const Accessor& parent, const Accessor& sibling );
		
	private:
		Accessor m_parent; // parent.
		Accessor m_sibling; // next sibling.
		Accessor m_child; // first child.
	};
}

#endif