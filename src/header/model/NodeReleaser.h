#ifndef NODE_RELEASER
#define NODE_RELEASER

#include <iostream>
#include <vector>
#include <memory>

using namespace std;

namespace model
{
	template< typename MortonCode, typename Contents >
	class LeafNode;
	
	template< typename MortonCode, typename Contents >
	class InnerNode;
	
	/** Methods to release node contents. */
	namespace NodeReleaser
	{
		/** Default implementation assumes that no additional efforts should be done in order to release contents. */
		template< typename MortonCode, typename Contents >
		void releaseLeaf( LeafNode< MortonCode, Contents >& node )
		{}
		
		/** Default implementation assumes that no additional efforts should be done in order to release contents. */
		template< typename MortonCode, typename Contents >
		void releaseInner( InnerNode< MortonCode, Contents >& node )
		{}
		
		template< typename MortonCode, typename T >
		void releaseLeaf( LeafNode< MortonCode, vector< shared_ptr< T > > >& node )
		{
			using ElementPtr = shared_ptr< T >;
			vector< ElementPtr >& vector = node.getContents();
			for( ElementPtr& element : vector )
			{
				element = nullptr;
			}
			vector.clear();
		}
		
		template< typename MortonCode, typename T >
		void releaseInner( InnerNode< MortonCode, vector< shared_ptr< T > > >& node )
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