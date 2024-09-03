#include "FileSystemHelpers.h"

#include "Core/FileSystem.h"
#include "Core/StringHelpers.h"
#include "Core/hp_assert.h"

#ifdef _MSC_VER
// #TODO: Remove this
#include <Windows.h> // _splitpath_s, _makepath_s
#else
#include <libgen.h> // dirname
#endif

void FileSystemHelpers::ConstructSidecarFileName(
	const char* path, /* No extension or arbitrary extension e.g. Calculator, Calculator.exe, Calculator.bin */
	const char* extension, /* e.g. ".lst" */
	char* sidecarPath,
	size_t bufferSizeBytes)
{
	HP_ASSERT(path && path[0]);

#ifdef WIN32

	char drive[256];
	char dir[256];
	char filename[256];
	char ext[256];
	_splitpath_s(
		path,
		drive, sizeof(drive),        // e.g. "C:"
		dir, sizeof(dir),            // e.g. "\dev\howprice\hoffgui\build\Debug\"
		filename, sizeof(filename),  // filename e.g. Calculator
		ext, sizeof(ext));           // includes dot e.g. "", ".exe"

	HP_ASSERT(sidecarPath != nullptr);
	_makepath_s(sidecarPath, bufferSizeBytes, drive, dir, filename, extension);
#else

#ifdef __APPLE__
	char directoryBuffer[kMaxPath];
	SafeStrcpy(directoryBuffer, sizeof(directoryBuffer), path);
	const char* directory = dirname(directoryBuffer);
#else
	char directory[kMaxPath];
	SafeStrcpy(directory, sizeof(directory), path);
	dirname(directory); // n.b. no trailing slash e.g. "/home/howard/GitHub/howprice/hoffgui/data"
#endif
	// Construct filename, without extension
	char filenameBuffer[kMaxPath];
	SafeStrcpy(filenameBuffer, sizeof(filenameBuffer), path);
	char* filename = basename(filenameBuffer); // e.g. "Bootblock"
	char* dot = (strrchr(filename, '.'));
	if (dot)
		*dot = '\0';

	// e.g. "/home/howard/GitHub/howprice/hoffgui/data/Bootblock.lst"
	SafeSnprintf(sidecarPath, bufferSizeBytes, "%s/%s.%s", directory, filename, extension);
#endif
}
