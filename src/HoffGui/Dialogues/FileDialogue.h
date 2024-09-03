// Wrap portable-file-dialogs.h and associated STL includes
// to reduce build time.

#pragma once

#include "Core/Helpers.h"

class FileDialogue
{
	NON_INSTANTIABLE_STATIC_CLASS(FileDialogue);
public:

	static bool Init();

	// blocks until user selects a file or cancels
	static void OpenFileDialogue(
		const char* title,
		char* pFileNameBuffer,
		unsigned int bufferSize,
		unsigned int filterCount = 0,
		const char** ppFilters = nullptr);

	// blocks until user selects a file or cancels
	static void SaveFileDialogue(
		const char* title,
		char* pFileNameBuffer,
		unsigned int bufferSize,
		unsigned int filterCount = 0,
		const char** ppFilters = nullptr);

	// blocks until user selects a folder or cancels
	static void FolderDialogue(
		const char* title,
		char* pFolderBuffer,
		unsigned int bufferSize);
};
