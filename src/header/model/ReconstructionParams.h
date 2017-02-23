#ifndef RECONSTRUCTION_PARAMS_H
#define RECONSTRUCTION_PARAMS_H

#include <Eigen/Dense>
#include <ostream>

using namespace Eigen;
using namespace std;

// ===== Identifiers =====

#define DAVID 0
#define ST_MATHEW 1
#define ATLAS 2
#define DUOMO 3

// ===== Independent parameters =====

// Current dataset.
#define MODEL DUOMO

// Number of threads used in the HierarchyCreator.
#define HIERARCHY_CREATION_THREADS 8
// #define HIERARCHY_CREATION_THREADS 4

// Work list size for hierarchy creation. 
#define WORK_LIST_SIZE 128
// #define WORK_LIST_SIZE 8

#define PARENT_POINTS_RATIO_ONE_FOURTH
// #define PARENT_POINTS_RATIO_ONE_FIFTH

#define PROJ_THRESHOLD 0.2f

// Number of expected front segments.
#define SEGMENTS_PER_FRONT 2

#define GPU_MEMORY 7ul * 1024ul * 1024ul * 1024ul
// #define GPU_MEMORY 1024ul * 1024ul * 1024ul

enum ReconstructionAlgorithm
{
	ZPBG01 = 0,
	BHZK05 = 1,
	WHA07 = 2,
	ZRB04 = 3
};

inline ostream& operator<<( ostream& out, const ReconstructionAlgorithm alg )
{
	switch( alg )
	{
		case ZPBG01 : out << "ZPBG01"; break;
		case BHZK05 : out << "BHZK05"; break;
		case WHA07 : out << "WHA07"; break;
		case ZRB04 : out << "ZRB04"; break;
	}
	
	return out;
}

#define RECONSTRUCTION_ALG WHA07

// ===== Dependent parameters =====
#ifdef PARENT_POINTS_RATIO_ONE_FOURTH
	#define PARENT_POINTS_RATIO_VALUE 0.25f
#elif defined PARENT_POINTS_RATIO_ONE_FIFTH
	#define PARENT_POINTS_RATIO_VALUE 0.2f
#endif
	
#if MODEL == DAVID
	#define LEAF_SURFEL_TANGENT_SIZE_X 0.000037f
	#define LEAF_SURFEL_TANGENT_SIZE_Y 0.00003f
#elif MODEL == ATLAS
	#define LEAF_SURFEL_TANGENT_SIZE_X 0.00008f
	#define LEAF_SURFEL_TANGENT_SIZE_Y 0.00007f
#elif MODEL == ST_MATHEW
	#define LEAF_SURFEL_TANGENT_SIZE_X 0.000085f
	#define LEAF_SURFEL_TANGENT_SIZE_Y 0.000055f
#elif MODEL ==  DUOMO
	#define LEAF_SURFEL_TANGENT_SIZE_X 0.00008f
	#define LEAF_SURFEL_TANGENT_SIZE_Y 0.00002f
#endif

namespace model
{
	class ReconstructionParams
	{
	public:
		static Vector2f calcMultipliers( const uint level )
		{
			switch( level )
			{
				#if MODEL == DAVID
					#ifdef PARENT_POINTS_RATIO_ONE_FOURTH
						case 7: return Vector2f( 3.7f, 3.7f );
						case 6: return Vector2f( 2.5f, 2.0f );
						case 5: return Vector2f( 2.0f, 2.0f );
						case 4: return Vector2f( 2.0f, 2.0f );
						case 3: return Vector2f( 2.0f, 2.0f );
					#elif defined PARENT_POINTS_RATIO_ONE_FIFTH
						case 7: return Vector2f( 4.5f, 4.5f );
						case 6: return Vector2f( 3.0f, 2.5f );
						case 5: return Vector2f( 2.0f, 2.0f );
						case 4: return Vector2f( 2.5f, 2.5f );
						case 3: return Vector2f( 2.0f, 2.0f );
					#endif
				#elif MODEL == ATLAS
					#ifdef PARENT_POINTS_RATIO_ONE_FOURTH
						case 7: return Vector2f( 3.5f, 3.1f );
						case 6: return Vector2f( 2.5f, 2.0f );
						case 5: return Vector2f( 2.0f, 2.0f );
						case 4: return Vector2f( 2.0f, 2.0f );
						case 3: return Vector2f( 2.0f, 2.0f );
					#elif defined PARENT_POINTS_RATIO_ONE_FIFTH
						case 7: return Vector2f( 4.0f, 3.6f );
						case 6: return Vector2f( 2.5f, 2.0f );
						case 5: return Vector2f( 2.3f, 2.3f );
						case 4: return Vector2f( 2.3f, 2.3f );
						case 3: return Vector2f( /*2.0f, 2.0f*/ 0.f, 0.f );
					#endif
				#elif MODEL == ST_MATHEW
					#ifdef PARENT_POINTS_RATIO_ONE_FOURTH
						case 7: return Vector2f( 3.7f, 3.5f );
						case 6: return Vector2f( 2.3f, 2.0f );
						case 5: return Vector2f( 2.0f, 2.0f );
						case 4: return Vector2f( 2.0f, 2.0f );
						case 3: return Vector2f( 2.0f, 2.0f );
					#elif defined PARENT_POINTS_RATIO_ONE_FIFTH
						case 7: return Vector2f( 4.7, 4.5f );
						case 6: return Vector2f( 2.6f, 2.3f );
						case 5: return Vector2f( 2.2f, 2.2f );
						case 4: return Vector2f( 2.5f, 2.5f );
						case 3: return Vector2f( 2.0f, 2.0f );
					#endif
				#elif MODEL == DUOMO
					#ifdef PARENT_POINTS_RATIO_ONE_FOURTH
						case 7: return Vector2f( 4.2f, 4.2f );
						case 6: return Vector2f( 2.7f, 1.9f );
						case 5: return Vector2f( 2.6f, 1.5f );
						case 4: return Vector2f( 2.0f, 1.7f );
						case 3: return Vector2f( 2.0f, 2.0f );
					#elif defined PARENT_POINTS_RATIO_ONE_FIFTH
						case 7: return Vector2f( 4.4f, 4.4f );
						case 6: return Vector2f( 3.0f, 2.1f );
						case 5: return Vector2f( 2.8f, 1.8f );
						case 4: return Vector2f( 2.0f, 2.0f );
						case 3: return Vector2f( 2.0f, 1.0f );
					#endif
				#endif
			}
		
			return Vector2f( 0.f, 0.f );
		}
	};
			
}

#endif