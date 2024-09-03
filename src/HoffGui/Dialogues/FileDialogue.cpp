#include "FileDialogue.h"

#include "Core/Log.h"
#include "Core/hp_assert.h"
#include "Core/StringHelpers.h"

#include "portable-file-dialogs.h"

bool FileDialogue::Init()
{
	// Check that a backend is available
	if (!pfd::settings::available())
	{
		LOG_ERROR("Portable File Dialogs are not available on this platform.\n");
		return false;
	}

	return true;
}

void FileDialogue::OpenFileDialogue(
	const char* title,
	char* pFileNameBuffer, unsigned int bufferSize,
	unsigned int filterCount /*= 0*/, const char** ppFilters /*= nullptr*/)
{
	std::string default_path = "";

	std::vector<std::string> filters; // { "Config Files (.cnf)", "*.cnf" };
	for (unsigned int filterIndex = 0; filterIndex < filterCount; filterIndex++)
	{
		filters.emplace_back(ppFilters[filterIndex]);
	}

	std::vector<std::string> result = pfd::open_file(title, default_path, filters).result();
	if (result.empty())
		return; // user cancelled the operation

	const std::string strFilename = result[0];
	SafeStrcpy(pFileNameBuffer, bufferSize, strFilename.c_str());
}

void FileDialogue::SaveFileDialogue(
	const char* title,
	char* pFileNameBuffer, unsigned int bufferSize,
	unsigned int filterCount /*= 0*/, const char** ppFilters /*= nullptr*/)
{
	std::string default_path = "";

	std::vector<std::string> filters; // { "Config Files (.cnf)", "*.cnf" };
	for (unsigned int filterIndex = 0; filterIndex < filterCount; filterIndex++)
	{
		filters.emplace_back(ppFilters[filterIndex]);
	}

	std::string strFilename = pfd::save_file(title, default_path, filters).result();
	if (strFilename.empty())
		return; // user cancelled the operation

	SafeStrcpy(pFileNameBuffer, bufferSize, strFilename.c_str());
}

void FileDialogue::FolderDialogue(const char* title, char* pFolderBuffer, unsigned int bufferSize)
{
	HP_ASSERT(title && title[0] != '\0');
	HP_ASSERT(pFolderBuffer && bufferSize > 0);

	std::string selection = pfd::select_folder(title).result();
	if (selection.empty())
		return; // user cancelled the operation

	SafeStrcpy(pFolderBuffer, bufferSize, selection.c_str());
}
