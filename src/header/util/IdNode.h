#ifndef IDNODE_H
#define IDNODE_H

#include <utility>
#include "OctreeNode.h"
#include "MortonCode.h"

using namespace std;

namespace model
{
	template< typename MortonCode, typename OctreeNode >
	using IdNode = pair< MortonCode, OctreeNode >;
	
	template< typename MortonCode, typename OctreeNode >
	using IdNodeVector = vector< IdNode< MortonCode, OctreeNode >, ManagedAllocator< IdNode< MortonCode, OctreeNode > > >;
	
	template< typename MortonCode, typename OctreeNode >
	using ManagedIdNode = pair< shared_ptr< MortonCode >, shared_ptr< OctreeNode > >;
	
	template< typename MortonCode, typename OctreeNode >
	using ManagedIdNodeVector = vector< ManagedIdNode< MortonCode, OctreeNode >,
										ManagedAllocator< ManagedIdNode< MortonCode, OctreeNode > > >;
	
	template< typename MortonCode, typename OctreeNode >
	ostream& operator<<( ostream& out, const ManagedIdNode< MortonCode, OctreeNode >& idNode )
	{
		out << idNode.first->getPathToRoot( true );
		return out;
	}
	
	template< typename MortonCode, typename OctreeNode >
	ostream& operator<<( ostream& out, const IdNode< MortonCode, OctreeNode >& idNode )
	{
		out << idNode.first->getPathToRoot( true );
		return out;
	}
}

#endif