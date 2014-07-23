#ifndef OCTREE_H
#define OCTREE_H

#include <map>
#include "MortonCode.h"
#include "OctreeNode.h"

using namespace std;

namespace model
{	
	template <typename T>
	class Octree
	{
	public:
		virtual void build(vector<shared_ptr<Point>> points);
		virtual void traverse();
	private:
		map<MortonCode<T>, OctreeNode> hierarchy;
	};
	
	template <typename T>
	using OctreePtr = shared_ptr<Octree<typename MortonCode<T>::type>>
}

#endif