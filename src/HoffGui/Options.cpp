#include "Options.h"

#include "HoffGui/Windows/OptionsWindow.h"
#include "HoffGui/Windows/OutputWindow.h"
#include "HoffGui/Windows/ModWindow.h"
#include "HoffGui/RecentFiles.h"

#include "Core/Window.h"
#include "Core/IniFile.h"
#include "Core/Log.h"
#include "Core/FileSystem.h"
#include "Core/StringHelpers.h"
#include "Core/hp_assert.h"

#include "ImGuiWrap/ImGuiWrap.h"

#include "AppVersion.h"

#include "SDL.h" // SDL_WINDOWPOS_CENTERED

//-----------------------------------------------------------------------------------------------------
// Helper macros for writing and parsing options

#define WRITE_OPTIONS_BOOL(x) IniFile::WriteBool(pFile, #x, options.x);
#define WRITE_OPTIONS_UINT(x) IniFile::WriteUint(pFile, #x, options.x);
#define WRITE_OPTIONS_HEX(x) IniFile::WriteHex(pFile, #x, options.x);
#define WRITE_OPTIONS_FLOAT(x) IniFile::WriteFloat(pFile, #x, options.x);
#define WRITE_OPTIONS_STRING(x) IniFile::WriteString(pFile, #x, options.x);
#define WRITE_OPTIONS_ENUM(x) IniFile::WriteEnum(pFile, #x, options.x);

#define PARSE_OPTIONS_BOOL(x) if (strcmp(key, #x) == 0) { options.x = IniFile::ParseBool(value); }
#define PARSE_OPTIONS_UINT(x) if (strcmp(key, #x) == 0) { options.x = IniFile::ParseUint(value); }
#define PARSE_OPTIONS_HEX(x) if (strcmp(key, #x) == 0) { options.x = IniFile::ParseHex(value); }
#define PARSE_OPTIONS_FLOAT(x) if (strcmp(key, #x) == 0) { options.x = IniFile::ParseFloat(value); }
#define PARSE_OPTIONS_STRING(x) if (strcmp(key, #x) == 0) { IniFile::ParseString(value, options.x, sizeof(options.x)); }
#define PARSE_OPTIONS_ENUM(x) if (strcmp(key, #x) == 0) { options.x = IniFile::ParseEnum<decltype(options.x)>(value); }

//-----------------------------------------------------------------------------------------------------

static const char* kFilename = "hoffgui.ini";

static char s_optionsStorage[1024];
static size_t s_optionsStorageSize = 0;

// e.g. "C:\Users\Howard\AppData\Roaming\TTE\hoffgui\hoffgui.ini"
void ConstructOptionsPath(char* optionsPath, size_t optionsPathSize)
{
	HP_ASSERT(optionsPath != nullptr);
	const char* directory = FileSystem::GetUserPrefDirectory();
	FileSystem::MakePath(optionsPath, optionsPathSize, directory, kFilename);
}

static void writeWindowSection(FILE* pFile)
{
	HP_ASSERT(pFile != nullptr);

	IniFile::WriteSection(pFile, "Window");
	LOG_TRACE("Writing [Window] settings:\n");

	// position
	int x, y;
	Window::GetPosition(x, y);
	IniFile::WriteInt(pFile, "x", x);
	IniFile::WriteInt(pFile, "y", y);
	LOG_TRACE("x=%u y=%u\n", x, y);

	// size
	unsigned int w, h;
	Window::GetSize(w, h);
	IniFile::WriteUint(pFile, "w", w);
	IniFile::WriteUint(pFile, "h", h);
	LOG_TRACE("w=%u h=%u\n", w, h);

	// display (monitor) index
	int displayIndex = Window::GetDisplayIndex();
	if (displayIndex >= 0)
	{
		IniFile::WriteInt(pFile, "displayIndex", displayIndex);
		LOG_TRACE("displayIndex=%d\n", displayIndex);
	}

	// maximised
	IniFile::WriteBool(pFile, "maximised", Window::IsMaximised());
	LOG_TRACE("maximised=%u\n", Window::IsMaximised() ? 1 : 0);

	// minimised
	IniFile::WriteBool(pFile, "minimised", Window::IsMinimised());
	LOG_TRACE("minimised=%u\n", Window::IsMinimised() ? 1 : 0);

	// fullscreen
	IniFile::WriteBool(pFile, "fullscreen", Window::IsFullscreen());
	LOG_TRACE("fullscreen=%u\n", Window::IsFullscreen() ? 1 : 0);
}

