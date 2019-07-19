#ifndef OCTREE_FILE_WRITER_H
#define OCTREE_FILE_WRITER_H

#include <queue>
#include "omicron/hierarchy/o1_octree_node.h"

namespace omicron::disk
{
	/** Tools for writing and reading binary octree files, which contain raw node data. */
	class OctreeFile
	{
	public:
		using Node = O1OctreeNode< Surfel >;
		using NodePtr = unique_ptr< Node >;
		
		/** Writes a binary octree file in depth-first order.
		 * @param filename path to the file to be written with the octree.
		 * @param root the root node of the octree. */
		static void writeDepth( const string& filename, const Node& root );
		
		/** Writes a binary octree file in breadth-first order.
		 * @param filename path to the file to be written with the octree.
		 * @param root the root node of the octree. */
		static void writeBreadth( const string& filename, const Node& root );

		/** Reads an octree file written previously by writeDepth() or writeBreadth().
		 * @param filename path to the octree binary file.
		 * @returns a pointer to the read octree. */
		static NodePtr read( const string& filename );
	};
	
	inline void OctreeFile::writeDepth( const string& filename, const OctreeFile::Node& root )
	{
		cout << "Saving binary octree in depth-first order to " << filename << endl << endl;
		
		ofstream file( filename, ofstream::out | ofstream::binary );
		
		if( file.fail() )
		{
			stringstream ss; ss << filename << " could not be opened properly.";
			throw logic_error( ss.str() );
		}
		
		bool isDepth = true;
		Binary::write(file, isDepth);
		root.persist( file );
	}

	inline void OctreeFile::writeBreadth( const string& filename, const OctreeFile::Node& root )
	{
		cout << "Saving binary octree in breadth-first order to " << filename << endl << endl;
		
		ofstream file( filename, ofstream::out | ofstream::binary );
		
		if( file.fail() )
		{
			stringstream ss; ss << filename << " could not be opened properly.";
			throw logic_error( ss.str() );
		}
		
		bool isDepth = false;
		Binary::write(file, isDepth);

		std::queue<const Node*> q;
		q.push(&root);

		while(!q.empty())
		{
			const Node* node = q.front();
			q.pop();

			node->persistContents(file);

			uint nChildren = node->child().size();
			Binary::write(file, nChildren);

			for(const Node& node : node->child())
			{
				q.push(&node);
			}
		}
	}

	inline OctreeFile::NodePtr OctreeFile::read( const string& filename )
	{
		cout << "Loading binary octree from " << filename << endl << endl;
		
		ifstream file( filename, ofstream::in | ofstream::binary );
		
		if( file.fail() )
		{
			stringstream ss; ss << filename << " could not be opened properly.";
			throw logic_error( ss.str() );
		}
		
		bool isDepth;
		Binary::read(file, isDepth);

		if(isDepth)
		{
			return NodePtr( new Node( file ) );
		}
		else
		{
			// std::queue<Node&> q;
			// NodePtr root( new Node( file, Node::NoRecursionMark() ) );
			// q.push(*root);

			// while(!q.empty())
			// {
			// 	Node& node = q.front();
			// 	q.pop();

			// }

			// return root;
		}
	}
}

#endif
