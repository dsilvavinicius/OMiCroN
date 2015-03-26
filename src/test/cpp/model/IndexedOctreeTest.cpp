#include <gtest/gtest.h>
#include <QApplication>

#include "IndexedOctree.h"
#include "Stream.h"

extern "C" int g_argc;
extern "C" char** g_argv;
extern "C" string g_appPath;

namespace model
{
	namespace test
	{
		template< typename P >
        class IndexedOctreeTest
        : public ::testing::Test
        {};
		
		
		template<>
		class OctreeTest< Point< float, vec3 > >
		: public ::testing::Test
		{
			using Point = model::Point< float, vec3 >;
			using PointVector = vector< shared_ptr< Point > >;
			
		public:
			static void SetUpTestCase()
			{
				string exeFilename = string( g_argv[ 0 ] );
				m_plyFileName = new string( g_appPath +
					"/../../../src/data/tests/simple_point_octree.ply" );
			}
			
			static string* m_plyFileName;
		};
		
		string* OctreeTest< Point< float, vec3 > >::m_plyFileName;
	}
}
		