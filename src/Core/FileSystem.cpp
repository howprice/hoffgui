#include "FileSystem.h"

#include "Core/Log.h"
#include "Core/hp_assert.h"
#include "Core/StringHelpers.h"

#include "SDL.h" // SDL_GetBasePath

#include <sys/stat.h>

#ifdef _MSC_VER
#include <Windows.h> // GetModuleFileNameA, _splitpath_s, _makepath_s
#else
#include <string.h>
#include <libgen.h> // dirname
#endif

#include <filesystem>

// The directory where the application was run from.
// Windows: "C:\dev\howprice\hoffgui\build\Debug\"
// Linux: "/home/howard/GitHub/howprice/hoffgui/build/"
// Mac: "/Users/Howard/Documents/GitHub/howprice/hoffgui/build/hoffgui.app/Contents/Resources/"
// Always has a trailing slash
static char* s_pApplicationDirectory;

static const char* s_pOrgName = "TTE";
static const char* s_pAppName = "hoffgui";

// Safe place to store user data
// Windows: "C:\Users\Howard\AppData\Roaming\TTE\hoffgui\"
// Linux: "/home/howard/.local/share/TTE/hoffgui/"
// Mac: "/Users/Howard/Library/Application Support/TTE/hoffgui/
// Always has a trailing slash
static const char* s_pPrefPath;

bool FileSystem::Init()
{
	s_pApplicationDirectory = SDL_GetBasePath();
	if (!s_pApplicationDirectory)
	{
		LOG_ERROR("SDL_GetBasePath failed: %s\n", SDL_GetError());
		return false;
	}

	LOG_TRACE("Application directory: %s\n", s_pApplicationDirectory);

	s_pPrefPath = SDL_GetPrefPath(s_pOrgName, s_pAppName);
	if (!s_pPrefPath)
	{
		LOG_ERROR("SDL_GetPrefPath failed: %s\n", SDL_GetError());
		return false;
	}
	LOG_TRACE("PrefPath: %s\n", s_pPrefPath);

	return true;
}

void FileSystem::Shutdown()
{
	if (s_pApplicationDirectory)
	{
		SDL_free(s_pApplicationDirectory);
		s_pApplicationDirectory = nullptr;
	}

	if (s_pPrefPath)
	{
		SDL_free((char*)s_pPrefPath);
		s_pPrefPath = nullptr;
	}
}

bool FileSystem::Exists(const char* path)
{
	HP_ASSERT(path && path[0]);

	std::error_code ec;

#ifdef _MSC_VER
	// std::filesystem uses wchar on Windows.
	// SDL uses UTF-8, so we need to convert to wide char.
	WCHAR wpath[kMaxPath];
	::MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, kMaxPath);
	bool exists = std::filesystem::exists(wpath, ec);
#else
	bool exists = std::filesystem::exists(path, ec);
#endif
	if (ec)
	{
		LOG_ERROR("std::filesystem::exists(%s) failed : %s", path, ec.message().c_str());
		return false;
	}

	return exists;
}

unsigned int FileSystem::GetFileSize(const char* path)
{
	HP_ASSERT(path && path[0]);
	FILE* pFile = fopen(path, "rb");
	if (!pFile)
	{
		LOG_ERROR("Failed to open file for read: %s\n", path);
		return 0;
	}

	fseek(pFile, 0, SEEK_END);
	const unsigned int fileSizeBytes = ftell(pFile);
	fclose(pFile);
	return fileSizeBytes;
}

void FileSystem::FilenameWithExtensionFromPath(const char* path, char* fileNameBuffer, size_t bufferSize)
{
	HP_ASSERT(path && path[0]);
	HP_ASSERT(fileNameBuffer);

#ifdef WIN32

//	char drive[256];
//	char dir[256];
	char filename[256];
	char ext[256];
	_splitpath_s(
		path,
		/*drive*/NULL, /*sizeof(drive)*/0,        // e.g. "C:"
		/*dir*/NULL, /*sizeof(dir)*/0,            // e.g. "\dev\howprice\hoffgui\build\Debug\"
		filename, sizeof(filename),  // filename e.g. Calculator
		ext, sizeof(ext));           // e.g. ".cnf" or ".asm". Maybe even ".exe"

	_makepath_s(fileNameBuffer, bufferSize, /*drive*/NULL, /*dir*/NULL, filename, ext);
#else
	// basename may be destructive, so make a copy
	char pathCopy[kMaxPath];
	SafeStrcpy(pathCopy, sizeof(pathCopy), path);
	const char* filename = basename(pathCopy);
	SafeStrcpy(fileNameBuffer, bufferSize, filename);
#endif
}

