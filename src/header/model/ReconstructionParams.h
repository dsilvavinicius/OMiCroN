#ifndef RECONSTRUCTION_PARAMS_H
#define RECONSTRUCTION_PARAMS_H

#include <Eigen/Dense>
#include <ostream>

using namespace Eigen;
using namespace std;

// ===== Independent parameters =====

// Number of threads used in the HierarchyCreator.
#define HIERARCHY_CREATION_THREADS 4

// Work list size for hierarchy creation. 
#define WORK_LIST_SIZE 8

#define MODEL DAVID
// #define MODEL ST_MATHEW
// #define MODEL ATLAS
// #define MODEL DUOMO

#define PARENT_POINTS_RATIO ONE_FOURTH
// #define PARENT_POINTS_RATIO ONE_FIFTH

// Number of expected front segments.
#define SEGMENTS_PER_FRONT 2

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

#define RECONSTRUCTION_ALG BHZK05

// ===== Dependent parameters =====
#if PARENT_POINTS_RATIO == ONE_FOURTH
	#define PARENT_POINTS_RATIO_VALUE 0.25f
#elif PARENT_POINTS_RATIO == ONE_FIFTH
	#define PARENT_POINTS_RATIO_VALUE 0.25f
#endif
	
#if MODEL == DAVID
	#define LEAF_SURFEL_TANGENT_SIZE_X 0.000037f
	#define LEAF_SURFEL_TANGENT_SIZE_Y 0.00003f
#elif MODEL == ATLAS
	#define LEAF_SURFEL_TANGENT_SIZE_X 0.00008f
	#define LEAF_SURFEL_TANGENT_SIZE_Y 0.000055f
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
					#if PARENT_POINTS_RATIO == ONE_FOURTH
						case 7: return Vector2f( 3.7f, 3.7f );
						case 6: return Vector2f( 2.5f, 2.0f );
						case 5: return Vector2f( 2.0f, 2.0f );
						case 4: return Vector2f( 2.0f, 2.0f );
						case 3: return Vector2f( 2.0f, 2.0f );
					#endif
				#elif MODEL == ATLAS
					#if PARENT_POINTS_RATIO == ONE_FOURTH
						case 7: return Vector2f( 3.5f, 3.1f );
						case 6: return Vector2f( 2.5f, 2.0f );
						case 5: return Vector2f( 2.0f, 2.0f );
						case 4: return Vector2f( 2.0f, 2.0f );
						case 3: return Vector2f( 2.0f, 2.0f );
					#endif
				#elif MODEL == ST_MATHEW
					#if PARENT_POINTS_RATIO == ONE_FOURTH
						case 7: return Vector2f( 3.7f, 3.5f );
						case 6: return Vector2f( 2.3f, 2.0f );
						case 5: return Vector2f( 2.0f, 2.0f );
						case 4: return Vector2f( 2.0f, 2.0f );
						case 3: return Vector2f( 2.0f, 2.0f );
					#endif
				#elif MODEL == DUOMO
					#if PARENT_POINTS_RATIO == ONE_FIFTH
						case 7: return Vector2f( 4.4f, 4.4f );
						case 6: return Vector2f( 3.0f, 2.1f );
						case 5: return Vector2f( 2.8f, 1.8f );
						case 4: return Vector2f( 2.0f, 2.0f );
						case 3: return Vector2f( 2.0f, 2.0f );
					#endif
				#endif
			}
		
			return Vector2f( 0.f, 0.f );
		}
	};
			
}

#endif