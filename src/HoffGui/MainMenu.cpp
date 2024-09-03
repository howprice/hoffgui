#include "MainMenu.h"

#include "HoffGui/Dialogues/AboutPopup.h"
#include "HoffGui/Dialogues/FileDialogue.h"
#include "HoffGui/Windows/ImGuiDemoWindow.h"
#include "HoffGui/Windows/OptionsWindow.h"
#include "HoffGui/Windows/OutputWindow.h"
#include "HoffGui/Windows/ModWindow.h"

#include "HoffGui/Options.h"
#include "HoffGui/HoffGui.h"
#include "HoffGui/RecentFiles.h"
#include "Mod/ModFile.h"

#include "ImGuiWrap/Fonts.h"

#include "Core/StringHelpers.h"
#include "Core/Window.h"
#include "Core/Log.h"

#include "ImGuiWrap/ImGuiWrap.h"

static bool s_visible = true;

struct Actions
{
	bool openNewFilePopup = false;
	bool openExistingFilePopup = false;
	bool openRecentFile = false;
	unsigned int recentFileIndex = 0;
	bool closeFile = false;
	bool saveFile = false;
	bool saveFileAs = false;
	bool openAboutPopup = false;
	bool quit = false;
};

static void doFileMenu(Actions& actions)
{
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("New...", /*shortcut*/nullptr))
			actions.openNewFilePopup = true;

		if (ImGui::MenuItem("Open...", /*shortcut*/nullptr))
			actions.openExistingFilePopup = true;

		ImGui::Separator();

		if (ImGui::BeginMenu("Recent Files"))
		{
			const unsigned int recentFilesCount = RecentFiles::GetCount();
			if (recentFilesCount == 0)
			{
				ImGui::MenuItem("No recent files", /*shortcut*/nullptr, /*selected*/false, /*enabled*/false);
			}
			else
			{
				for (unsigned int fileIndex = 0; fileIndex < recentFilesCount; fileIndex++)
				{
					const char* filename = RecentFiles::GetFileFullPath(fileIndex);

					if (ImGui::MenuItem(filename))
					{
						actions.openRecentFile = true;
						actions.recentFileIndex = fileIndex;
					}
				}

				ImGui::Separator();
				if (ImGui::MenuItem("Clear"))
					RecentFiles::Clear();
			}

			ImGui::EndMenu();
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Close", /*shortcut*/nullptr, /*selected*/false, /*enabled*/ModFile::IsLoaded()))
			actions.closeFile = true;

		ImGui::Separator();

		if (ImGui::MenuItem("Save.", /*shortcut*/nullptr, /*pSelected*/nullptr, /*enabled*/ModFile::IsLoaded() && ModFile::GetPath()[0] != '\0'))
			actions.saveFile = true;

		if (ImGui::MenuItem("Save as...", /*shortcut*/nullptr, /*pSelected*/nullptr, /*enabled*/ModFile::IsLoaded()))
			actions.saveFileAs = true;

		ImGui::Separator();
		if (ImGui::MenuItem("Exit", /*shortcut*/nullptr))
			actions.quit = true;

		ImGui::EndMenu();
	}
}

