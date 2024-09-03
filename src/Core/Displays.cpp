#include "Displays.h"

#include "Core/Log.h"

#include "imgui/imgui.h" // ImVec2 #TODO: Remove me to reduce build time and dependencies

#include "SDL.h"

// From imgui_impl_sdl2.cpp 
#define SDL_HAS_USABLE_DISPLAY_BOUNDS       SDL_VERSION_ATLEAST(2,0,5)
#define SDL_HAS_PER_MONITOR_DPI             SDL_VERSION_ATLEAST(2,0,4)

struct Display
{
	// Coordinates of the area displayed on this monitor (Min = upper left, Max = bottom right)
	ImVec2 pos = {};
	ImVec2 size = {};

	// Coordinates without task bars / side bars / menu bars. Used to avoid positioning popups/tooltips inside this region. If you don't have this info, please copy the value for MainPos/MainSize.
	ImVec2 usablePos = {};
	ImVec2 usableSize = {};

	// DPI aware applications render at native resolution. The OS does not upscale, which can
	// lead to blurriness for non integer scaling.
	float dpiScale = 1.0f; // 1.0f = 96 DPI
};

static ImVector<Display> s_displays;

//------------------------------------------------------------------------------------------------
//
// Adapted from ImGui_ImplSDL2_UpdateMonitors so doesn't require imgui to be initialised
//
// Monitor info can be seen in in the ImGui Demo Window under metrics
// 
void Displays::Enumerate()
{
	s_displays.resize(0);
	int display_count = SDL_GetNumVideoDisplays();
	LOG_TRACE("Num displays: %d\n", display_count);
	for (int displayIndex = 0; displayIndex < display_count; displayIndex++)
	{
		const char* displayName = SDL_GetDisplayName(displayIndex);
		LOG_TRACE("Display %u \"%s\"\n", displayIndex, displayName);

		// Warning: the validity of monitor DPI information on Windows depends on the application DPI awareness settings, which generally needs to be set in the manifest or at runtime.
		Display display;
		SDL_Rect rect;
		SDL_GetDisplayBounds(displayIndex, &rect);
		display.pos = ImVec2((float)rect.x, (float)rect.y);
		display.size = ImVec2((float)rect.w, (float)rect.h);
		LOG_TRACE("  Display Bounds: (%d, %d) (%d, %d)\n", rect.x, rect.y, rect.w, rect.h);
#if SDL_HAS_USABLE_DISPLAY_BOUNDS
		SDL_GetDisplayUsableBounds(displayIndex, &rect);
		display.usablePos = ImVec2((float)rect.x, (float)rect.y);
		display.usableSize = ImVec2((float)rect.w, (float)rect.h);
		LOG_TRACE("  Display Usable Bounds: (%d, %d) (%d, %d)\n", rect.x, rect.y, rect.w, rect.h);
#else
		display.usablePos = display.pos;
		display.usableSize = display.size;
		LOG_TRACE("  Display Usable Bounds not available\n");
#endif

#if SDL_HAS_PER_MONITOR_DPI
		// FIXME-VIEWPORT: On MacOS SDL reports actual monitor DPI scale, ignoring OS configuration. We may want to set
		//  DpiScale to cocoa_window.backingScaleFactor here.
		float dpi = 0.0f;
		if (SDL_GetDisplayDPI(displayIndex, &dpi, nullptr, nullptr) == 0)
		{
			LOG_TRACE("  DPI: %.1f\n", dpi);

			display.dpiScale = dpi / 96.0f;
			LOG_TRACE("  DPI Scale: %.2f (%.0f%%)\n", display.dpiScale, 100.0f * display.dpiScale);
		}
		else
			LOG_ERROR("  SDL_GetDisplayDPI failed: %s\n", SDL_GetError());
#else
		LOG_TRACE("  SDL_GetDisplayDPI not available\n");
#endif
		s_displays.push_back(display);
	}
}

unsigned int Displays::GetCount()
{
	return s_displays.size();
}

float Displays::GetDpiScale(unsigned int displayIndex)
{
#ifdef __APPLE__
	// DPI scaling is not currently supported on Mac
	// See https://github.com/ocornut/imgui/issues/3757
	HP_UNUSED(displayIndex);
	return 1.0f;
#else
	return s_displays[displayIndex].dpiScale;
#endif
}
