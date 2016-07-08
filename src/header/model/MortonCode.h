#ifndef MORTON_CODE_H
#define MORTON_CODE_H

#include <cassert>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <memory>
#include <qsurfaceformat.h>

#include "Stream.h"
#include "IMemoryManager.h"
#include "MemoryUtils.h"

using namespace std;

namespace model
{
	// Forward declaration.
	template< typename T > class MortonCode;
	
	template< typename T >
	using MortonCodePtr = shared_ptr< MortonCode< T > >;
	
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
		//TODO: Put move constructors here.
		void* operator new( size_t size );
		void* operator new[]( size_t size );
		void operator delete( void* p );
		void operator delete[]( void* p );
		
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
		uint getLevel() const;
		
		T getBits() const;
		
		shared_ptr< MortonCode< T > > traverseUp() const;
		vector< shared_ptr< MortonCode< T > >  > traverseDown() const;
		
		/** @param lvl is the lvl of the returned descendant.
		 * @returns the descendant of this MortonCode in the desired level. */
		MortonCode getAncestorInLvl( uint lvl ) const;
		
		MortonCode getFirstDescendantInLvl( uint lvl ) const;
		
		/** @returns the first child of this morton code. The first child has the morton code of the father appended
		 * with the bitmask 000. */
		MortonCodePtr< T > getFirstChild() const;
		
		/** @returns the last child of this morton code. The last child has the morton code of the father appended
		 * with the bitmask 111. */
		MortonCodePtr< T > getLastChild() const;
		
		/** Returns the children code closed interval [ a, b ] for this morton code.
		 * @returns a pair with first element being a and second element being b. */
		pair< MortonCodePtr< T >, MortonCodePtr< T > > getChildInterval() const;
		
		/** Checks if this MortonCode is child the code passed as parameter. */
		bool isChildOf( const MortonCode& code ) const;
		
		/** Checks if this MortonCode is descendent of code. */
		bool isDescendantOf( const MortonCode& code ) const;
		
		/** Gets the morton code that precede this one. */
		MortonCodePtr< T > getPrevious() const;
		
		/** Gets the morton code that procede this one. */
		MortonCodePtr< T > getNext() const;
		
		bool operator==( const MortonCode& other ) const;
		bool operator!=( const MortonCode& other ) const;
		bool operator< ( const MortonCode& other ) const;
		bool operator<=( const MortonCode& other ) const;
		
		/** Returns a string with the code hexadecimal representation. */
		string toString() const;
		
		/** Returns a string indicating the path to the root code from this code.
		 * @param simple indicates that the node should be printed in a simpler representation. */
		string getPathToRoot( bool simple ) const;
		
		/** Gets the first code of a given lvl. */
		static MortonCode< T > getLvlFirst( const unsigned int& lvl );
		
		/** Gets the last code of a given lvl. */
		static MortonCode< T > getLvlLast( const unsigned int& lvl );
		
		/** Returns the maximum allowed lvl for this MortonCode type. */
		static uint maxLvl(); 
		
		template< typename Precision >
		friend ostream& operator<<( ostream& out, const MortonCode<Precision>& dt );
		
		template< typename Precision >
		friend ostream& operator<<( ostream& out, const MortonCodePtr< Precision >& dt );
		
	private:
		/** Spreads the bits to build Morton code. */
		T spread3( T x );
		
		/** Compacts the bits in code to decode it as a coordinate. */
		T compact3( T x ) const;
		