static void doEditMenu()
{
	if (ImGui::BeginMenu("Edit"))
	{
		if (ImGui::MenuItem("Cut", "Ctrl+C", /*pSelected*/nullptr, /*enabled*/false))
		{
		}

		if (ImGui::MenuItem("Copy", "Ctrl+C", /*pSelected*/nullptr, /*enabled*/false))
		{
		}

		if (ImGui::MenuItem("Paste", "Ctrl+V", /*pSelected*/nullptr, /*enabled*/false))
		{
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Undo", "Ctrl+Z", /*pSelected*/nullptr, /*enabled*/false))
		{
		}
		if (ImGui::MenuItem("Redo", "Ctrl+Y", /*pSelected*/nullptr, /*enabled*/false))
		{
		}

		ImGui::EndMenu();
	}
}

static void doViewMenu()
{
	if (ImGui::BeginMenu("View"))
	{
		if (ImGui::MenuItem("Goto", "G", /*selected*/false, /*enabled*/false))
		{
		}

		char label[64];
		SafeSnprintf(label, sizeof(label), "Zoom + (%.0f%%)", 100.0f * ImGuiWrap::GetZoom());
		if (ImGui::MenuItem(label, "Ctrl+=", /*selected*/false, /*enabled*/ImGuiWrap::CanIncreaseZoom()))
			ImGuiWrap::IncreaseZoom();

		if (ImGui::MenuItem("Zoom -", "Ctrl+-", /*selected*/false, /*enabled*/ImGuiWrap::CanDecreaseZoom()))
			ImGuiWrap::DecreaseZoom();

		if (ImGui::MenuItem("Full Screen", "Shift+Alt+Enter", Window::IsFullscreen()))
		{
#if FULLSCREEN_ALLOWED
			// Only allow fullscreen in release builds (can cause debugging problems)
			Window::SetFullscreen(!Window::IsFullscreen());
#else
			LOG_WARN("Fullscreen is not allowed in this build\n");
#endif
		}

		ImGui::EndMenu();
	}
}

static void doOptionsMenu()
{
	if (ImGui::BeginMenu("Options"))
	{
		if (ImGui::MenuItem("Options..."))
		{
			OptionsWindow::SetVisible(true);
			OptionsWindow::Focus();
		}

		ImGui::EndMenu();
	}
}

static void doWindowMenu()
{
	if (ImGui::BeginMenu("Window"))
	{
		if (ImGui::MenuItem("Reset Window Layout"))
			HoffGui::ResetWindowLayout();

		ImGui::Separator();

		bool windowVisible;

		// Sorted alphabetically to make it easier for the user to find the Window in the list

		windowVisible = OptionsWindow::IsVisible();
		if (ImGui::MenuItem("Options", nullptr, &windowVisible))
			OptionsWindow::SetVisible(windowVisible);

		windowVisible = OutputWindow::IsVisible();
		if (ImGui::MenuItem("Output", nullptr, &windowVisible))
			OutputWindow::SetVisible(windowVisible);

		windowVisible = ModWindow::IsVisible();
		if (ImGui::MenuItem("MOD", nullptr, &windowVisible))
			ModWindow::SetVisible(windowVisible);

		ImGui::EndMenu();
	}
}

static void doDevMenu()
{
#ifndef RELEASE
	if (ImGui::BeginMenu("Dev"))
	{
		ImGui::MenuItem("Placeholder", /*shortcut*/nullptr, /*selected*/false, /*enabled*/false);
		ImGui::EndMenu();
	}
#endif
}

static void doHelpMenu(Actions& actions)
{
	if (ImGui::BeginMenu("Help"))
	{
		bool windowVisible;

		ImGui::Separator();

		windowVisible = ImGuiDemoWindow::IsVisible();
		if (ImGui::MenuItem("Show ImGui Demo Window", nullptr, &windowVisible))
			ImGuiDemoWindow::SetVisible(windowVisible);

		ImGuiDemoWindow::Options& options = ImGuiDemoWindow::GetOptions();
		ImGui::MenuItem("Show ImGui About Window", nullptr, &options.showAboutWindow);

		ImGui::Separator();

		if (ImGui::MenuItem("About..."))
			actions.openAboutPopup = true;

		ImGui::EndMenu();
	}

}

static void openRecentFile(unsigned int recentFileIndex)
{
	// if config file exists, then process with config (auto-detect file type) else preprocess

	const char* path = RecentFiles::GetFileFullPath(recentFileIndex);

	if (!FileSystem::Exists(path))
	{
		LOG_ERROR("Recent file not found. Removing from list: %s\n", path);
		RecentFiles::RemoveByIndex(recentFileIndex);
		return;
	}

	if (ModFile::Load(path))
		ModWindow::SetFilePath(path);
	else
		LOG_ERROR("Failed to load file\n");
}

static void processActions(const Actions& actions)
{
	if (actions.openNewFilePopup)
		ModFile::New();

	if (actions.openExistingFilePopup)
	{
		char path[kMaxPath] = {};

		// This call blocks until user selects a file or cancels
		FileDialogue::OpenFileDialogue("File", path, sizeof(path)); // n.b. no return code. If cancelled then path is empty

		if (path[0] != '\0')
		{
			if (ModFile::Load(path))
			{
				LOG_INFO("Loaded file: %s\n", path);
				ModWindow::SetFilePath(path);
				RecentFiles::Add(path);
			}
			else
			{
				LOG_ERROR("Failed to load file: %s\n", path);
			}
		}
	}

	if (actions.openRecentFile)
		openRecentFile(actions.recentFileIndex);

	if (actions.closeFile)
	{
		ModFile::Free();
		ModWindow::SetFilePath(nullptr);
	}

	if (actions.saveFile)
	{
		if (ModFile::Save())
			LOG_INFO("File saved: %s\n", ModFile::GetPath());
		else
			LOG_ERROR("Failed to save file\n");
	}

	if (actions.saveFileAs)
	{
		char path[kMaxPath] = {};

		// This call blocks until user selects a file or cancels
		FileDialogue::SaveFileDialogue("Save file", path, sizeof(path)); // n.b. no return code. If cancelled then path is empty

		if (path[0] != '\0')
		{
			if (ModFile::SaveAs(path))
			{
				LOG_INFO("File saved: %s\n", path);
				ModWindow::SetFilePath(path);
				RecentFiles::Add(path);
			}
			else
				LOG_ERROR("Failed to save file: %s\n", path);
		}
	}

	if (actions.openAboutPopup)
		ImGui::OpenPopup(AboutPopup::kName);
}

void MainMenu::Update(bool& quit)
{
	if (!s_visible)
		return;

	if (ImGui::BeginMainMenuBar() == false)
		return;

	Actions actions;
	doFileMenu(actions);
	doViewMenu();
	doEditMenu();
	doOptionsMenu();
	doWindowMenu();
	doDevMenu();
	doHelpMenu(actions);

	ImGui::EndMainMenuBar();

	processActions(actions);

	quit = actions.quit;
}

bool MainMenu::IsVisible()
{
	return s_visible;
}

void MainMenu::SetVisible(bool visible)
{
	s_visible = visible;
}
