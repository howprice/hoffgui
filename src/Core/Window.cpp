#include "Window.h"

#include "Core/StringHelpers.h"
#include "Core/hp_assert.h"
#include "Core/Log.h"

#include "SDL.h"

static SDL_Window* s_pWindow;
static char s_title[256];
static unsigned int s_initialWidth;
static unsigned int s_initialHeight;

//-------------------------------------------------------------------------------------------------------

bool Window::Create(const char* title, int x, int y, unsigned int w, unsigned int h, bool fullscreen, bool maximised)
{
	HP_ASSERT(s_pWindow == nullptr);
	HP_ASSERT(title && title[0]);
	HP_ASSERT(w > 0);
	HP_ASSERT(h > 0);

	LOG_TRACE("CreateWindow x=%d y=%d, w=%u h=%u maximised=%u fullscreen=%u\n", x, y, w, h, maximised ? 1 : 0, fullscreen ? 1 : 0);

	Uint32 windowFlags = (SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

	if (fullscreen)
	{
		windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP; // fullscreen window at desktop resolution
		s_pWindow = SDL_CreateWindow(title,
			x, y,
			/*w*/0, /*h*/0, // If the window is set fullscreen, the width and height parameters w and h will not be used. https://wiki.libsdl.org/SDL2/SDL_CreateWindow
			windowFlags);
	}
	else
	{
		if (maximised)
			windowFlags |= SDL_WINDOW_MAXIMIZED;

		s_pWindow = SDL_CreateWindow(title, x, y, w, h, windowFlags);
	}

	if (s_pWindow == nullptr)
	{
		printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	s_initialWidth = w;
	s_initialHeight = h;

	SDL_GetWindowSize(s_pWindow, (int*)&w, (int*)&h);
	LOG_TRACE("SDL_GetWindowSize w=%d h=%d\n", w, h);

	SafeStrcpy(s_title, sizeof(s_title), title);

	return true;
}

void Window::Destroy()
{
	HP_ASSERT(s_pWindow != nullptr);
	SDL_DestroyWindow(s_pWindow);
}

SDL_Window& Window::GetSDLWindow()
{
	HP_ASSERT(s_pWindow != nullptr);
	return *s_pWindow;
}

void Window::SetSubtitle(const char* subtitle)
{
	HP_ASSERT(s_pWindow != nullptr);

	char title[256];
	if (subtitle && subtitle[0])
		SafeSnprintf(title, sizeof(title), "%s - %s", s_title, subtitle);
	else
		SafeStrcpy(title, sizeof(title), s_title);

	SDL_SetWindowTitle(s_pWindow, title);
}

void Window::GetPosition(int& x, int& y)
{
	HP_ASSERT(s_pWindow != nullptr);
	SDL_GetWindowPosition(s_pWindow, &x, &y);
}

void Window::SetPosition(int x, int y)
{
	HP_ASSERT(s_pWindow != nullptr);
	LOG_TRACE("SDL_SetWindowPosition x=%u y=%u\n", x, y);
	SDL_SetWindowPosition(s_pWindow, x, y);
}

void Window::GetSize(unsigned int& w, unsigned int& h)
{
	HP_ASSERT(s_pWindow != nullptr);
	SDL_GetWindowSize(s_pWindow, (int*)&w, (int*)&h);
}

void Window::SetSize(unsigned int w, unsigned int h)
{
	HP_ASSERT(s_pWindow != nullptr);
	HP_ASSERT(w > 0);
	HP_ASSERT(h > 0);
	LOG_TRACE("SDL_SetWindowSize w=%u h=%u\n", w, h);
	SDL_SetWindowSize(s_pWindow, (int)w, (int)h);
}

int Window::GetDisplayIndex()
{
	HP_ASSERT(s_pWindow != nullptr);
	int displayIndex = SDL_GetWindowDisplayIndex(s_pWindow);
	if (displayIndex < 0)
	{
		LOG_ERROR("SDL_GetWindowDisplayIndex failed: %s\n", SDL_GetError());
		return -1;
	}

	return displayIndex;
}

bool Window::IsMaximised()
{
	HP_ASSERT(s_pWindow != nullptr);
	Uint32 flags = SDL_GetWindowFlags(s_pWindow);
	return (flags & SDL_WINDOW_MAXIMIZED) == SDL_WINDOW_MAXIMIZED;
}

bool Window::IsMinimised()
{
	HP_ASSERT(s_pWindow != nullptr);
	Uint32 flags = SDL_GetWindowFlags(s_pWindow);
	return (flags & SDL_WINDOW_MINIMIZED) == SDL_WINDOW_MINIMIZED;
}

bool Window::IsFullscreen()
{
	HP_ASSERT(s_pWindow != nullptr);
	Uint32 flags = SDL_GetWindowFlags(s_pWindow);
	return (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP;
}

void Window::SetFullscreen(bool fullscreen)
{
	HP_ASSERT(s_pWindow != nullptr);
	LOG_TRACE("SetFullscreen fullscreen=%u\n", fullscreen ? 1 : 0);
	if (SDL_SetWindowFullscreen(s_pWindow, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) != 0)
	{
		LOG_ERROR("SDL_SetWindowFullscreen failed: %s\n", SDL_GetError());
	}

	// On Windows, if the Window is created in fullscreen, then fullscreen is disabled, the
	// window size can be set to (1,1) !
	if (!fullscreen)
	{
		int w, h;
		SDL_GetWindowSize(s_pWindow, (int*)&w, (int*)&h);
		LOG_TRACE("SDL_GetWindowSize w=%d h=%d\n", w, h);
		if (w == 1 && h == 1)
		{
			// Restore default window size and position
			SetSize(s_initialWidth, s_initialHeight);
			SDL_SetWindowPosition(s_pWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
		}
	}
}
