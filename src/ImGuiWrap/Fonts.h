#pragma once

#include "Core/Helpers.h"

struct ImFont;

enum class FontType
{
	// Pixel perfect fonts
	ProggyClean13, // 100% (1x13)
	ProggyClean26, // 200% (2x13)
	ProggyClean39, // 300% (3x13)
	ProggyTiny10 , // 100% (1x10)
	ProggyTiny20 , // 200% (2x10)
	ProggyTiny30 , // 300% (3x10)
	Topaz16,       // 100% (1x16)
	Topaz32,	   // 200% (2x16)
	Topaz48,	   // 300% (3x16)
	ProggyVector,  // Scalable

	Max = ProggyVector
};

inline const char* kFontTypeNames[] =
{
	"ProggyCleanTTSZ 13pt",
	"ProggyCleanTTSZ 26pt",
	"ProggyCleanTTSZ 39pt",
	"ProggyTinyTTSZ 10pt",
	"ProggyTinyTTSZ 20pt",
	"ProggyTinyTTSZ 30pt",
	"Topaz Unicode KS13 16pt",
	"Topaz Unicode KS13 32pt",
	"Topaz Unicode KS13 48pt",
	"Proggy Vector (scalable)",
};
static_assert(COUNTOF_ARRAY(kFontTypeNames) == ENUM_COUNT(FontType));

//
// Manages ImGui fonts
//
// See:
// - imgui/docs/FAQ.md, especially Q: How should I handle DPI in my application?
// - imgui/docs/FONTS.md
//
class Fonts
{
public:
	NON_INSTANTIABLE_STATIC_CLASS(Fonts);

	static bool Load(float zoomFactor);
	static void Clear();

	static ImFont* GetFont(FontType fontType);
};
