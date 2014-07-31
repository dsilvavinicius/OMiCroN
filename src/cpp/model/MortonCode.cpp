#include "MortonCode.h"

namespace model
{
	template <typename T>
	void MortonCodeBase<T>::build(const T& x, const T& y, const T& z, const unsigned int& level)
	{
		m_bits = spread3(x) | (spread3(y) << 1) | (spread3(z) << 2);
		
		T twoPowLevel = 1 << level * 3;
		// Discards all bits in positions greater than level * 3.
		m_bits &= (twoPowLevel) - 1;
		// Put the 1 sufix.
		m_bits |= twoPowLevel;
	}
	
	template <typename T>
	void MortonCodeBase<T>::build(const T& codeBits) { m_bits = codeBits; }
	
	template <typename T>
	T MortonCodeBase<T>::getBits() { return m_bits; }
	
	template <typename T>
	MortonCodeBase<T> MortonCodeBase<T>::traverseUp(const MortonCodeBase<T>& code) const
	{
		T bits = code.getBits() >> 3;
		return MortonCodeBase<T>(bits);
	}
	
	template <typename T>
	vector< MortonCodeBase< T > >  MortonCodeBase<T>::traverseDown(const MortonCodeBase<T>& code) const
	{
		vector< MortonCodeBase<T> > children(8);
		
		for (int i = 0; i < 8; ++i)
		{
			children[i] = MortonCodeBase<T>((code << 3) | i);
		}
	}
	
	/*template<>
	void MortonCode< unsigned int >::build(const unsigned int& x, const unsigned int& y, const unsigned int& z,
										   const unsigned int& level)
	{
		m_bits = spread3(x) | (spread3(y) << 1) | (spread3(z) << 2);
		
		unsigned int twoPowLevel = 1 << level * 3;
		// Discards all bits in positions greater than level * 3.
		m_bits &= (twoPowLevel) - 1;
		// Put the 1 sufix.
		m_bits |= twoPowLevel;
	}*/
	
	template<>
	unsigned int MortonCode< unsigned int >::spread3(unsigned int x)
	{
		x &= 0x3ff;
		x = (x | x << 16) & 0x30000ff;
		x = (x | x << 8) & 0x300f00f;
		x = (x | x << 4) & 0x30c30c3;
		x = (x | x << 2) & 0x9249249;
		
		return x;
	}
	
	/*template<>
	void MortonCode< unsigned long >::build(const unsigned long& x, const unsigned long& y, const unsigned long& z,
											const unsigned int& level)
	{
		m_bits = spread3(x) | (spread3(y) << 1) | (spread3(z) << 2);
		
		unsigned long twoPowLevel = 1 << level * 3;
		// Discards all bits in positions greater than level * 3.
		m_bits &= (twoPowLevel - 1);
		// Put the 1 sufix.
		m_bits |= twoPowLevel;
	}*/
	
	template<>
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
	
	/*template<>
	void MortonCode< unsigned long long >::build(const unsigned long long& x, const unsigned long long& y,
												 const unsigned long long& z, const unsigned int& level)
	{
		m_bits = spread3(x) | (spread3(y) << 1) | (spread3(z) << 2);
		
		unsigned long long twoPowLevel = 1 << level * 3;
		// Discards all bits in positions greater than level * 3.
		m_bits &= (twoPowLevel) - 1;
		// Put the 1 sufix.
		m_bits |= twoPowLevel;
	}*/
	
	template<>
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