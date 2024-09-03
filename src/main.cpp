// This file is based on imgui-docking\examples\example_sdl2_opengl3\main.cpp
// 
// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "HoffGui/HoffGui.h"
#include "HoffGui/Options.h"

#include "Platform/SystemInfo.h"

#include "Core/Window.h"
#include "Core/Displays.h"
#include "Core/IniFile.h"
#include "Core/Log.h"
#include "Core/StringHelpers.h"
#include "Core/FileSystem.h"

#include "ImGuiWrap/ImGuiWrap.h"

#include "CommandLineArgs.h"
#include "AppVersion.h"

#include "SDL.h"
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include "SDL_opengles2.h"
#else
#include "SDL_opengl.h"
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

#include <stdio.h>

struct WindowInitParams
{
	int x = 0; // can be negative depending on multi-monitor OS configuration
	int y = 0;
	unsigned int w = 1024;
	unsigned int h = 600;
	unsigned int displayIndex = 0; // primary display
	bool maximised = false;
	bool minimised = false;
	bool fullscreen = false;
};

static bool s_quitOnEscape = false; // Don't want this on because Esc is a useful key for the user

static bool windowIniHandler(const char* pSection, const char* key, const char* value, void* pUserData, unsigned int lineNumber)
{
	HP_ASSERT(pUserData != nullptr);
	WindowInitParams& windowInitParams = *(WindowInitParams*)pUserData;

	LOG_TRACE("Line %u: section=%s %s=%s\n", lineNumber, pSection ? pSection : "<none>", key, value);

	if (pSection == nullptr)
		return false;

	if (strcmp(pSection, "Window") == 0)
	{
		int intVal = IniFile::ParseInt(value);
		bool boolVal = IniFile::ParseBool(value);

		if (strcmp(key, "x") == 0)
			windowInitParams.x = intVal;
		else if (strcmp(key, "y") == 0)
			windowInitParams.y = intVal;
		else if (strcmp(key, "w") == 0)
			windowInitParams.w = intVal;
		else if (strcmp(key, "h") == 0)
			windowInitParams.h = intVal;
		else if (strcmp(key, "displayIndex") == 0)
			windowInitParams.displayIndex = intVal;
		else if (strcmp(key, "maximised") == 0)
			windowInitParams.maximised = boolVal;
		else if (strcmp(key, "minimised") == 0)
			windowInitParams.minimised = boolVal;
		else if (strcmp(key, "fullscreen") == 0)
			windowInitParams.fullscreen = boolVal;
		else
		{
			LOG_ERROR("Unknown Window INI option: %s=%s\n", key, value);
			return false;
		}

		return true;
	}
	else
	{
		// ignore all other sections
		return false;
	}
}

static bool calcDefaultWindowXYWH(WindowInitParams& params)
{
	params.x = 0;
	params.y = 0;
	params.w = Window::kDefaultWidth;
	params.h = Window::kDefaultHeight;

	static const float kDefaultSizeFraction = 0.75f;
	SDL_Rect displayBounds;
	if (SDL_GetDisplayBounds(params.displayIndex, &displayBounds) != 0)
	{
		LOG_ERROR("Failed to get display bounds for display index: %d\n", params.displayIndex);
		return false;
	}
	params.w = (unsigned int)(kDefaultSizeFraction * displayBounds.w);
	params.h = (unsigned int)(kDefaultSizeFraction * displayBounds.h);

	// Default position is centred on the primary display
	// SDL_WINDOWPOS_CENTERED is no good because it doesn't take display index into account.
	params.x = displayBounds.x + (displayBounds.w - params.w) / 2; // centre
	params.y = displayBounds.y + (displayBounds.h - params.h) / 2;

	return true;
}

