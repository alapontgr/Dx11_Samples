////////////////////////////////////////////////////////////////////////////////
//
//	Author: Sergio Alapont Granero (seralgrainf@gmail.com)
//	Date: 	2020/01/10
//	File: 	GfCommandLine.h
//
//	Copyright (c) 2020 (See README.md)
//
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "framework/Types.h"

////////////////////////////////////////////////////////////////////////////////

namespace framework
{

	class CommandLine
	{
	public:
		static void init(const String& args);
		static void init(char** args, s32 argc);

		static String getArg(const u64 argHash);

	private:

		static String ms_commandLineArgs;
		static UMap<u64, String> ms_arguments;
	};
}

////////////////////////////////////////////////////////////////////////////////