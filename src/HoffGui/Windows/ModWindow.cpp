#include "ModWindow.h"

#include "Mod/ModFile.h"

#include "Core/Log.h"
#include "Core/FileSystem.h"
#include "Core/StringHelpers.h"

#include "ImGuiWrap/ImGuiWrap.h"

static bool s_visible = false;
static char s_windowName[256];

void ModWindow::Init()
{
	SafeStrcpy(s_windowName, sizeof(s_windowName), kWindowName);
}

void ModWindow::Shutdown()
{
	
}

static void showContents()
{
	if (!ModFile::IsLoaded())
		ImGui::Text("No file loaded");

	const uint8_t* pData = ModFile::GetData();
	const unsigned int dataSizeBytes = ModFile::GetDataSizeBytes();
	unsigned int i = 0;
	while (i < dataSizeBytes)
	{
		ImGui::Text("%02X", pData[i]);
		i++;
		if (i & 0xf)
			ImGui::SameLine();
	}
}

void ModWindow::Update()
{
	if (!s_visible)
		return;

	if (!ImGui::Begin(s_windowName, &s_visible))
	{
		ImGui::End();
		return;
	}

	showContents();

	ImGui::End();
}

void ModWindow::SetFilePath(const char* path)
{
	if (path && path[0])
	{
		char filename[256];
		FileSystem::FilenameWithExtensionFromPath(path, filename, sizeof(filename));
		SafeSnprintf(s_windowName, sizeof(s_windowName), "%s###MOD", filename);
	}
	else
	{
		SafeStrcpy(s_windowName, sizeof(s_windowName), kWindowName);
	}
}

bool ModWindow::IsVisible()
{
	return s_visible;
}

void ModWindow::SetVisible(bool visible)
{
	s_visible = visible;
}

const char* ModWindow::GetWindowName()
{
	return s_windowName;
}
