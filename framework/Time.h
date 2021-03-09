#pragma once

#include "framework/types.h"

namespace framework
{

	class Time
	{
	public:
		static f64 getTimeStampS();
		static f64 getTimeStampMs();
	};
}