static SDL_Window* createWindow()
{
	// Defaults for normal window in primary display
	WindowInitParams params;
	calcDefaultWindowXYWH(params);

	// Read settings from ini file, if exists
	const CommandLineArgs& commandLineArgs = GetCommandLineArgs();
	if (!commandLineArgs.ignoreIniFile)
	{
		char optionsPath[kMaxPath];
		ConstructOptionsPath(optionsPath, sizeof(optionsPath));
		if (FileSystem::Exists(optionsPath))
		{
			if (!IniFile::Parse(optionsPath, windowIniHandler, /*pUserData*/&params))
			{
				LOG_ERROR("Failed to parse options file: %s\n", optionsPath);
			}
		}
		else
		{
			LOG_TRACE("Options file not found: %s\n", optionsPath);
		}
	}

	// Command line args override ini settings
	if (commandLineArgs.displayIndex >= 0)
	{
		// Command line has overridden the display index, so calculate the default xywh for this display.
		if (commandLineArgs.displayIndex < (int)Displays::GetCount())
		{
			params.displayIndex = commandLineArgs.displayIndex;

			calcDefaultWindowXYWH(params); // These values may be subsequently overridden by xywh command line args
		}
		else
			LOG_ERROR("Command line Display index %d is out of range\n", commandLineArgs.displayIndex);
	}
	if (commandLineArgs.hasWindowX)
		params.x = commandLineArgs.windowX;
	if (commandLineArgs.hasWindowY)
		params.y = commandLineArgs.windowY;
	if (commandLineArgs.windowWidth > 0)
		params.w = commandLineArgs.windowWidth;
	if (commandLineArgs.windowHeight > 0)
		params.h = commandLineArgs.windowHeight;
	if (commandLineArgs.hasMaximised)
		params.maximised = commandLineArgs.maximised;
	if (commandLineArgs.hasFullscreen)
		params.fullscreen = commandLineArgs.fullscreen;

	if (params.maximised || params.fullscreen)
	{
		// Loaded width and height may represent the maximised window, may be invalid, or may be too large for the display
		// Set sensible default size and position for when user restores the window
		calcDefaultWindowXYWH(params);
	}

#if !FULLSCREEN_ALLOWED
	if (params.fullscreen)
	{
		LOG_WARN("Fullscreen not allowed in this build\n");
		params.fullscreen = false;
	}
#endif

	char title[256];
	SafeSnprintf(title, sizeof(title), "hoffgui V%s%s", GetAppVersion(), GetAppVersonSuffix());

	HP_UNUSED(params.minimised); // Clang Wunused-but-set-variable. Never want to create a minimised window 

	if (!Window::Create(title, params.x, params.y, params.w, params.h, params.fullscreen, params.maximised))
	{
		fprintf(stderr, "Failed to create window\n");
		return nullptr;
	}

	SDL_Window* pWindow = &Window::GetSDLWindow();
	return pWindow;
}

int main(int argc, char** argv)
{
	printf("hoffgui V%s%s\n", GetAppVersion(), GetAppVersonSuffix());

#ifdef DEBUG
	SetLogLevel(LOG_LEVEL_DEBUG); // default log level for debug build
#endif

	LogSystemInfo();

	ParseCommandLine(argc, argv);

	// Using SDL_INIT_GAMECONTROLLER produces a load of annoying debug output spam
	Uint32 sdlInitFlags = SDL_INIT_VIDEO | SDL_INIT_TIMER /*| SDL_INIT_GAMECONTROLLER*/;
	if (SDL_Init(sdlInitFlags) != 0)
	{
		LOG_ERROR("SDL_Init failed with error: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	if (sdlInitFlags & SDL_INIT_GAMECONTROLLER)
	{
		// SDL_INIT_GAMECONTROLLER causes a load of xbox debug spam without a newline
		LogMsg(stdout, "\n"); // Calls OutputDebugString internally
	}

	if (!FileSystem::Init())
	{
		LOG_ERROR("FileSystem::Init failed\n");
		return EXIT_FAILURE;
	}

	Displays::Enumerate();

	// Decide GL+GLSL versions
	// SDL_GL_SetAttribute must be called before window creation
#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100
	const char* glsl_version = "#version 100";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
	// GL 3.2 Core + GLSL 150
	const char* glsl_version = "#version 150";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

	// From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_Window* pWindow = createWindow();
	if (!pWindow)
	{
		LOG_ERROR("Failed to create window\n");
		return EXIT_FAILURE;
	}

	SDL_GLContext gl_context = SDL_GL_CreateContext(pWindow);
	SDL_GL_MakeCurrent(pWindow, gl_context);
	SDL_GL_SetSwapInterval(1); // Enable vsync

	if (!ImGuiWrap::Init(gl_context, glsl_version))
	{
		LOG_ERROR("Failed to initialise ImGui\n");
		return EXIT_FAILURE;
	}

	if (!HoffGui::Init())
	{
		LOG_ERROR("HoffGui failed to initialise\n");
		return EXIT_FAILURE;
	}

	// Main loop
	bool done = false;
#ifdef __EMSCRIPTEN__
	// For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
	// You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
	io.IniFilename = nullptr;
	EMSCRIPTEN_MAINLOOP_BEGIN
#else
	while (!done)
#endif
	{
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGuiWrap::ProcessEvent(event);

			if (event.type == SDL_QUIT)
				done = true;
			else if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_ESCAPE && s_quitOnEscape)
					done = true;
			}
			else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(pWindow))
				done = true;
		}

		ImFont* pDefaultFont = Fonts::GetFont(g_options.view.defaultFontType);
		ImGuiWrap::NewFrame(pDefaultFont);

		if (HoffGui::Update() == false)
			done = true;

		ImVec4 clearColor = HoffGui::GetClearColor();
		ImGuiWrap::Render(clearColor);
	}
#ifdef __EMSCRIPTEN__
	EMSCRIPTEN_MAINLOOP_END;
#endif

	HoffGui::Shutdown();

	ImGuiWrap::Shutdown();

	SDL_GL_DeleteContext(gl_context);
	Window::Destroy();
	pWindow = nullptr;

	FileSystem::Shutdown();

	SDL_Quit();

	return EXIT_SUCCESS;
}
