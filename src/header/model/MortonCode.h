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
	class MortonCodeBase
	{
	public:
		/** Use this method to calculate code from position. */
		void build(const T& x, const T& y, const T& z, const unsigned int& level);
		
		/** Use this method to inform the code. */
		void build(const T& codeBits);
		
		T getBits();
		
		MortonCodeBase< T > traverseUp(const MortonCodeBase<T>& code) const;
		vector< MortonCodeBase < T > > traverseDown(const MortonCodeBase<T>& code) const;
	protected:
		T spread3(T);
	private:
		T m_bits;
	};
	
	template <typename T>
	class MortonCode : public MortonCodeBase<T> {};
	
	template <>
	class MortonCode<unsigned int> : public MortonCodeBase<unsigned int>
	{
	public:
		/** Builds 32 bits Morton code from coordinates and level. Octrees can reach 10 levels max. */
		/*void build(const unsigned int& x, const unsigned int& y, const unsigned int& z,
											 const unsigned int& level);*/
	private:
		/** "Spreads" coordinate bits to build Morton code. Applied bit-wise operations are explained here:
		 * http://stackoverflow.com/a/18528775/1042102 */
		unsigned int spread3(unsigned int x);
	};
	
	template <>
	class MortonCode<unsigned long> : public MortonCodeBase<unsigned long>
	{
	public:
		/** Builds 64 bits Morton code from coordinates and level. Octrees can reach 21 levels max. */
		/*void build(const unsigned long& x, const unsigned long& y, const unsigned long& z,
											  const unsigned int& level);*/
	private:
		/** "Spreads" coordinate bits to build Morton code. Applied bit-wise operations are explained here:
		 * http://stackoverflow.com/a/18528775/1042102 */
		unsigned long spread3(unsigned long x);
	};
	
	template <>
	class MortonCode<unsigned long long> : public MortonCodeBase<unsigned long long>
	{
	public:
		/** Builds 128 bits Morton code from coordinates and level. Octrees can reach 42 levels max. The compiler can
		 * complain about unsigned long long size. In this case don't use this type. */
		/*void build(const unsigned long long& x, const unsigned long long& y,
				   const unsigned long long& z, const unsigned int& level);*/
	private:
		/** "Spreads" coordinate bits to build Morton code. Applied bit-wise operations are explained here:
		 * http://stackoverflow.com/a/18528775/1042102 */
		unsigned long long spread3(unsigned long long x);
	};
	
	using ShallowMortonCode = MortonCode< unsigned int >;
	using MediumMortonCode = MortonCode< unsigned long >;
	using DeepMortonCode = MortonCode< unsigned long long >;
}

#endif