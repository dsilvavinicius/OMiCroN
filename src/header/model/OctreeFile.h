#ifndef OCTREE_FILE_WRITER_H
#define OCTREE_FILE_WRITER_H

#include "O1OctreeNode.h"

namespace model
{
	/** Tools for writing and reading binary octree files, which contain raw node data. */
	class OctreeFile
	{
	public:
		using Node = O1OctreeNode< Surfel >;
		using NodePtr = unique_ptr< Node >;
		
		/** Writes a binary octree file.
		 * @param filename path to the file to be written with the octree.
		 * @param root the root node of the octree. */
		static void write( const string& filename, const Node& root );
		
		/** Reads an octree file written previously by write( const string&, const Node& ).
		 * @param filename path to the octree binary file.
		 * @returns a pointer to the read octree. */
		static NodePtr read( const string& filename );
	};
	
	inline void OctreeFile::write( const string& filename, const OctreeFile::Node& root )
	{
		cout << "Saving binary octree to " << filename << endl << endl;
		
		ofstream file( filename, ofstream::out | ofstream::binary );
		
		if( file.fail() )
		{
			stringstream ss; ss << filename << " could not be opened properly.";
			throw logic_error( ss.str() );
		}
		
		root.persist( file );
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
		
		auto root = new Node( file );
		
		return NodePtr( root );
	}
}

#endif