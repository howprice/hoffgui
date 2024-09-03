#pragma once

#include "Core/Helpers.h"

class RecentFiles
{
public:
	NON_INSTANTIABLE_STATIC_CLASS(RecentFiles);

	static void Add(const char* fullPath);
	static void Clear();

	static unsigned int GetCount();
	static const char* GetFileFullPath(unsigned int index);
	static void Append(const char* fullPath);
	static void RemoveByIndex(unsigned int index);
};
