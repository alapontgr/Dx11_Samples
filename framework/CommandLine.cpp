#include "framework/Framework.h"

#include <sstream>
#include <iostream>

namespace framework
{

	String CommandLine::ms_commandLineArgs = "";
	UMap <u64, String> CommandLine::ms_arguments;

	void CommandLine::init(const String& args)
	{
		ms_commandLineArgs = args;

		// Tokenize
		ms_arguments.clear();
		if (args.size() > 0)
		{
			String token;
			String payload = "";
			std::istringstream tokenStream(ms_commandLineArgs);
			u64 hash = 0;
			while (std::getline(tokenStream, token, ' '))
			{
				if (token[0] == '-')
				{
					if (hash != 0 && payload.size() != 0)
					{
						ms_arguments[hash] = payload;
						payload = "";
					}
					hash = Hash::compute(token);
				}
				else
				{
					if (payload.size() > 0)
					{
						payload += " " + token;
					}
					else
					{
						payload = token;
					}
				}
			}

			if (hash != 0 && payload.size() != 0)
			{
				ms_arguments[hash] = payload;
			}
		}
	}

	void CommandLine::init(char** args, s32 argc)
	{
		s32 pairCount = (argc - 1) / 2;
		s32 argIdx = 1;
		for (s32 i = 0; i < pairCount; i++)
		{
			const char* key = args[argIdx++];
			const char* value = args[argIdx++];
			ms_arguments[Hash::compute(key, strlen(key))] = String(value);
		}
	}

	String CommandLine::getArg(const u64 argHash)
	{
		auto it = ms_arguments.find(argHash);
		if (it != ms_arguments.end())
		{
			return it->second;
		}
		return "";
	}

}

////////////////////////////////////////////////////////////////////////////////