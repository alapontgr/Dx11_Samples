#pragma once 

#include "framework/Types.h"

namespace framework
{

	class Hash
	{
	public:

		//MurmurHash2
		static u64 compute(const void* buffer, u64 size, u64 seed = 123);

		static u64 compute(const String& str, u64 seed = 123);
	};
}