static void writeViewSection(FILE* pFile, const ViewOptions& options)
{
	HP_ASSERT(pFile != nullptr);

	IniFile::WriteSection(pFile, "View");

	WRITE_OPTIONS_ENUM(defaultFontType); // #TODO: Save string instead to make robust to font changes
	IniFile::WriteUint(pFile, "ZoomFactorIndex", ImGuiWrap::GetZoomFactorIndex());
}

static bool parseViewOption(const char* key, const char* value, ViewOptions& options, unsigned int lineNumber)
{
	PARSE_OPTIONS_ENUM(defaultFontType)
	else if (strcmp(key, "ZoomFactorIndex") == 0)
	{
		unsigned int x = IniFile::ParseUint(value);
		ImGuiWrap::SetZoomFactorIndex(x);
	}
	else
	{
		LOG_ERROR("Unrecognised key in View section in options file line %u: %s\n", lineNumber, key);
		return false;
	}

	return true;
}

static void writeWindowVisibilitySection(FILE* pFile)
{
	HP_ASSERT(pFile != nullptr);

	IniFile::WriteSection(pFile, "WindowVisibility");

#define WINDOW_LIST_MACRO(T) fprintf(pFile, #T"=%u\n", T::IsVisible() ? 1 : 0);
#include "HoffGui/Windows/WindowList.h"
}

