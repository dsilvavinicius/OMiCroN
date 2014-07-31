#ifndef MORTON_CODE_H
#define MORTON_CODE_H

#include <vector>
#include <stdexcept>
#include <iostream>
#include <sstream>

using namespace std;

namespace model
{
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
		void build(const T& x, const T& y, const T& z, const unsigned int& level);
		
		/** Use this method to inform the code. */
		void build(const T& codeBits);
		
		T getBits() const;
		
		MortonCode< T > traverseUp() const;
		vector< MortonCode < T > > traverseDown() const;
		T spread3(T x);
	private:
		T m_bits;
	};
	
	/** "Spreads" coordinate bits to build Morton code. Applied bit-wise operations are explained here:
	 * http://stackoverflow.com/a/18528775/1042102 */
	template <>
	unsigned int MortonCode<unsigned int>::spread3(unsigned int x);
	
	/** "Spreads" coordinate bits to build Morton code. Applied bit-wise operations are explained here:
	 * http://stackoverflow.com/a/18528775/1042102 */
	template <>
	unsigned long MortonCode<unsigned long>::spread3(unsigned long x);
	
	/** "Spreads" coordinate bits to build Morton code. Applied bit-wise operations are explained here:
	 * http://stackoverflow.com/a/18528775/1042102 */
	template <>
	unsigned long long MortonCode<unsigned long long>::spread3(unsigned long long x);
	
	using ShallowMortonCode = MortonCode< unsigned int >;
	using MediumMortonCode = MortonCode< unsigned long >;
	using DeepMortonCode = MortonCode< unsigned long long >;
	
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
	void MortonCode<T>::build(const T& codeBits) { m_bits = codeBits; }
	
	template <typename T>
	T MortonCode<T>::getBits() const{ return m_bits; }
	
	template <typename T>
	MortonCode<T> MortonCode<T>::traverseUp() const
	{
		T bits = getBits() >> 3;
		MortonCode<T> parentMorton;
		parentMorton.build(bits);
		return parentMorton;
	}
	
	template <typename T>
	vector< MortonCode< T > >  MortonCode<T>::traverseDown() const
	{
		vector< MortonCode<T> > children(8);
		T bits = getBits();
		T shifted = bits << 3;
		
		// Checks for overflow.
		if(shifted < bits)
		{
			stringstream ss;
			ss << "Overflow detected while traversing down morton code " << hex << bits;
			throw logic_error(ss.str());
		}
		
		for (int i = 0; i < 8; ++i)
		{
			MortonCode<T> child;
			child.build(shifted | i);
			children[i] = child;
		}
		
		return children;
	}
	
	template <>
	unsigned int MortonCode< unsigned int >::spread3(unsigned int x)
	{
		x &= 0x3ff;
		x = (x | x << 16) & 0x30000ff;
		x = (x | x << 8) & 0x300f00f;
		x = (x | x << 4) & 0x30c30c3;
		x = (x | x << 2) & 0x9249249;
		
		return x;
	}
	
	template <>
	unsigned long MortonCode< unsigned long >::spread3(unsigned long x)
	{
		x &= 0x1fffffL;
		x = (x | x << 32) & 0x1f00000000ffffL;
		x = (x | x << 16) & 0x1f0000ff0000ffL;
		x = (x | x << 8) & 0x100f00f00f00f00fL;
		x = (x | x << 4) & 0x10c30c30c30c30c3L;
		x = (x | x << 2) & 0x1249249249249249L;
		
		return x;
	}
	
	template <>
	unsigned long long MortonCode< unsigned long long >::spread3(unsigned long long x)
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
}

#endif