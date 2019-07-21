#ifndef OCTREE_FILE_WRITER_H
#define OCTREE_FILE_WRITER_H

#include <queue>
#include "omicron/hierarchy/o1_octree_node.h"
#include "omicron/hierarchy/octree_dimensions.h"

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

		/** Asynchronously reads an octree file written previously by writeBreadth().
		 * @param filename is the path to the octree binary file.
		 * @param dimension contains the octree dimensions used to calculate morton codes from node data.
		 * @param onLevelDone describes what to do when a hierarchy level is finished.
		 * @returns a pointer to the read octree. */
		template<typename Morton>
		static pair<typename OctreeFile::NodePtr, future<void>> asyncRead(
			const string& filename, const OctreeDimensions<Morton>& dimensions, const function< void(uint) >& onLevelDone = [](uint){});

	private:
		// Reads the header of the file
		// @returns the open file and a boolean equals to true if the file is in depth-first order, false otherwise (breadth-first order).
		static pair<ifstream, bool> readHeader(const string& filename);

		// Reads the remaining of a breadth-first ordered file, after the root is read. Enables support for async read, since the root can be returned to the calling thread and the remaing of the file can be read by another thread.
		// @param root is the root of hierarchy, already read.
		// @param file is the octree file, ordered in breadth-first order.
		// @param onSiblingGroupRead 
		static void readBreadthFirst(NodePtr& root, ifstream& file,
			const function< void(const typename Node::NodeArray&) >& onSiblingGroupRead = [](const typename Node::NodeArray&){});
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

			if(!node->isLeaf())
			{
				uint nChildren = node->child().size();
				Binary::write(file, nChildren);
			}

			for(const Node& node : node->child())
			{
				q.push(&node);
			}
		}
	}

	inline OctreeFile::NodePtr OctreeFile::read( const string& filename )
	{
		pair<ifstream, bool> fileAndHeader = readHeader(filename);
		ifstream file(std::move(fileAndHeader.first));
		bool isDepth = fileAndHeader.second;

		if(isDepth)
		{
			cout << "Depth-first order format detected." << endl << endl;

			return NodePtr( new Node( file ) );
		}
		else
		{
			cout << "Breadth-first order format detected." << endl << endl;

			NodePtr root(new Node(file, Node::NoRecursionMark()));
			
			readBreadthFirst(root, file);
			
			return root;
		}
	}

	template<typename Morton>
	inline pair<typename OctreeFile::NodePtr, future<void>> OctreeFile::asyncRead(
			const string& filename, const OctreeDimensions<Morton>& dimensions, const function< void(uint levelDone) >& onLevelDone)
	{
		pair<ifstream, bool> fileAndHeader = OctreeFile::readHeader(filename);
		ifstream file = std::move(fileAndHeader.first);
		bool isDepth = fileAndHeader.second;

		if(isDepth)
		{
			throw logic_error("Octree file must have breadth-first ordered contents to be read asynchronously.");
		}
		else
		{
			cout << "Breadth-first order format detected." << endl << endl;

			OctreeFile::NodePtr root(new OctreeFile::Node(file, OctreeFile::Node::NoRecursionMark()));

			OctreeDimensions<Morton> currentLvlDim(dimensions, 0);
			Morton previousMorton = currentLvlDim.calcMorton(*root);

			future<void> readFuture = std::async(std::launch::async,
				[&]{
					readBreadthFirst(root, file,
						[&](const typename OctreeFile::Node::NodeArray& siblings){
							Morton morton = currentLvlDim.calcMorton(siblings[0]);
							if(morton <= previousMorton)
							{
								// New level detected
								onLevelDone(currentLvlDim.level());
								currentLvlDim = OctreeDimensions<Morton>(currentLvlDim, currentLvlDim.level() + 1);
							}
							previousMorton = morton;
						}
					);
				}
			);

			return pair<OctreeFile::NodePtr, future<void>>(std::move(root), std::move(readFuture));
		}
	}

	inline pair<ifstream,bool> OctreeFile::readHeader(const string& filename)
	{
		cout << "Loading binary octree from " << filename << endl << endl;
		
		ifstream file(filename, ofstream::in | ofstream::binary );
		
		if(file.fail())
		{
			stringstream ss; ss << filename << " could not be opened properly.";
			throw logic_error( ss.str() );
		}
		
		bool isDepth;
		Binary::read(file, isDepth);

		return pair<ifstream, bool>(std::move(file), isDepth);
	}

	inline void OctreeFile::readBreadthFirst(NodePtr& root, ifstream& file, const function< void(const typename Node::NodeArray&) >& onSiblingGroupRead)
	{
		std::queue<pair<Node*, uint>> q;
			
		if(!root->isLeaf())
		{
			pair<Node*, uint> queueRoot;
			queueRoot.first = root.get();
			Binary::read(file, queueRoot.second);
			q.push(queueRoot);
		}

		while(!q.empty())
		{
			Node* node = q.front().first;
			typename Node::NodeArray children(q.front().second);
			q.pop();
			
			for(typename Node::NodeArray::iterator it = children.begin(); it != children.end(); ++it)
			{
				*it = Node(file, Node::NoRecursionMark());
				it->setParent(node);
				
				if(!it->isLeaf())
				{
					pair<Node*, uint> queueNode;
					queueNode.first = it;
					Binary::read(file, queueNode.second);
					q.push(queueNode);
				}
			}
			
			node->setChildren(std::move(children));
			onSiblingGroupRead(node->child());
		}
	}
}

#endif