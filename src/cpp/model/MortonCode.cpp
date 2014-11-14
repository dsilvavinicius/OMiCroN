#include "MortonCode.h"

namespace model
{
	/*template <>
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
}