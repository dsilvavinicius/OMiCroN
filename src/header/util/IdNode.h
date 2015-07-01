#ifndef IDNODE_H
#define IDNODE_H

#include <utility>
#include "OctreeNode.h"

using namespace std;

namespace model
{
	template< typename MortonCode >
	using IdNode = pair< shared_ptr< MortonCode >, model::OctreeNodePtr< MortonCode > >;
	
	template< typename MortonCode >
	using IdNodeVector = vector< IdNode< MortonCode > >;
	
	template< typename MortonCode >
	ostream& operator<<( ostream& out, const IdNode< MortonCode >& idNode )
	{
		out << idNode.first->getPathToRoot( true );
		return out;
	}
}

#endif