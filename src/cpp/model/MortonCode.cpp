#include "MortonCode.h"

namespace model
{
	template <typename T>
	T MortonCode<T>::traverseUp(MortonCode<T> code)
	{
	}
	
	template <typename T>
	T MortonCode<T>::traverseDown(MortonCode<T> code)
	{
	}
	
	template<>
	MortonCode< unsigned int >::MortonCode(unsigned int x, unsigned int y, unsigned int z)
	{
		return spread3(x) | (spread3(y)>>1) | (spread3(z)>>2);
	}
	
	template<>
	MortonCode< unsigned int >::spread3(unsigned int x)
	{
		x=(0xF0000000&x) | ((0x0F000000&x)>>8) | (x>>16); // spread top 3 nibbles
		x=(0xC00C00C0&x) | ((0x30030030&x)>>4);
		x=(0x82082082&x) | ((0x41041041&x)>>2);
		return x;
	}
	
	template<>
	MortonCode< unsigned long >::MortonCode(unsigned long x, unsigned long y, unsigned long z)
	{
		return spread3(x) | (spread3(y)>>1) | (spread3(z)>>2);
	}
	
	template<>
	MortonCode< unsigned long >::spread3(unsigned long x)
	{
		x=(0xF0000000&x) | ((0x0F000000&x)>>8) | (x>>16); // spread top 3 nibbles
		x=(0xC00C00C0&x) | ((0x30030030&x)>>4);
		x=(0x82082082&x) | ((0x41041041&x)>>2);
		return x;
	}
	
	template<>
	MortonCode< unsigned long long >::MortonCode(unsigned long long x, unsigned long long y,
												 unsigned long long z)
	{
		return spread3(x) | (spread3(y)>>1) | (spread3(z)>>2);
	}
	
	template<>
	MortonCode< unsigned long long >::spread3(unsigned long long x)
	{
		x=(0xF0000000&x) | ((0x0F000000&x)>>8) | (x>>16); // spread top 3 nibbles
		x=(0xC00C00C0&x) | ((0x30030030&x)>>4);
		x=(0x82082082&x) | ((0x41041041&x)>>2);
		return x;
	}
}