void FileSystem::FilenameWithoutExtensionFromPath(const char* path, char* fileNameBuffer, size_t bufferSize)
{
	HP_ASSERT(path && path[0]);
	HP_ASSERT(fileNameBuffer);

#ifdef WIN32

	_splitpath_s(
		path,
		/*drive*/NULL, /*sizeof(drive)*/0,  // e.g. "C:"
		/*dir*/NULL, /*sizeof(dir)*/0,      // e.g. "\dev\howprice\hoffgui\build\Debug\"
		fileNameBuffer, bufferSize,         // e.g. Calculator
		/*ext*/NULL, /*sizeof(ext)*/0);          // e.g. ".cnf" or ".asm". Maybe even ".exe"
#else
	// basename may be destructive, so make a copy
	char pathCopy[kMaxPath];
	SafeStrcpy(pathCopy, sizeof(pathCopy), path);
	const char* filename = basename(pathCopy);
	SafeStrcpy(fileNameBuffer, bufferSize, filename);
#endif
}

void FileSystem::ExtensionFromPath(const char* path, char* extensionBuffer, size_t bufferSize)
{
	HP_ASSERT(path && path[0]);
	HP_ASSERT(extensionBuffer);

#ifdef WIN32

//	char drive[256];
//	char dir[256];
//	char filename[256];
//	char ext[256];
	_splitpath_s(
		path,
		/*drive*/NULL, /*sizeof(drive)*/0,        // e.g. "C:"
		/*dir*/NULL, /*sizeof(dir)*/0,            // e.g. "\dev\howprice\hoffgui\build\Debug\"
		/*filename*/NULL, /*sizeof(filename)*/0,  // filename e.g. Calculator
		extensionBuffer, bufferSize);           // e.g. ".cnf" or ".asm". Maybe even ".exe"
#else
	extensionBuffer[0] = '\0'; // default

	// basename may be destructive, so make a copy
	char pathCopy[kMaxPath];
	SafeStrcpy(pathCopy, sizeof(pathCopy), path);
	const char* filename = basename(pathCopy); // e.g. "Bootblock.cnf"
	if (!filename)
		return;
	const char* extension = (strrchr(filename, '.'));
	if (!extension)
		return;
	SafeStrcpy(extensionBuffer, bufferSize, extension);
#endif
}

void FileSystem::MakePath(char* path, size_t bufferSize, const char* directory, const char* filename)
{
	HP_ASSERT(directory != nullptr && directory[0] != '\0');
	HP_ASSERT(filename != nullptr && filename[0] != '\0');

	if (directory[strlen(directory) - 1] == '\\' || directory[strlen(directory) - 1] == '/')
		SafeSnprintf(path, bufferSize, "%s%s", directory, filename);
	else
		SafeSnprintf(path, bufferSize, "%s%c%s", directory, std::filesystem::path::preferred_separator, filename);
}

const char* FileSystem::GetApplicationDirectory()
{
	HP_ASSERT(s_pApplicationDirectory && s_pApplicationDirectory[0]);
	return s_pApplicationDirectory;
}

const char* FileSystem::GetUserPrefDirectory()
{
	HP_ASSERT(s_pPrefPath && s_pPrefPath[0]);
	return s_pPrefPath;
}

bool FileSystem::Copy(const char* from, const char* to)
{
	HP_ASSERT(from != nullptr);
	HP_ASSERT(to != nullptr);

	std::error_code ec;

#ifdef _MSC_VER
	// std::filesystem uses wchar on Windows.
	// SDL uses UTF-8, so we need to convert to wide char.
	WCHAR wfrom[kMaxPath];
	WCHAR wto[kMaxPath];
	::MultiByteToWideChar(CP_UTF8, 0, from, -1, wfrom, kMaxPath);
	::MultiByteToWideChar(CP_UTF8, 0, to, -1, wto, kMaxPath);
	std::filesystem::copy(wfrom, wto, std::filesystem::copy_options::overwrite_existing, ec);
#else
	std::filesystem::copy(from, to, std::filesystem::copy_options::overwrite_existing, ec);
#endif
	if (ec)
	{
		LOG_ERROR("Failed to copy %s to %s : %s\n", from, to, ec.message().c_str());
		return false;
	}

	LOG_TRACE("Copied %s to %s\n", from, to);

	return true;
}

bool FileSystem::MakeDir(const char* path)
{
	HP_ASSERT(path && path[0]);

	std::error_code ec;

#ifdef _MSC_VER
	// std::filesystem uses wchar on Windows.
	// SDL uses UTF-8, so we need to convert to wide char.
	WCHAR wpath[kMaxPath];
	::MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, kMaxPath);
	std::filesystem::create_directory(wpath, ec);
	if (ec)
	{
		LOG_ERROR("Failed to create directory %s: %s", path, ec.message().c_str());
		return false;
	}

	// #TODO: Should log the wide char path
//	LOG_TRACE("Created directory %s\n", wpath);
	LOG_TRACE("Created directory %s\n", path);
#else
	std::filesystem::create_directory(path, ec);
	if (ec)
	{
		LOG_ERROR("Failed to create directory %s: %s", path, ec.message().c_str());
		return false;
	}
	LOG_TRACE("Created directory %s\n", path);
#endif

	return true;
}
