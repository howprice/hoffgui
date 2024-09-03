#include "HoffGui.h"

#include "HoffGui/Windows/ImGuiDemoWindow.h"
#include "HoffGui/Windows/OptionsWindow.h"
#include "HoffGui/Windows/OutputWindow.h"
#include "HoffGui/Windows/ModWindow.h"

#include "HoffGui/Dialogues/AboutPopup.h"
#include "HoffGui/Dialogues/FileDialogue.h"

#include "HoffGui/MainMenu.h"
#include "HoffGui/Options.h"

#include "ImGuiWrap/Fonts.h"

#include "Core/FileSystem.h"
#include "Core/Window.h"
#include "Core/Log.h"
#include "Core/StringHelpers.h"

#include "CommandLineArgs.h"
#include "AppVersion.h"

#include "imgui/imgui_internal.h"
#include "ImGuiWrap/ImGuiWrap.h"

#include "SDL.h"

//------------------------------------------------------------------------------------------------
// State
//------------------------------------------------------------------------------------------------

static bool s_initialised = false;

static ImVec4 s_clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

static bool s_resetWindowLayout;

//------------------------------------------------------------------------------------------------

static void logCallback(int logLevel, const char* format, va_list argList)
{
	// n.b. Can't re-use a va_list. Need to make copies each time it is used.
	// Undefined behaviour otherwise. OK on Windows, but segfaults on Linux.
	va_list argcopy;
	va_copy(argcopy, argList);

	// Append to output window
	OutputWindow::Vfprintf(format, argList);

#if 0 // Deliberately disabled because has side effect of closing modal popups e.g. ProcessWithConfigDialogue
	// Ensure the user is aware of any error messages
	if (logLevel == LOG_LEVEL_ERROR || logLevel == LOG_LEVEL_WARN)
		OutputWindow::Focus();
#else
	HP_UNUSED(logLevel);
#endif

	va_end(argcopy);
}

//------------------------------------------------------------------------------------------------

// Implementation of function defined in hp_assert.h
// Deliberately not static so has external linkage
const char* HpAssertGetApplicationVersionNumberString()
{
#if 0 // Not thread safe and sprintf is not save (warnings)
	static char version[64];
	sprintf(version, "%s%s", GetAppVersion(), GetAppVersonSuffix());
	return version;
#else
	return GetAppVersion();
#endif
}

//------------------------------------------------------------------------------------------------

static void updateGlobalKeyboardShortcuts()
{
	const ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard)
		return;

	if (ImGui::IsKeyPressed(ImGuiKey_Enter) && io.KeyMods == (ImGuiMod_Shift | ImGuiMod_Alt))
	{
#if FULLSCREEN_ALLOWED
		// Only allow fullscreen in release builds (can cause debugging problems)
		Window::SetFullscreen(!Window::IsFullscreen());
#else
		LOG_WARN("Fullscreen is not allowed in this build\n");
#endif
	}

#if 0

	if (ImGui::IsKeyPressed(ImGuiKey_F5))
	{
		// if shift pressed and analysis is running then stop
		if (io.KeyShift)
		{
			if (io.KeyCtrl) // Ctrl+Shift+F5
			{
				if (Debugger::CanRestart())
					Debugger::Restart();
			}
			else // Shift+F5
			{

				if (Debugger::CanStop())
					Debugger::Stop();
			}
		}
		else // F5
		{
			if (Debugger::CanStart())
				Debugger::Start();
			else if (Debugger::CanContinue())
				Debugger::Continue();
		}
	}
	else if (ImGui::IsKeyPressed(ImGuiKey_F6))
	{
		if (Debugger::CanBreak())
			DebuggerUI::Break();
	}
	if (ImGui::IsKeyPressed(ImGuiKey_F11))
	{
		if (Debugger::CanStepInto())
			DebuggerUI::StepInto();
	}

	// Global shortcut keys that are only available if an ira disassembling session is active
	if (Disassembly::IsLoaded())
	{

	}
#endif
}

