#ifndef MORTON_CODE_H
#define MORTON_CODE_H

namespace model
{
	/** Morton code generic type. */
	template <typename T>
	class MortonCode
	{
	public:
		static T traverseUp(MortonCode<T> code);
		static T traverseDown(MortonCode<T> code);
		
		T m_bits;
	};
	
	/** 32 bits Morton code. Octrees can reach 10 levels max. */
	template <>
	class MortonCode<unsigned int>
	{
	public:
		MortonCode(unsigned int x, unsigned int y, unsigned int z);
		
	private:
		/* Takes a value and "spreads" the HIGH bits to lower slots to seperate them.
		ie, bit 31 stays at bit 31, bit 30 goes to bit 28, bit 29 goes to bit 25, etc.
		Anything below bit 21 just disappears. Useful for interleaving values
		for Morton codes. */
		unsigned int spread3(unsigned int x);
	};
	
	/** 64 bits Morton code. Octrees can reach 21 levels max. */
	template <>
	class MortonCode<unsigned long>
	{
	public:
		MortonCode(unsigned long x, unsigned long y, unsigned long z);
		
	private:
		/* Takes a value and "spreads" the HIGH bits to lower slots to seperate them.
		ie, bit 31 stays at bit 31, bit 30 goes to bit 28, bit 29 goes to bit 25, etc.
		Anything below bit 21 just disappears. Useful for interleaving values
		for Morton codes. */
		unsigned long spread3(unsigned long x);
	};
	
	/** 128 bits Morton code. Octrees can reach 42 levels max. */
	template <>
	class MortonCode<unsigned long long>
	{
	public:
		MortonCode(unsigned long long x, unsigned long long y, unsigned long long z);
		
	private:
		/* Takes a value and "spreads" the HIGH bits to lower slots to seperate them.
		ie, bit 31 stays at bit 31, bit 30 goes to bit 28, bit 29 goes to bit 25, etc.
		Anything below bit 21 just disappears. Useful for interleaving values
		for Morton codes. */
		unsigned long long spread3(unsigned long long x);
	};
}

#endif