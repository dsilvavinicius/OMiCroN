#ifndef MORTON_CODE_H
#define MORTON_CODE_H

#include <vector>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <memory>
#include <glm/ext.hpp>

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
		void build(const T& x, const T& y, const T& z, const unsigned int& level);
		
		/** Use this method to inform the code. */
		void build(const T& codeBits);
		
		/** Decodes this morton code into an array of 3 coordinates. Use this when the level of code is known
		 * a priori. */
		vector<T> decode(const unsigned int& level) const;
		
		/** Decodes this morton code into an array of 3 coordinates. Use this when the level of the code is
		 * unknown (slower). */
		vector<T> decode() const;
		
		/** Computes the level of this code. */
		unsigned int getLevel() const;
		
		T getBits() const;
		
		shared_ptr< MortonCode< T > > traverseUp() const;
		vector< shared_ptr< MortonCode< T > >  > traverseDown() const;
		
		bool operator==(const MortonCode& other);
		bool operator!=(const MortonCode& other);
		
		/** Prints the nodes in the path from this node to the root node.
		 * @param simple indicates that the node should be printed in a simpler representation. */
		void printPathToRoot(ostream& out, bool simple);
		
		template <typename Precision>
		friend ostream& operator<<(ostream& out, const MortonCode<Precision>& dt);
	private:
		/** Spreads the bits to build Morton code. */
		T spread3(T x);
		
		/** Compacts the bits in code to decode it as a coordinate. */
		T compact3(T x) const;
		
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
	void MortonCode<T>::build(const T& codeBits) { m_bits = codeBits; }
	
	template <typename T>
	vector<T> MortonCode<T>::decode(const unsigned int& level) const
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
	vector<T> MortonCode<T>::decode() const
	{
		unsigned int level = getLevel();
		return decode(level);
	}
	
	template <typename T>
	unsigned int MortonCode<T>::getLevel() const
	{
		// Finds the MortonCode level.
		unsigned int numBits = sizeof(T) * 8;
		T bits = getBits();
		unsigned int level = 0;
		for (level = numBits / 3; level > 0; --level)
		{
			unsigned int shift = level * 3;
			if ((bits & ((T)1 << shift)) != 0) { break; }
		}
		
		return level;
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
	
	template <typename T>
	bool MortonCode< T >::operator==(const MortonCode< T >& other)
	{
		return m_bits == other.getBits();
	}
	
	template <typename T>
	bool MortonCode< T >::operator!=(const MortonCode< T >& other)
	{
		return !(m_bits == other.getBits());
	}
	
	template <typename T>
	void MortonCode< T >::printPathToRoot(ostream& out, bool simple)
		{
			MortonCodePtr< T > ancestor = traverseUp();
			out << "Path to root: ";
			
			if (simple)
			{
				out << "0x" << hex << getBits() << dec;
				do {
					out << " -> 0x" << hex << ancestor->getBits() << dec;
					ancestor = ancestor->traverseUp();
				} while (ancestor->getBits() != 0);	
			}
			else
			{
				out << *this;
				do {
					out << " -> " << hex << *ancestor << dec;
					ancestor = ancestor->traverseUp();
				} while (ancestor->getBits() != 0);	
			}
			cout << endl;
		}
	
	template <typename Precision>
	ostream& operator<<(ostream& out, const MortonCode<Precision>& code)
	{
		unsigned int level = code.getLevel();
		vector< Precision > decoded = code.decode(level);
		out << "MortonCode: " << endl << "level = " << level << endl
			<< "coords = " << decoded << endl
			<< "0x" << hex << code.m_bits << dec << endl
			<< "parent: 0x" << hex << code.traverseUp()->m_bits << dec << endl;
		return out;
	}
	
	//=================
	// Specializations.
	//=================
	
	/** "Spreads" coordinate bits to build Morton code. Applied bit-wise operations are explained here:
	 * http://stackoverflow.com/a/18528775/1042102 */
	template <>
	unsigned int MortonCode<unsigned int>::spread3(unsigned int x);
	
	template <>
	unsigned int MortonCode<unsigned int>::compact3(unsigned int x) const;
	
	/** "Spreads" coordinate bits to build Morton code. Applied bit-wise operations are explained here:
	 * http://stackoverflow.com/a/18528775/1042102 */
	template <>
	unsigned long MortonCode<unsigned long>::spread3(unsigned long x);
	
	template <>
	unsigned long MortonCode<unsigned long>::compact3(unsigned long x) const;
	
	/** "Spreads" coordinate bits to build Morton code. Applied bit-wise operations are explained here:
	 * http://stackoverflow.com/a/18528775/1042102 */
	template <>
	unsigned long long MortonCode<unsigned long long>::spread3(unsigned long long x);
	
	// TODO: Finish this.
	//template <>
	//unsigned long long * MortonCode<unsigned long long>::compact3(unsigned long long) const;
}

#endif