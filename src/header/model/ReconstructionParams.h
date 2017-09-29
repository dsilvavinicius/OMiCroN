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

#define HEAP_SORT 0
#define PARTIAL_SORT 1
#define FULL_SORT 2

enum ReconstructionAlgorithm
{
	ZPBG01 = 0,
	BHZK05 = 1,
	WHA07 = 2,
	ZRB04 = 3
};

// ===== Independent parameters =====

// Current dataset.
#define MODEL DAVID

// Define to indicates that the system will be run in the laboratory.
// #define LAB

// Number of threads used in the HierarchyCreator.
#define HIERARCHY_CREATION_THREADS 8
// #define HIERARCHY_CREATION_THREADS 4

// Work list size for hierarchy creation. 
#define WORK_LIST_SIZE 8
// #define WORK_LIST_SIZE 32

#define RAM_QUOTA 1024ul * 1024ul * 1024ul * 7ul

// Activates rendering in parallel with hierarchy creation.
#define HIERARCHY_CREATION_RENDERING

// The input is not sorted. The preprocessing step must be performed.
#define UNSORTED

// Selects the sorting algorithm. 
#define SORTING PARTIAL_SORT

// Number of segments for partial sorting.
#define SORTING_SEGMENTS 10

// Activates de loading of binary octree files instead of hierarchy creation.
// #define BINARY_OCTREE_FILE

#define PARENT_POINTS_RATIO_ONE_FOURTH
// #define PARENT_POINTS_RATIO_ONE_FIFTH
// #define PARENT_POINTS_RATIO_ONE_TENTH

// #define PROJ_THRESHOLD 0.2f
#define PROJ_THRESHOLD 0.05f
// #define PROJ_THRESHOLD 0.005f

// Number of expected front segments.
// #define SEGMENTS_PER_FRONT 2
// #define SEGMENTS_PER_FRONT 10
#define SEGMENTS_PER_FRONT 5

// Enables node colapse when leaves do not have siblings.
#define NODE_COLAPSE

// Number of placeholders expected to be substituted in the hierarchy creation.
// #define EXPECTED_SUBSTITUTED_PLACEHOLDERS 19777u // David + no leaf collapse
#define EXPECTED_SUBSTITUTED_PLACEHOLDERS 19895u // David + leaf collapse
// #define EXPECTED_SUBSTITUTED_PLACEHOLDERS 21187u // Atlas + no leaf collapse
// #define EXPECTED_SUBSTITUTED_PLACEHOLDERS 23711u // StMathew + no leaf collapse

#ifdef LAB
	#define GPU_MEMORY 7ul * 1024ul * 1024ul * 1024ul
#else
	#define GPU_MEMORY 1024ul * 1024ul * 1024ul
#endif

#define RECONSTRUCTION_ALG WHA07

// ===== Dependent parameters =====
#ifdef PARENT_POINTS_RATIO_ONE_FOURTH
	#define PARENT_POINTS_RATIO_VALUE 0.25f
#elif defined PARENT_POINTS_RATIO_ONE_FIFTH
	#define PARENT_POINTS_RATIO_VALUE 0.2f
#elif defined PARENT_POINTS_RATIO_ONE_TENTH
	#define PARENT_POINTS_RATIO_VALUE 0.1f
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

#if MODEL == DAVID
	#define CAMERA_PATH_SPEED 0.005
#elif MODEL == ATLAS
	#define CAMERA_PATH_SPEED 0.004
#elif MODEL == ST_MATHEW
	#define CAMERA_PATH_SPEED 0.002
#elif MODEL ==  DUOMO
	#define CAMERA_PATH_SPEED 0.001
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
						case 7: return Vector2f( 4.2f, 4.2f );
						case 6: return Vector2f( 2.5f, 2.0f );
						case 5: return Vector2f( 2.0f, 2.0f );
						case 4: return Vector2f( 2.0f, 2.0f );
						case 3: return Vector2f( 2.0f, 2.0f );
					#elif defined PARENT_POINTS_RATIO_ONE_FIFTH
						case 7: return Vector2f( 4.7f, 4.7f );
						case 6: return Vector2f( 3.0f, 2.5f );
						case 5: return Vector2f( 2.0f, 2.0f );
						case 4: return Vector2f( 2.5f, 2.5f );
						case 3: return Vector2f( 2.0f, 2.0f );
					#elif defined PARENT_POINTS_RATIO_ONE_TENTH
						case 7: return Vector2f( 6.f, 6.0f );
						case 6: return Vector2f( 3.0f, 2.5f );
						case 5: return Vector2f( 2.0f, 2.0f );
						case 4: return Vector2f( 2.5f, 2.5f );
						case 3: return Vector2f( 2.0f, 2.0f );
					#endif
				#elif MODEL == ATLAS
					#ifdef PARENT_POINTS_RATIO_ONE_FOURTH
						case 7: return Vector2f( 3.8f, 3.5f );
						case 6: return Vector2f( 2.5f, 2.0f );
						case 5: return Vector2f( 2.0f, 2.0f );
						case 4: return Vector2f( 2.0f, 2.0f );
						case 3: return Vector2f( 2.0f, 2.0f );
					#elif defined PARENT_POINTS_RATIO_ONE_FIFTH || defined PARENT_POINTS_RATIO_ONE_TENTH
						case 7: return Vector2f( 4.2f, 4.2f );
						case 6: return Vector2f( 2.5f, 2.0f );
						case 5: return Vector2f( 2.3f, 2.3f );
						case 4: return Vector2f( 2.3f, 2.3f );
						case 3: return Vector2f( 2.0f, 2.0f );
					#endif
				#elif MODEL == ST_MATHEW
					#ifdef PARENT_POINTS_RATIO_ONE_FOURTH
						case 7: return Vector2f( 4.2f, 4.0f );
						case 6: return Vector2f( 2.3f, 2.0f );
						case 5: return Vector2f( 2.0f, 2.0f );
						case 4: return Vector2f( 2.0f, 2.0f );
						case 3: return Vector2f( 2.0f, 2.0f );
					#elif defined PARENT_POINTS_RATIO_ONE_FIFTH || defined PARENT_POINTS_RATIO_ONE_TENTH
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
					#elif defined PARENT_POINTS_RATIO_ONE_FIFTH || defined PARENT_POINTS_RATIO_ONE_TENTH
						case 7: return Vector2f( 4.4f, 4.4f );
						case 6: return Vector2f( 3.0f, 2.1f );
						case 5: return Vector2f( 2.8f, 1.8f );
						case 4: return Vector2f( 2.0f, 2.0f );
						case 3: return Vector2f( 2.0f, 1.0f );
					#endif
				#endif
			}
		
			return Vector2f( 1.f, 1.f );
		}
	};
			
}

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

#endif