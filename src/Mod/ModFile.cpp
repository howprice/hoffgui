#include "ModFile.h"

#include "Core/StringHelpers.h"
#include "Core/hp_assert.h"
#include "Core/Log.h"
#include "Core/FileSystem.h"

static char s_path[kMaxPath];
static uint8_t* s_pData;
static unsigned int s_bufferSizeBytes;

static bool readFileIntoBuffer(const char* path)
{
	HP_ASSERT(path && path[0]);

	FILE* pFile = fopen(path, "rb");
	if (!pFile)
	{
		LOG_ERROR("Failed to open file for read: %s\n", path);
		return false;
	}

	LOG_TRACE("Opened file: %s\n", path);
	fseek(pFile, 0, SEEK_END);
	const unsigned int fileSizeBytes = ftell(pFile);
//	LOG_INFO("File size: %u (0x%X) bytes\n", fileSizeBytes, fileSizeBytes);
	fseek(pFile, 0, SEEK_SET);

	if (s_pData != nullptr)
		delete[] s_pData;
	s_pData = new uint8_t[fileSizeBytes];
	s_bufferSizeBytes = fileSizeBytes;

	size_t numElementsRead = fread(s_pData, 1, fileSizeBytes, pFile);
	fclose(pFile);
	pFile = nullptr;

	if (numElementsRead != fileSizeBytes)
	{
		LOG_ERROR("File read failed.\n");
		return false;
	}

	SafeStrcpy(s_path, sizeof(s_path), path);

	return true;
}

static bool saveBufferToFile(const char* path)
{
	HP_ASSERT(path && path[0]);

	if (s_bufferSizeBytes == 0)
	{
		LOG_ERROR("No data to save.\n");
		return false;
	}

	FILE* pFile = fopen(path, "wb");
	if (!pFile)
	{
		LOG_ERROR("Failed to open file for write: %s\n", path);
		return false;
	}

	LOG_TRACE("Opened file for write: %s\n", path);
	const size_t numElementsWritten = fwrite(s_pData, 1, s_bufferSizeBytes, pFile);
	fclose(pFile);
	pFile = nullptr;

	if (numElementsWritten != s_bufferSizeBytes)
	{
		LOG_ERROR("File write failed.\n");
		return false;
	}

	SafeStrcpy(s_path, sizeof(s_path), path);
	return true;
}

static void newFile()
{
	if (s_pData != nullptr)
		delete[] s_pData;
	s_bufferSizeBytes = 16;
	s_pData = new uint8_t[s_bufferSizeBytes];
	for (unsigned int i = 0; i < s_bufferSizeBytes; i++)
	{
		s_pData[i] = (uint8_t)i;
	}

	s_path[0] = '\0';
}

bool ModFile::Load(const char* path)
{
	if (IsLoaded())
		Free();

	return readFileIntoBuffer(path);
}

void ModFile::Free()
{
	delete[] s_pData; // fine for null
	s_pData = nullptr;

	s_bufferSizeBytes = 0;
}

bool ModFile::IsLoaded()
{
	return s_pData != nullptr;
}

const char* ModFile::GetPath()
{
	return s_path;
}

const uint8_t* ModFile::GetData()
{
	return s_pData;
}

unsigned int ModFile::GetDataSizeBytes()
{
	return s_bufferSizeBytes;
}

bool ModFile::Save()
{
	HP_ASSERT(s_pData != nullptr);
	HP_ASSERT(s_path[0] != '\0');
	return saveBufferToFile(s_path);
}

bool ModFile::SaveAs(const char* path)
{
	return saveBufferToFile(path);
}

void ModFile::New()
{
	Free();
	newFile();
}
