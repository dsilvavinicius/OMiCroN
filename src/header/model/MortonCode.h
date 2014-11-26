#ifndef MORTON_CODE_H
#define MORTON_CODE_H

#include <cassert>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <memory>
#include <boost/functional/hash.hpp>
#include <glm/ext.hpp>

#include "Stream.h"

using namespace std;

namespace model
{
	// Forward declaration.
	template <typename T> class MortonCode;
	
	//=================
	// Type sugar.
	//=================
	
	using ShallowMortonCode = MortonCode< unsigned int >;
	using MediumMortonCode = MortonCode< unsigned long >;
	using DeepMortonCode = MortonCode< unsigned long long >;
	
	using ShallowMortonCodePtr = shared_ptr<ShallowMortonCode>;
	using MediumMortonCodePtr = shared_ptr<MediumMortonCode>;
	using DeepMortonCodePtr = shared_ptr<DeepMortonCode>;
	
	template <typename T>
	using MortonCodePtr = shared_ptr< MortonCode<T> >;
	
	/** Morton code designed for use as an octree node index, represented by interleaving the bits of the node coordinate.
	 * To avoid collision of node indices, the code for a given node at level l in the Octree only considers the first
	 * l groups of interleaved bits. Also an 1 is concatenated at the end of the code to disambiguate with other
	 * nodes which related code is the same, but prefixed with zeroes.
	 *
	 * So the pattern of a code is:
	 * 0 ... | 1 | interleaved l | interleaved l-1 | ... | interleaved 1
	 * 
	 * For example, the code for node position (7, 5, 0) at level 3 is:
	 * 0 ... | 1 | 110 | 100 | 110.
	 */
	template <typename T>
	class MortonCode
	{
	public:
		/** Use this method to calculate code from position. */
		void build( const T& x, const T& y, const T& z, const unsigned int& level );
		
		/** Use this method to inform the code. */
		void build( const T& codeBits );
		
		/** Decodes this morton code into an array of 3 coordinates. Use this when the level of code is known
		 * a priori. */
		// TODO: Use a template to specify a vec3 type here.
		vector<T> decode( const unsigned int& level ) const;
		
		/** Decodes this morton code into an array of 3 coordinates. Use this when the level of the code is
		 * unknown (slower). */
		// TODO: Use a template to specify a vec3 type here.
		vector<T> decode() const;
		
		/** Computes the level of this code. */
		unsigned int getLevel() const;
		
		T getBits() const;
		
		shared_ptr< MortonCode< T > > traverseUp() const;
		vector< shared_ptr< MortonCode< T > >  > traverseDown() const;
		
		bool operator==( const MortonCode& other ) const;
		bool operator!=( const MortonCode& other ) const;
		
		/** Prints the nodes in the path from this node to the root node.
		 * @param simple indicates that the node should be printed in a simpler representation. */
		void printPathToRoot( ostream& out, bool simple ) const;
		
		template< typename Precision >
		friend ostream& operator<<( ostream& out, const MortonCode<Precision>& dt );
		
		template< typename Precision >
		friend ostream& operator<<( ostream& out, const MortonCodePtr<Precision>& dt );
		
	private:
		/** Spreads the bits to build Morton code. */
		T spread3( T x );
		
		/** Compacts the bits in code to decode it as a coordinate. */
		T compact3( T x ) const;
		
		T m_bits;
	};
	
	//===============
	// Implementation
	//===============
	
	template <typename T>
	void MortonCode<T>::build(const T& x, const T& y, const T& z, const unsigned int& level)
	{
		m_bits = spread3(x) | (spread3(y) << 1) | (spread3(z) << 2);
		
		T twoPowLevel = (T)1 << (level * 3);
		
		// Discards all bits in positions greater than level * 3.
		m_bits &= (twoPowLevel) - 1;
		// Put the 1 sufix.
		m_bits |= twoPowLevel;
	}
	
	template <typename T>
	inline void MortonCode<T>::build(const T& codeBits) { m_bits = codeBits; }
	
