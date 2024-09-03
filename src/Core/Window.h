#pragma once

#include "Core/Helpers.h"

// Only allow fullscreen in release builds because can cause OS to become unresponsive if hit breakpoint while in fullscreen
// #TODO: I think there is a Windows keyboard shortcut to avoid this, but can't remember what it is. Maybe related to switching dekstop.
#ifdef RELEASE
#define FULLSCREEN_ALLOWED 1
#else
#define FULLSCREEN_ALLOWED 0
#endif

struct SDL_Window;

class Window
{
public:
	// Currently the application only needs a single window, so using a static class for simplicity.
	NON_INSTANTIABLE_STATIC_CLASS(Window);

	// 1920x1080 is too large for some monitors and will be larger than the desktop
	// Microsoft recommends default window size of 800 x 600 or 1024 x 768, but these are 4:3 and small
	// 1024x600 (WSVGA) is a sensible compromise. Wide aspect ration and not too small
	static const unsigned int kDefaultWidth = 1024;
	static const unsigned int kDefaultHeight = 600;

	static bool Create(const char* title, int x, int y, unsigned int w, unsigned int h, bool fullscreen, bool maximised);
	static void Destroy();

	static SDL_Window& GetSDLWindow();

	static void SetSubtitle(const char* subtitle);

	static void GetPosition(int& x, int& y);
	static void SetPosition(int x, int y);

	static void GetSize(unsigned int& w, unsigned int& h);
	static void SetSize(unsigned int w, unsigned int h);

	static int GetDisplayIndex();
	static bool IsMaximised();
	static bool IsMinimised();

	static bool IsFullscreen();
	static void SetFullscreen(bool fullscreen);
};
