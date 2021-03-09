#pragma once

namespace framework
{

	class FileUtils
	{
	public:

		static UniquePtr<char[]> loadFileContent(const char* fileRelPath, u32& outFileSize);
		static UniquePtr<char[]> loadFileContent(const char* fileRelPath);

		static bool doesFileExist(const char* fileRelPath);

		static HANDLE openFileForRead(const char* fileAbsPath);
		static u32 getFileSize(const HANDLE file);
		static u32 readBytes(const HANDLE file, u32 toRead, void* outBuffer);
		static void closeFile(const HANDLE file);
	};
}