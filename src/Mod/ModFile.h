#pragma once

#include "Core/Helpers.h"

#include <stdint.h>

class ModFile
{
public:

	// Static class - only one file supported for now
	NON_INSTANTIABLE_STATIC_CLASS(ModFile);

	static void New();
	static bool Load(const char* path);
	static bool Save();
	static bool SaveAs(const char* path);
	static void Free();
	static bool IsLoaded();

	static const char* GetPath();

	static const uint8_t* GetData();
	static unsigned int GetDataSizeBytes();
};
