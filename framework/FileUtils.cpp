#include "framework/Framework.h"

namespace framework
{

	HANDLE FileUtils::openFileForRead(const char* fileAbsPath)
	{
		HANDLE file = CreateFileA(
			fileAbsPath,
			GENERIC_READ,
			0, NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL);

		if (file == INVALID_HANDLE_VALUE)
		{
			printf("Attempt to open file '%s' failed with error: %d", fileAbsPath, GetLastError());
		}
		return file;
	}

	u32 FileUtils::getFileSize(const HANDLE file)
	{
		size_t size(0);
		if (file != INVALID_HANDLE_VALUE)
		{
			LARGE_INTEGER ulSize;
			GetFileSizeEx(file, &ulSize);
			size = ulSize.QuadPart;
		}
		return static_cast<u32>(size);
	}

	u32 FileUtils::readBytes(const HANDLE file, u32 toRead, void* outBuffer)
	{
		DWORD outRead(0);
		if (file != INVALID_HANDLE_VALUE)
		{
			if (!ReadFile(file,
				outBuffer,
				toRead,
				&outRead,
				NULL))
			{
				printf("Attempt to read file data failed with error: %d", GetLastError());
			}
		}
		return outRead;
	}

	void FileUtils::closeFile(const HANDLE file)
	{
		if (file != INVALID_HANDLE_VALUE)
		{
			CloseHandle(file);
		}
	}

	// ----------------------------------------------------------------------

	UniquePtr<char[]> FileUtils::loadFileContent(const char* fileRelPath, u32& outFileSize)
	{
		HANDLE file = openFileForRead(fileRelPath);
		if (file != INVALID_HANDLE_VALUE)
		{
			outFileSize = getFileSize(file);
			u32 sizeWithNullTerminator = outFileSize + 1;
			UniquePtr<char[]> rawData = UniquePtr<char[]>(new char[sizeWithNullTerminator]);
			memset(rawData.get(), 0, sizeWithNullTerminator);
			u32 bytesRead = readBytes(file, outFileSize, rawData.get());
			VERIFY(bytesRead == outFileSize, "Not the entire file was loaded");
			closeFile(file);
			return std::move(rawData);
		}
		printf("Failed to load file: %s\n", fileRelPath);
		return nullptr;
	}

	UniquePtr<char[]> FileUtils::loadFileContent(const char* fileRelPath)
	{
		u32 fileSize = 0;
		return loadFileContent(fileRelPath, fileSize);
	}

	bool FileUtils::doesFileExist(const char* fileRelPath)
	{
		WIN32_FIND_DATAA fd = { 0 };
		HANDLE hFound = FindFirstFileA(fileRelPath, &fd);
		bool result = hFound != INVALID_HANDLE_VALUE;
		FindClose(hFound);
		return result;
	}

}