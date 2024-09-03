#pragma once

#include "Core/Helpers.h"

#include "imgui/imgui.h"

#ifdef _MSC_VER
#define _PRISizeT   "I"
#else
#define _PRISizeT   "z"
#endif

typedef union SDL_Event SDL_Event;

inline const float kDefaultFontSize = 13.0f; // Default ImGui font size is 13 for DPI 96 PPI

//
// ImGui DPI-aware helper to convert width or height to pixels
// Pass width or height in units of font size (height)
// Default font size is 13 for DPI 96 PPI (100% Desktop Scaling) USER_DEFAULT_SCREEN_DPI = 96
//
#define DIM_FONT_UNITS(x) (x) * ImGui::GetFontSize()

#define VEC2_FONT_UNITS(x,y) ImVec2(DIM_FONT_UNITS(x), DIM_FONT_UNITS(y))

// 96 DPI helpers
// Allow a dimension to be specified in pixels for 96 DPI (100% Desktop Scaling)
#define DIM_96_PPI(x) DIM_FONT_UNITS((x) / kDefaultFontSize)
#define VEC2_96_PPI(x,y) ImVec2(DIM_96_PPI(x), DIM_96_PPI(y))

inline const ImGuiTableFlags kDefaultTableFlags =
	  ImGuiTableFlags_SizingFixedFit
	| ImGuiTableFlags_Borders
	| ImGuiTableFlags_NoHostExtendX;

inline float kZoomFactors[] = { 1.0f, 1.1f, 1.25f, 1.5f, 1.75f, 2.0f, 2.25f, 2.5f, 2.75f, 3.0f };

class ImGuiWrap
{
public:

	NON_INSTANTIABLE_STATIC_CLASS(ImGuiWrap);

	static bool Init(void* gl_context, const char* glsl_version);
	static void Shutdown();

	static bool ProcessEvent(const SDL_Event& event);
	static void NewFrame(ImFont* pFont);
	static void Render(const ImVec4& clearColor);

	static bool CanIncreaseZoom();
	static void IncreaseZoom();
	static bool CanDecreaseZoom();
	static void DecreaseZoom();

	static float GetZoom();
	static unsigned int GetZoomFactorIndex();
	static void SetZoomFactorIndex(unsigned int zoomFactorIndex);
};


//
// Add helper functions to ImGui namespace
//
namespace ImGui
{
void HelpMarker(const char* fmt, ...);

void CopyHexByteToClipboard(ImU8 val);
void CopyHexWordToClipboard(ImU16 val);
void CopyHexLongToClipboard(ImU32 val);
void PushStyleCompact();
void PopStyleCompact();

int FindWindowFocusIndexByName(const char* name);
};