static void setDefaultDockingLayout(ImGuiID mainDockNode)
{
	// Clear any preexisting layouts
	ImGui::DockBuilderRemoveNodeChildNodes(mainDockNode);

	// Docking has the concept of a central node, which uses the remaining space when a window is resized.
	// The direction of your DockBuilderSplitNode() calls are meaningful, because they determine where the
	// central node will be.
	// https://github.com/ocornut/imgui/issues/5209#issuecomment-1102272275
	//
	// We have two options:
	// 1. Make the disassembly window the central node, because this is the most important window.
	// 2. Remove the central node, so all windows are equally important and can be resized equally.

	ImGuiID leftDock, rightDock;
	const float kLeftRightRatio = 0.4f;
#if 1
	ImGui::DockBuilderSplitNode(mainDockNode, ImGuiDir_Left, kLeftRightRatio, &leftDock, &rightDock);
#else
	// Reverse split direction so left dock becomes the central node
	// This doesn't seem to work as expected.
	ImGui::DockBuilderSplitNode(mainDockNode, ImGuiDir_Right, 1.0f - kLeftRightRatio, &rightDock, &leftDock);
#endif

	ImGui::DockBuilderDockWindow(ModWindow::kWindowName, leftDock);

	ImGui::DockBuilderDockWindow(OutputWindow::kWindowName, rightDock);

	ImGui::DockBuilderFinish(mainDockNode);

	// Remove the CentralNode so all ImGui windows scale proportionally when the native window is resized
	// Hope this is OK. See https://github.com/ocornut/imgui/issues/5802#issuecomment-2202734342
	ImGuiDockNode* pCentralNode = ImGui::DockBuilderGetCentralNode(mainDockNode);
	if (pCentralNode) // can be null when user resets window layout and this function is called again
	{
		HP_ASSERT(pCentralNode->LocalFlags & ImGuiDockNodeFlags_CentralNode);
		pCentralNode->SetLocalFlags(pCentralNode->LocalFlags & ~ImGuiDockNodeFlags_CentralNode);
	}

	// Set all default visibilities
	ModWindow::SetVisible(true);
	OutputWindow::SetVisible(true);
}

static void updateDockingLayout()
{
	// Must be called each frame
	const ImGuiID mainDockNode = ImGui::DockSpaceOverViewport(/*dockspace_id*/0, ImGui::GetMainViewport());

	static bool firstFrame = true;

	// Don't set on first frame if already split, which implies a layout has been loaded from imgui.ini
	bool setLayout = ((firstFrame && !ImGui::DockBuilderGetNode(mainDockNode)->IsSplitNode()) || s_resetWindowLayout);

	// May already have docking layout loaded from ini file
	if (setLayout)
		setDefaultDockingLayout(mainDockNode);

	if (firstFrame)
		firstFrame = false;

	if (s_resetWindowLayout)
		s_resetWindowLayout = false;
}

void HoffGui::ResetWindowLayout()
{
	// #TODO: Should this also reset size and position of main OS Window?
	s_resetWindowLayout = true;
}

static void updateWindows()
{
	// Global popups and dialogues
	AboutPopup::Update();

	// Windows
	ImGuiDemoWindow::Update(s_clearColor);
	ModWindow::Update();
	OutputWindow::Update();
	OptionsWindow::Update();
}

//------------------------------------------------------------------------------------------------

bool HoffGui::Init()
{
	HP_ASSERT(!s_initialised);

	SetLogCallback(logCallback);

	LOG_INFO("Welcome to hoffgui V%s%s\n", GetAppVersion(), GetAppVersonSuffix());

	if (!Fonts::Load(/*zoomFactor*/1.0f))
	{
		LOG_ERROR("Failed to load fonts\n");
		return false;
	}

	if (!FileDialogue::Init())
	{
		LOG_ERROR("FileDialogue failed to initialise\n");
		return EXIT_FAILURE;
	}

	// Need to load options before loading any data that may depend on them
	if (!GetCommandLineArgs().ignoreIniFile)
		LoadOptions(g_options);

	OutputWindow::Init(&g_options.view.outputWindow);
	ModWindow::Init();

	s_initialised = true;

	return true;
}

void HoffGui::Shutdown()
{
	HP_ASSERT(s_initialised);

	SaveOptions(g_options);

	ModWindow::Shutdown();
	OutputWindow::Shutdown();
	SetLogCallback(nullptr);
	s_initialised = false;
}

bool HoffGui::Update()
{
	HP_ASSERT(s_initialised);

	ImGui::GetIO().FontDefault = Fonts::GetFont(g_options.view.defaultFontType);

	updateDockingLayout();

	bool quit = false;
	MainMenu::Update(/*out*/quit);
	if (quit)
	{
		// #TODO: Display confirmation dialogue if unsaved changes
		return false;
	}

	updateGlobalKeyboardShortcuts();

	updateWindows();

	return true;
}

const ImVec4& HoffGui::GetClearColor()
{
	return s_clearColor;
}
