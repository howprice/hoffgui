#pragma once

#include "Core/Helpers.h"

static const unsigned int kMaxPath = 256;

class FileSystem
{
public:
	NON_INSTANTIABLE_STATIC_CLASS(FileSystem);

	// Must be called after initialising SDL
	static bool Init();

	static void Shutdown();

	static bool Exists(const char* path);
	static unsigned int GetFileSize(const char* path);
	static void FilenameWithExtensionFromPath(const char* path, char* fileNameBuffer, size_t bufferSize);
	static void FilenameWithoutExtensionFromPath(const char* path, char* fileNameBuffer, size_t bufferSize);
	static void ExtensionFromPath(const char* path, char* extensionBuffer, size_t bufferSize);

	// it does not matter if directory has a trailing slash or not
	static void MakePath(char* path, size_t bufferSize, const char* directory, const char* filename);

	// The directory where the application was run from.
	// Always has a trailing slash.
	static const char* GetApplicationDirectory();

	// Safe place to store user data.
	// Always has a trailing slash.
	static const char* GetUserPrefDirectory();

	static bool Copy(const char* from, const char* to);

	// Can't call this CreateDirectory because it conflicts with Windows.h macro
	static bool MakeDir(const char* path);
};
