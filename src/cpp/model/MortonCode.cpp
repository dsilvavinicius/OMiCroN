#include "MortonCode.h"

namespace model
{
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