static bool parseWindowVisibilityOption(const char* key, const char* value, unsigned int lineNumber)
{
	unsigned int x = strtoul(value, nullptr, 10);
	bool visible = (x != 0);

#define WINDOW_LIST_MACRO(T) \
	if (strcmp(key, #T) == 0) \
	{ \
		T::SetVisible(visible); \
		return true; \
	}
#include "HoffGui/Windows/WindowList.h"
	else
	{
		LOG_ERROR("Unrecognised key in WindowVisibility section in options file line %u: %s\n", lineNumber, key);
		return false;
	}
}

//-----------------------------------------------------------------------------------------------------

static void writeOutputWindowSection(FILE* pFile, const OutputWindow::Options& options)
{
	HP_ASSERT(pFile != nullptr);

	IniFile::WriteSection(pFile, "OutputWindow");
	WRITE_OPTIONS_BOOL(useDefaultFont);
	WRITE_OPTIONS_ENUM(fontType); // #TODO: Save string instead to make robust to font changes
}

static bool parseOutputWindowOption(const char* key, const char* value, OutputWindow::Options& options, unsigned int lineNumber)
{
	PARSE_OPTIONS_BOOL(useDefaultFont)
	else PARSE_OPTIONS_ENUM(fontType)
	else
	{
		LOG_ERROR("Unrecognised OutputWindow option on line %u: %s=%s\n", lineNumber, key, value);
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------------------------------

static void writeResourceSection(FILE* pFile, const ResourceOptions& options)
{
	HP_ASSERT(pFile != nullptr);

	IniFile::WriteSection(pFile, "Resource");

	WRITE_OPTIONS_STRING(vstDirectory);
}

static bool parseResourceOption(const char* key, const char* value, ResourceOptions& options, unsigned int lineNumber)
{
	PARSE_OPTIONS_STRING(vstDirectory)
	else
	{
		LOG_ERROR("Unrecognised Resource option on line %u: %s=%s\n", lineNumber, key, value);
		return false;
	}

	return true;
}

static void writeRecentFilesSection(FILE* pFile)
{
	HP_ASSERT(pFile != nullptr);

	IniFile::WriteSection(pFile, "RecentFiles");

	for (unsigned int i = 0; i < RecentFiles::GetCount(); i++)
	{
		char key[32];
		SafeSnprintf(key, sizeof(key), "File%u", i);
		IniFile::WriteString(pFile, key, RecentFiles::GetFileFullPath(i));
	}
}

static bool parseRecentFilesOption(const char* key, const char* value)
{
	// Ignore key, just append entries in order
	HP_UNUSED(key);
	RecentFiles::Append(value);
	return true;
}

//-----------------------------------------------------------------------------------------------------

static bool optionsIniHandler(const char* pSection, const char* key, const char* value, void* pUserData, unsigned int lineNumber)
{
	HP_ASSERT(pUserData != nullptr);
	Options& options = *(Options*)pUserData;

	LOG_TRACE("Line %u: section=%s %s=%s\n", lineNumber, pSection ? pSection : "<none>", key, value);

	if (pSection == nullptr)
		return false;

	if (strcmp(pSection, "Window") == 0)
	{
		// Ignore window settings, which are parsed earlier.
		return true;
	}
	else if (strcmp(pSection, "View") == 0)
	{
		return parseViewOption(key, value, options.view, lineNumber);
	}
	else if (strcmp(pSection, "WindowVisibility") == 0)
	{
		return parseWindowVisibilityOption(key, value, lineNumber);
	}
	else if (strcmp(pSection, "OutputWindow") == 0)
	{
		return parseOutputWindowOption(key, value, options.view.outputWindow, lineNumber);
	}
	else if (strcmp(pSection, "Resource") == 0)
	{
		return parseResourceOption(key, value, options.resource, lineNumber);
	}
	else if (strcmp(pSection, "RecentFiles") == 0)
	{
		return parseRecentFilesOption(key, value);
	}
	else
	{
		LOG_ERROR("Unhandled [%s] section in options ini file\n", pSection);
		return false;
	}
}

bool LoadOptions(Options& options)
{
	HP_ASSERT(s_optionsStorageSize == 0, "#TODO: Support loading options multiple times iff required");

	char optionsPath[kMaxPath];
	ConstructOptionsPath(optionsPath, sizeof(optionsPath));

	if (!FileSystem::Exists(optionsPath))
	{
		LOG_TRACE("Options file not found: %s\n", optionsPath);
		return false;
	}

	if (!IniFile::Parse(optionsPath, optionsIniHandler, /*pUserData*/&options))
	{
		LOG_ERROR("Failed to parse options file: %s\n", optionsPath);
		return false;
	}

	LOG_INFO("Loaded options file: %s\n", optionsPath);
	return true;
}

bool SaveOptions(const Options& options)
{
	char optionsPath[kMaxPath];
	ConstructOptionsPath(optionsPath, sizeof(optionsPath));

	HP_ASSERT(optionsPath[0] != '\0');

	FILE* pFile = fopen(optionsPath, "w");
	if (!pFile)
	{
		LOG_ERROR("Failed to open options file for write: %s\n", optionsPath);
		return false;
	}

	IniFile::WriteComment(pFile, "hoffgui options file (version %s%s)", GetAppVersion(), GetAppVersonSuffix());
	IniFile::WriteComment(pFile, "Delete this file to restore default settings.");
	writeWindowSection(pFile);
	writeViewSection(pFile, options.view);
	writeWindowVisibilitySection(pFile);
	writeOutputWindowSection(pFile, options.view.outputWindow);
	writeResourceSection(pFile, options.resource);
	writeRecentFilesSection(pFile);

	fclose(pFile);
	pFile = nullptr;

	LOG_INFO("Saved options file: %s\n", optionsPath);

	return true;
}
