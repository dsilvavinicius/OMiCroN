#ifndef MORTON_CODE_H
#define MORTON_CODE_H

#include <vector>

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
		/** Use this constructor to just inform the code bits. */
		MortonCode(T codeBits);
		/** Use this constructor to calculate code from position. */
		MortonCode(T x, T y, T z, unsigned int level);
		T getBits();
		
		static MortonCode< T > traverseUp(const MortonCode<T>& code);
		static vector< MortonCode < T > > traverseDown(const MortonCode<T>& code);
	private:
		T spread3(T);
	};
	
	/** 32 bits Morton code. Octrees can reach 10 levels max. */
	template <>
	class MortonCode<unsigned int>
	{
	public:
		MortonCode(unsigned int x, unsigned int y, unsigned int z, unsigned int level);
		
	private:
		/** Takes a value and "spreads" the lower 10 bits, seperating them in slots of 3 bits.
		 * Applied bit-wise operations are explained here:
		 * http://stackoverflow.com/a/18528775/1042102. */
		unsigned int spread3(unsigned int x);
		
		unsigned int m_bits;
	};
	
	/** 64 bits Morton code. Octrees can reach 21 levels max. */
	template <>
	class MortonCode<unsigned long>
	{
	public:
		MortonCode(unsigned long x, unsigned long y, unsigned long z, unsigned int level);
		
	private:
		/** Takes a value and "spreads" the lower 21 bits, seperating them in slots of 3 bits.
		 * Applied bit-wise operations are explained here:
		 * http://stackoverflow.com/a/18528775/1042102. */
		unsigned long spread3(unsigned long x);
		
		unsigned m_bits;
	};
	
	/** 128 bits Morton code. Octrees can reach 42 levels max.*/
	template <>
	class MortonCode<unsigned long long>
	{
	public:
		MortonCode(unsigned long long x, unsigned long long y, unsigned long long z, unsigned int level);
		
	private:
		/** Takes a value and "spreads" the lower 42 bits, seperating them in slots of 3 bits.
		 * Applied bit-wise operations are explained here:
		 * http://stackoverflow.com/a/18528775/1042102. */
		unsigned long long spread3(unsigned long long x);
		
		unsigned m_bits;
	};
	
	using ShallowMortonCode = MortonCode< unsigned int >;
	using MediumMortonCode = MortonCode< unsigned long >;
	using DeepMortonCode = MortonCode< unsigned long long >;
}

#endif