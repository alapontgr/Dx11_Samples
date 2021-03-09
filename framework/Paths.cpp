////////////////////////////////////////////////////////////////////////////////
//
//	Author: Sergio Alapont Granero (seralgrainf@gmail.com)
//	Date: 	2021/01/13
//	File: 	GFDEBUG.cpp
//
//	Copyright (c) 2021 (See README.md)
//
////////////////////////////////////////////////////////////////////////////////
// Includes

#include "framework/Framework.h"

////////////////////////////////////////////////////////////////////////////////

namespace framework
{

	String Paths::ms_workingDir = "./";

	static String s_workingDirArg = "--workDir";

	void Paths::init()
	{
		String workDir = CommandLine::getArg(Hash::compute(s_workingDirArg.data(), s_workingDirArg.size()));
		ms_workingDir = "./";
		if (workDir.size() > 0)
		{
			if (workDir[workDir.size() - 1] != '/' && workDir[workDir.size() - 1] != '\\')
			{
				workDir += "/";
			}
			ms_workingDir = workDir;
			std::replace(ms_workingDir.begin(), ms_workingDir.end(), '\\', '/');
		}
	}

	String Paths::getAssetPath(const String& relPath)
	{
		static const char* s_prefix = "./";
		size_t pos = relPath.find(s_prefix);
		String path = relPath;
		if (pos != String::npos)
		{
			path = relPath.substr(pos + 2);
		}
		String absPath = ms_workingDir + path;
		std::replace(absPath.begin(), absPath.end(), '\\', '/');
		return absPath;
	}

	String Paths::getAssetPath(const char* relPath)
	{
		return getAssetPath(String(relPath));
	}

}
////////////////////////////////////////////////////////////////////////////////