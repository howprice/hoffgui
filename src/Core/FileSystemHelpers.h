#pragma once

#include "Core/Helpers.h"

class FileSystemHelpers
{
public:
	NON_INSTANTIABLE_STATIC_CLASS(FileSystemHelpers);

	static void ConstructSidecarFileName(
		const char* path, // No extension or arbitrary extension e.g. Calculator, Calculator.exe, Calculator.bin
		const char* extension, // e.g. ".lst"
		char* sidecarPath,
		size_t bufferSizeBytes);
};
