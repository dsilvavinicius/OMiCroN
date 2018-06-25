#ifndef NODE_RELEASER
#define NODE_RELEASER

#include <iostream>
#include <vector>
#include <memory>
#include "ManagedAllocator.h"

using namespace std;

namespace omicron
{
	template< typename Contents >
	class OctreeNode;
	
	/** Methods to release node contents. */
	namespace NodeReleaser
	{
		/** Default implementation assumes that no additional efforts should be done in order to release contents. */
		template< typename Contents >
		void release( OctreeNode< Contents >& node )
		{}
		
		template< typename T >
		void release( OctreeNode< vector< shared_ptr< T >, ManagedAllocator< shared_ptr< T > > > >& node )
		{
			using ElementPtr = shared_ptr< T >;
			using Allocator = ManagedAllocator< ElementPtr >;
			
			vector< ElementPtr, Allocator >& vector = node.getContents();
			for( ElementPtr& element : vector )
			{
				element = nullptr;
			}
			vector.clear();
		}
		
		template< typename T >
		void release( OctreeNode< vector< shared_ptr< T > > >& node )
		{
			using ElementPtr = shared_ptr< T >;
			vector< ElementPtr >& vector = node.getContents();
			for( ElementPtr& element : vector )
			{
				element = nullptr;
			}
			vector.clear();
		}
	};
}

#endif
