#include <windows.h>

#include "framework/time.h"

namespace framework
{

	f64 Time::getTimeStampS()
	{
		LARGE_INTEGER freq;
		LARGE_INTEGER stamp;
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&stamp);
		return static_cast<f64>(stamp.QuadPart) / static_cast<f64>(freq.QuadPart);
	}

	f64 Time::getTimeStampMs()
	{
		LARGE_INTEGER freq;
		LARGE_INTEGER stamp;
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&stamp);
		return (static_cast<f64>(stamp.QuadPart) * 1000.0) / static_cast<f64>(freq.QuadPart);
	}
}