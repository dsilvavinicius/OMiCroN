#include "RenderingState.h"

namespace model
{
	namespace test
	{
		/** A mock renderer that does not render anything. Any bounding box is culled and any projection threshold rendering
		 * test indicates that geometry should not be rendered. */
		class MockRenderer
		: public RenderingState
		{
			virtual unsigned int render() override {};
			
			virtual bool isCullable( const pair< Vec3, Vec3 >& box ) const override { return true; }
			
			virtual bool isRenderable( const pair< Vec3, Vec3 >& box, const Float& projThresh ) const override
			{ return false; }
			
			virtual void renderText( const Vec3& pos, const string& str ) override {}
			
			virtual void handleNodeRendering( const PointPtr& point ) override {}
			
			virtual void handleNodeRendering( const PointVector& points ) override {}
			
			virtual void handleNodeRendering( const ExtendedPointPtr& point ) override {}
			
			virtual void handleNodeRendering( const ExtendedPointVector& points ) override {}
			
			virtual void handleNodeRendering( const Array< PointPtr >& points ) override {}
			
			virtual void handleNodeRendering( const Array< ExtendedPointPtr >& points ) override {}
			
			virtual void handleNodeRendering( const IndexVector& points ) override {}
		};
	}
}