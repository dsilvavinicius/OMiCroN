#ifndef MORTON_CODE_H
#define MORTON_CODE_H

#include <vector>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <memory>

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
		
		/** Decodes this morton code into an array of 3 coordinates. */
		vector<T> decode(const unsigned int& level);
		
		T getBits() const;
		
		shared_ptr< MortonCode< T > > traverseUp() const;
		vector< shared_ptr< MortonCode< T > >  > traverseDown() const;
		
		template <typename Precision>
		friend ostream& operator<<(ostream& os, const MortonCode<Precision>& dt);
	private:
		/** Spreads the bits to build Morton code. */
		T spread3(T x);
		
		/** Compacts the bits in code to decode it as a coordinate. */
		T compact3(T x);
		
		T m_bits;
	};
	
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
	void MortonCode<T>::build(const T& codeBits) { m_bits = codeBits; }
	
	template <typename T>
	vector<T> MortonCode<T>::decode(const unsigned int& level)
	{
		vector<T> coords(3);
		
		//T bits = m_bits & ((1 << (level * 3)) - 1);
		
		coords[0] = compact3(m_bits);
		coords[1] = compact3(m_bits >> 1);
		coords[2] = compact3(m_bits >> 2);
		
		return coords;
	}
	
	template <typename T>
	T MortonCode<T>::getBits() const{ return m_bits; }
	
	template <typename T>
	MortonCodePtr<T> MortonCode<T>::traverseUp() const
	{
		T bits = getBits() >> 3;
		MortonCodePtr<T> parentMorton = make_shared< MortonCode<T> >();
		parentMorton->build(bits);
		return parentMorton;
	}
	
	template <typename T>
	vector< MortonCodePtr< T > >  MortonCode<T>::traverseDown() const
	{
		vector< MortonCodePtr<T> > children(8);
		T bits = getBits();
		T shifted = bits << 3;
		
		// TODO: Code this overflow check as an assert.
		// Checks for overflow.
		if(shifted < bits)
		{
			stringstream ss;
			ss << "Overflow detected while traversing down morton code " << hex << bits;
			throw logic_error(ss.str());
		}
		
		for (int i = 0; i < 8; ++i)
		{
			MortonCodePtr<T> child = make_shared< MortonCode<T> >();
			child->build(shifted | i);
			children[i] = child;
		}
		
		return children;
	}
	
	template <typename Precision>
	ostream& operator<<(ostream& os, const MortonCode<Precision>& code)
	{
		os << "MortonCode: 0x" << hex << code.m_bits << endl << endl;
		return os;
	}
	
	//=================
	// Specializations.
	//=================
	
	/** "Spreads" coordinate bits to build Morton code. Applied bit-wise operations are explained here:
	 * http://stackoverflow.com/a/18528775/1042102 */
	template <>
	unsigned int MortonCode<unsigned int>::spread3(unsigned int x);
	
	template <>
	unsigned int MortonCode<unsigned int>::compact3(unsigned int x);
	
	/** "Spreads" coordinate bits to build Morton code. Applied bit-wise operations are explained here:
	 * http://stackoverflow.com/a/18528775/1042102 */
	template <>
	unsigned long MortonCode<unsigned long>::spread3(unsigned long x);
	
	template <>
	unsigned long MortonCode<unsigned long>::compact3(unsigned long x);
	
	/** "Spreads" coordinate bits to build Morton code. Applied bit-wise operations are explained here:
	 * http://stackoverflow.com/a/18528775/1042102 */
	template <>
	unsigned long long MortonCode<unsigned long long>::spread3(unsigned long long x);
	
	// TODO: Finish this.
	//template <>
	//unsigned long long * MortonCode<unsigned long long>::compact3(unsigned long long);
}

#endif