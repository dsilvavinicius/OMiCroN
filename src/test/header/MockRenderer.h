// #include "RenderingState.h"
// #include <StreamingRenderer.h>
// 
// namespace omicron
// {
// 	namespace test
// 	{
// 		/** A mock renderer that does not render anything. Any bounding box is culled and any projection threshold rendering
// 		 * test indicates that geometry should not be rendered. */
// 		template< typename Point >
// 		class MockRenderer
// 		: public StreamingRenderer< Point >
// 		{
// 			virtual unsigned int render() override {};
// 			
// 			virtual bool isCullable( const AlignedBox3f& box ) const override { return true; }
// 			
// 			virtual bool isRenderable( const AlignedBox3f& box, const Float projThresh ) const override { return false; }
// 			
// 			virtual void handleNodeRendering( const Array< shared_ptr< Point > >& points ) override {}
// 		};
// 	}
// }