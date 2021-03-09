#pragma once

#include "framework/Types.h"

namespace framework
{

	class Paths
	{
	public:

		static void init();

		// Convert relative path to absolute path in the working dir. Return <workingDir>/<relPath>
		static String getAssetPath(const String& relPath);
		static String getAssetPath(const char* relPath);

		static String getWorkingDir()
		{
			return ms_workingDir;
		}

	private:

		// Commandline: "--workDir <path>"
		static String ms_workingDir;
	};
}