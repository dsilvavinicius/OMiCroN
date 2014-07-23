#ifndef MORTON_CODE_H
#define MORTON_CODE_H

namespace model
{
	/** Morton code generic type. */
	template <typename T>
	struct MortonCode;
	
	/** 32 bits Morton code. Octrees can reach 10 levels max (3 bits each level). */
	template <> struct MortonCode<int>
	{
		typedef int type;
	};
	
	/** 64 bits Morton code. Octrees can reach 21 levels max (3 bits each level). */
	template <> struct MortonCode<long>
	{
		typedef long type;
	};
}

#endif