		T m_bits;
	};
	
	template< typename T >
	inline void* MortonCode< T >::operator new( size_t size )
	{
		return ManagedAllocator< MortonCode< T > >().allocate( 1 );
	}
	
	template< typename T >
	inline void* MortonCode< T >::operator new[]( size_t size )
	{
		return ManagedAllocator< MortonCode< T > >().allocate( size / sizeof( MortonCode< T > ) );
	}
	
	template< typename T >
	inline void MortonCode< T >::operator delete( void* p )
	{
		ManagedAllocator< MortonCode< T > >().deallocate(
			static_cast< typename ManagedAllocator< MortonCode< T > >::pointer >( p ), 1
		);
	}
	
	template< typename T >
	inline void MortonCode< T>::operator delete[]( void* p )
	{
		ManagedAllocator< MortonCode< T > >().deallocate(
			static_cast< typename ManagedAllocator< MortonCode< T > >::pointer >( p ), 2
		);
	}
	
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
	inline T MortonCode<T>::getBits() const{ return m_bits; }
	
	template <typename T>
	inline MortonCodePtr<T> MortonCode<T>::traverseUp() const
	{
		assert( m_bits > 1 );
		T bits = m_bits >> 3;
		MortonCodePtr<T> parentMorton = makeManaged< MortonCode< T > >();
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
		
		vector< MortonCodePtr<T> > children(8);
		
		for (int i = 0; i < 8; ++i)
		{
			MortonCodePtr<T> child = makeManaged< MortonCode< T > >();
			child->build(shifted | i);
			children[i] = child;
		}
		
		return children;
	}
	
	template <typename T>
	inline MortonCode< T > MortonCode< T >::getAncestorInLvl( uint lvl ) const
	{
		uint thisLvl = getLevel();
		assert( lvl <= thisLvl && "An ancestor should be above in the hierarchy." );
		
		T ancestorBits = m_bits >> ( thisLvl - lvl ) * 3;
		
		MortonCode< T > ancestor;
		ancestor.build( ancestorBits );
		return ancestor;
	}
	
	template <typename T>
	inline MortonCode< T > MortonCode< T >::getFirstDescendantInLvl( const uint lvl ) const
	{
		assert( lvl <= maxLvl() && "The level should not be greater than the maximum level." );
		
		uint thisLvl = getLevel();
		assert( lvl >= thisLvl && "An descendant should be bellow in the hierarchy." );
		
		T descendantBits = m_bits << ( lvl - thisLvl ) * 3;
		
		MortonCode< T > descendant;
		descendant.build( descendantBits );
		return descendant;
	}
	
	template <typename T>
	inline MortonCodePtr< T > MortonCode< T >::getFirstChild() const
	{
		MortonCodePtr< T > firstChild = makeManaged< MortonCode< T > >();
		firstChild->build( m_bits << 3 );
		return firstChild;
	}
	
	template <typename T>
	inline MortonCodePtr< T > MortonCode< T >::getLastChild() const
	{
		MortonCodePtr< T > lastChild = makeManaged< MortonCode< T > >();
		lastChild->build( ( m_bits << 3 ) | ( T ) 0x7 );
		return lastChild;
	}
	
	template <typename T>
	inline pair< MortonCodePtr< T >, MortonCodePtr< T > > MortonCode< T >::getChildInterval() const
	{
		MortonCodePtr< T > a = getFirstChild();
		MortonCodePtr< T > b = getLastChild();
		
		return pair< MortonCodePtr< T >, MortonCodePtr< T > >( a, b );
	}
	
	template <typename T>
	inline bool MortonCode< T >::isChildOf( const MortonCode& code ) const
	{
		return code.getBits() == ( m_bits >> 3 );
	}
	
	template <typename T>
	inline bool MortonCode< T >::isDescendantOf( const MortonCode& code ) const
	{
		uint ancestorLvl = code.getLevel();
		uint lvlDiff = getLevel() - ancestorLvl;
		
		return code.getBits() == ( m_bits >> ( 3 * lvlDiff ) );
	}
	
	template <typename T>
	inline MortonCodePtr< T > MortonCode< T >::getPrevious() const
	{
		MortonCodePtr< T > prev = makeManaged< MortonCode< T > >();
		prev->build( m_bits - ( T )1 );
		return prev;
	}
	
	template <typename T>
	inline MortonCodePtr< T > MortonCode< T >::getNext() const
	{
		MortonCodePtr< T > next = makeManaged< MortonCode< T > >();
		next->build( m_bits + ( T )1 );
		return next;
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
	inline bool MortonCode< T >::operator<( const MortonCode& other ) const
	{
		return m_bits < other.m_bits;
	}
	
	template <typename T>
	inline bool MortonCode< T >::operator<=( const MortonCode& other ) const
	{
		return m_bits <= other.m_bits;
	}
	
	template <typename T>
	string MortonCode< T >::toString() const
	{
		stringstream ss;
		ss << "0x" << hex << m_bits;
		return ss.str();
	}
	
	template <typename T>
	string MortonCode< T >::getPathToRoot( bool simple ) const
	{
		stringstream ss;
		MortonCode< T > code = *this;
		
		if (simple)
		{
			while( code.getBits() != 1 )
			{
				ss << "0x" << hex << code.getBits() << dec << "->";
				code = *code.traverseUp();
			}
			ss << "0x" << hex << code.getBits() << dec << endl;
		}
		else
		{
			while( code.getBits() != 1 )
			{
				ss << code << dec << " -> ";
				code = *code.traverseUp();
			}
			ss << code << dec << endl;
		}
		
		return ss.str();
	}
	
	template <typename T>
	inline MortonCode< T > MortonCode< T >::getLvlFirst( const unsigned int& lvl )
	{
		MortonCode< T > code;
		code.build( 0, 0, 0, lvl );
		return code;
	}
		
	template <typename T>
	inline MortonCode< T > MortonCode< T >::getLvlLast( const unsigned int& lvl )
	{
		MortonCode< T > code;
		code.build( ( ( ( T )1 ) << ( 3 * lvl + 1 ) ) - 1 );
		return code;
	}
	
	template <typename Precision>
	ostream& operator<<( ostream& out, const MortonCode<Precision>& code )
	{
		unsigned int level = code.getLevel();
		vector< Precision > decoded = code.decode(level);
		out << "MortonCode: " << endl << "level = " << level << endl
			<< "coords = " << decoded << endl << code.getPathToRoot( true ) << endl;
		
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
	inline unsigned int MortonCode< unsigned int >::compact3(unsigned int x) const
	{
		x &= 0x09249249;
		x = (x ^ (x >>  2)) & 0x030c30c3;
		x = (x ^ (x >>  4)) & 0x0300f00f;
		x = (x ^ (x >>  8)) & 0xff0000ff;
		x = (x ^ (x >> 16)) & 0x000003ff;
		
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
	
	template<>
	inline uint MortonCode< uint >::maxLvl()
	{
		return 10;
	}
	
	template<>
	inline uint MortonCode< ulong >::maxLvl()
	{
		return 21;
	}
	
	template<>
	inline uint MortonCode< uint >::getLevel() const
	{
		return ( 31u - __builtin_clz( m_bits ) ) / 3u;
	}
	
	template<>
	inline uint MortonCode< ulong >::getLevel() const
	{
		return ( 63u - __builtin_clzl( m_bits ) ) / 3u;
	}
	
	/** "Spreads" coordinate bits to build Morton code. Applied bit-wise operations are explained here:
	 * http://stackoverflow.com/a/18528775/1042102 */
	/*template <>
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
	}*/
	
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
	
	//=================
	// Type sugar.
	//=================
	
	using ShallowMortonCode = MortonCode< unsigned int >;
	using MediumMortonCode = MortonCode< unsigned long >;
	//using DeepMortonCode = MortonCode< unsigned long long >;
	
	using ShallowMortonCodePtr = shared_ptr< ShallowMortonCode >;
	using MediumMortonCodePtr = shared_ptr< MediumMortonCode >;
	//using DeepMortonCodePtr = shared_ptr< DeepMortonCode >;
}

namespace std
{
	template<>
    struct hash< model::ShallowMortonCode >
    {
        size_t operator()( const model::ShallowMortonCode& k ) const
        {
			return hash< unsigned int >()( k.getBits() );
        }
    };
	
	template<>
    struct hash< model::MediumMortonCode >
    {
        size_t operator()( const model::MediumMortonCode& k ) const
        {
			return hash< unsigned long >()( k.getBits() );
        }
    };
}

#endif