	template <typename T>
	inline vector<T> MortonCode<T>::decode(const unsigned int& level) const
	{
		vector<T> coords(3);
		
		T clearPrefixMask = (T(1) << T(3 * level)) - 1;
		T bits = m_bits & clearPrefixMask;
		
		coords[0] = compact3(bits);
		coords[1] = compact3(bits >> 1);
		coords[2] = compact3(bits >> 2);
		
		return coords;
	}
	
	template <typename T>
	inline vector<T> MortonCode<T>::decode() const
	{
		unsigned int level = getLevel();
		return decode(level);
	}
	
	template <typename T>
	inline unsigned int MortonCode<T>::getLevel() const
	{
		// Finds the MortonCode level.
		unsigned int numBits = sizeof(T) * 8;
		T bits = m_bits;
		unsigned int level = 0;
		for (level = numBits / 3; level > 0; --level)
		{
			unsigned int shift = level * 3;
			if ((bits & ((T)1 << shift)) != 0) { break; }
		}
		
		return level;
	}
	
	template <typename T>
	inline T MortonCode<T>::getBits() const{ return m_bits; }
	
	template <typename T>
	inline MortonCodePtr<T> MortonCode<T>::traverseUp() const
	{
		assert( m_bits > 1 );
		T bits = m_bits >> 3;
		MortonCodePtr<T> parentMorton = make_shared< MortonCode<T> >();
		parentMorton->build(bits);
		return parentMorton;
	}
	
	template <typename T>
	inline vector< MortonCodePtr< T > >  MortonCode<T>::traverseDown() const
	{
		T bits = m_bits;
		T shifted = bits << 3;
		
		if( shifted < bits )
		{
			// Overflow: cannot go deeper.
			return vector< MortonCodePtr<T> >();
		}
		//assert( shifted > bits && "MortonCode traversal overflow." );
		
		vector< MortonCodePtr<T> > children(8);
		
		for (int i = 0; i < 8; ++i)
		{
			MortonCodePtr<T> child = make_shared< MortonCode<T> >();
			child->build(shifted | i);
			children[i] = child;
		}
		
		return children;
	}
	
	template <typename T>
	inline bool MortonCode< T >::operator==(const MortonCode< T >& other) const
	{
		return m_bits == other.m_bits;
	}
	
	template <typename T>
	inline bool MortonCode< T >::operator!=(const MortonCode< T >& other) const
	{
		return !( m_bits == other.m_bits );
	}
	
	template <typename T>
	void MortonCode< T >::printPathToRoot(ostream& out, bool simple) const
		{
			MortonCode< T > code = *this;
			out << "Path to root: ";
			
			if (simple)
			{
				while( code.getBits() != 1 )
				{
					out << "0x" << hex << code.getBits() << dec << "->";
					code = *code.traverseUp();
				}
				out << "0x" << hex << code.getBits() << dec;
			}
			else
			{
				while( code.getBits() != 1 )
				{
					out << code << dec << " -> ";
					code = *code.traverseUp();
				}
				out << code << dec;
			}
		}
	
	template <typename Precision>
	ostream& operator<<(ostream& out, const MortonCode<Precision>& code)
	{
		unsigned int level = code.getLevel();
		vector< Precision > decoded = code.decode(level);
		out << "MortonCode: " << endl << "level = " << level << endl
			<< "coords = " << decoded << endl;
		
		code.printPathToRoot( out, true );
		
		out << endl;
		
		return out;
	}
	
	template <typename Precision>
	ostream& operator<<(ostream& out, const MortonCodePtr<Precision>& code)
	{
		out << *code;
		return out;
	}
	
	//=================
	// Specializations.
	//=================
	
	/** "Spreads" coordinate bits to build Morton code. Applied bit-wise operations are explained here:
	 * http://stackoverflow.com/a/18528775/1042102 */
	template <>
	inline unsigned int MortonCode< unsigned int >::spread3(unsigned int x)
	{
		x &= 0x3ff;
		x = (x | x << 16) & 0x30000ff;
		x = (x | x << 8) & 0x300f00f;
		x = (x | x << 4) & 0x30c30c3;
		x = (x | x << 2) & 0x9249249;
		
		return x;
	}
	
