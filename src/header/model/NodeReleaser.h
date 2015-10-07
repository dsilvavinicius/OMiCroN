#ifndef NODE_RELEASER
#define NODE_RELEASER

#include <iostream>
#include <vector>
#include <memory>
#include "BitMapAllocator.h"

using namespace std;

namespace model
{
	template< typename Contents >
	class LeafNode;
	
	template< typename Contents >
	class InnerNode;
	
	/** Methods to release node contents. */
	namespace NodeReleaser
	{
		/** Default implementation assumes that no additional efforts should be done in order to release contents. */
		template< typename Contents >
		void releaseLeaf( LeafNode< Contents >& node )
		{}
		
		/** Default implementation assumes that no additional efforts should be done in order to release contents. */
		template< typename Contents >
		void releaseInner( InnerNode< Contents >& node )
		{}
		
		template< typename T >
		void releaseLeaf( LeafNode< vector< shared_ptr< T >, BitMapAllocator< shared_ptr< T > > > >& node )
		{
			using ElementPtr = shared_ptr< T >;
			using Allocator = BitMapAllocator< ElementPtr >;
			
			vector< ElementPtr, Allocator >& vector = node.getContents();
			for( ElementPtr& element : vector )
			{
				element = nullptr;
			}
			vector.clear();
		}
		
		template< typename T >
		void releaseInner( InnerNode< vector< shared_ptr< T >, BitMapAllocator< shared_ptr< T > > > >& node )
		{
			using ElementPtr = shared_ptr< T >;
			using Allocator = BitMapAllocator< ElementPtr >;
			
			vector< ElementPtr, Allocator >& vector = node.getContents();
			for( ElementPtr& element : vector )
			{
				element = nullptr;
			}
			vector.clear();
		}
		
		template< typename T >
		void releaseLeaf( LeafNode< vector< shared_ptr< T > > >& node )
		{
			using ElementPtr = shared_ptr< T >;
			vector< ElementPtr >& vector = node.getContents();
			for( ElementPtr& element : vector )
			{
				element = nullptr;
			}
			vector.clear();
		}
		
		template< typename T >
		void releaseInner( InnerNode< vector< shared_ptr< T > > >& node )
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