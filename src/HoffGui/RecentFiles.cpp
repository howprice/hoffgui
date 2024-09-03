#include "RecentFiles.h"

#include "Core/hp_assert.h"
#include "Core/FileSystem.h"
#include "Core/StringHelpers.h"

#include <string.h>

struct RecentFile
{
	char fullPath[kMaxPath] = {};
};

static const unsigned int kMaxRecentFiles = 10;
static RecentFile s_recentFiles[kMaxRecentFiles]; // most recent file is at index 0
static unsigned int s_recentFileCount;

void RecentFiles::Add(const char* fullPath)
{
	// if the file is already in the list, move it to the top
	for (unsigned int i = 0; i < s_recentFileCount; ++i)
	{
		if (strcmp(s_recentFiles[i].fullPath, fullPath) == 0)
		{
			if (i == 0)
				return; // already at the top; no need to move it

			// move the file to the top
			for (unsigned int j = i; j > 0; --j)
			{
				s_recentFiles[j] = s_recentFiles[j - 1];
			}

			strcpy(s_recentFiles[0].fullPath, fullPath);
			return;
		}
	}

	if (s_recentFileCount > 0)
	{
		// shift all entries up
		int maxIndex = Min(s_recentFileCount, kMaxRecentFiles - 1);
		for (int i = maxIndex; i >= 1; --i)
		{
			s_recentFiles[i] = s_recentFiles[i - 1];
		}
	}
	strcpy(s_recentFiles[0].fullPath, fullPath); // move new entry into top slot
	s_recentFileCount = Min(s_recentFileCount + 1, kMaxRecentFiles);
}

void RecentFiles::Clear()
{
	s_recentFileCount = 0;
}

unsigned int RecentFiles::GetCount()
{
	return s_recentFileCount;
}

const char* RecentFiles::GetFileFullPath(unsigned int index)
{
	HP_ASSERT(index < s_recentFileCount);
	return s_recentFiles[index].fullPath;
}

void RecentFiles::Append(const char* fullPath)
{
	HP_ASSERT(s_recentFileCount < COUNTOF_ARRAY(s_recentFiles));
	unsigned int index = s_recentFileCount++;
	SafeStrcpy(s_recentFiles[index].fullPath, kMaxPath, fullPath);
}

void RecentFiles::RemoveByIndex(unsigned int index)
{
	HP_ASSERT(index < s_recentFileCount);

	for (unsigned int i = index; i < s_recentFileCount - 1; i++)
	{
		s_recentFiles[i] = s_recentFiles[i + 1];
	}

	--s_recentFileCount;
}