	template <>
	inline unsigned int MortonCode< unsigned int >::compact3(unsigned int x) const
	{
		x &= 0x09249249;
		x = (x ^ (x >>  2)) & 0x030c30c3;
		x = (x ^ (x >>  4)) & 0x0300f00f;
		x = (x ^ (x >>  8)) & 0xff0000ff;
		x = (x ^ (x >> 16)) & 0x000003ff;
		
		return x;
	}
	
	/** "Spreads" coordinate bits to build Morton code. Applied bit-wise operations are explained here:
	 * http://stackoverflow.com/a/18528775/1042102 */
	template <>
	inline unsigned long MortonCode< unsigned long >::spread3(unsigned long x)
	{
		x &= 0x1fffffUL;
		x = (x | x << 32UL) & 0x1f00000000ffffUL;
		x = (x | x << 16UL) & 0x1f0000ff0000ffUL;
		x = (x | x << 8UL) & 0x100f00f00f00f00fUL;
		x = (x | x << 4UL) & 0x10c30c30c30c30c3UL;
		x = (x | x << 2UL) & 0x1249249249249249UL;
		
		return x;
	}
	
	template <>
	inline unsigned long MortonCode< unsigned long >::compact3(unsigned long x) const
	{
		x &= 0x1249249249249249UL;
		x = (x ^ x >> 2UL) & 0x10c30c30c30c30c3UL;
		x = (x ^ x >> 4UL) & 0x100f00f00f00f00fUL;
		x = (x ^ x >> 8UL) & 0x001f0000ff0000ffUL;
		x = (x ^ x >> 16UL) & 0x001f00000000ffffUL;
		x = (x ^ x >> 32UL) & 0x00000000001fffffUL;
		
		return x;
	}
	
	/** "Spreads" coordinate bits to build Morton code. Applied bit-wise operations are explained here:
	 * http://stackoverflow.com/a/18528775/1042102 */
	template <>
	inline unsigned long long MortonCode< unsigned long long >::spread3(unsigned long long x)
	{
		x &= 0x3ffffffffffLL;
		x = (x | x << 64) & 0x3ff0000000000000000ffffffffLL;
		x = (x | x << 32) & 0x3ff00000000ffff00000000ffffLL;
		x = (x | x << 16) & 0x30000ff0000ff0000ff0000ff0000ffLL;
		x = (x | x << 8) & 0x300f00f00f00f00f00f00f00f00f00fLL;
		x = (x | x << 4) & 0x30c30c30c30c30c30c30c30c30c30c3LL;
		x = (x | x << 2) & 0x9249249249249249249249249249249LL;
		
		return x;
	}
	
	// TODO: Finish this (so boring!).
	/*template <>
	inline unsigned long long MortonCode< unsigned long long >::compact3(unsigned long long x) const
	{
		x &= 0x3ffffffffffLL;
		x = (x | x << 64) & 0x3ff0000000000000000ffffffffLL;
		x = (x | x << 32) & 0x3ff00000000ffff00000000ffffLL;
		x = (x | x << 16) & 0x30000ff0000ff0000ff0000ff0000ffLL;
		x = (x | x << 8) & 0x300f00f00f00f00f00f00f00f00f00fLL;
		x = (x | x << 4) & 0x30c30c30c30c30c30c30c30c30c30c3LL;
		x = (x | x << 2) & 0x9249249249249249249249249249249LL;
		
		return x;
	}*/
}

namespace std
{
	template<>
    struct hash< model::MortonCode< unsigned int > >
    {
        typedef model::MortonCode< unsigned int > argument_type;
        typedef size_t result_type;

        size_t operator()( const model::MortonCode< unsigned int >& code ) const
        {
			boost::hash< unsigned int > hasher;
			return hasher( code.getBits() );
        }
    };
}

#endif