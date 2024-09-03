// Notes from imgui:
//
// Tip: monospace fonts are convenient because they allow to facilitate horizontal alignment directly at the string level.
// 
// - If no fonts are loaded, dear imgui will use the default font.
// - You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
// - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application
//   (e.g. use an assertion, or display an error and quit).
// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling
//   ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
// - Read 'docs/FONTS.md' for more instructions and details.
// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
// - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.

#include "Fonts.h"

#include "Core/Displays.h"
#include "Core/FileSystem.h"
#include "Core/Log.h"

#include "imgui/imgui_internal.h" // ImFloor
#include "ImGuiWrap/ImGuiWrap.h"

static ImFont* s_pFonts[ENUM_COUNT(FontType)];

// ProggyClean.ttf (by Tristan Grimmer)
// Monospaced, 13 pixels high, pixel-perfect font
// Must only be used with integer scaling
static const char kProggyCleanFilename[] = "ProggyCleanSZ.ttf"; // slashed zero variant. #TODO: Source centered punctuation variant for disassembly ProggyCleanCP.ttf or ProggyCleanSZCP.ttf https://github.com/bluescan/proggyfonts/issues/9
static const unsigned int kProggyCleanFontSize = 13;

// ProggyTiny.ttf (by Tristan Grimmer)
// Monospaced, 10 pixels high, pixel-perfect font
static const char kProggyTinyFilename[] = "ProggyTinySZ.ttf";
static const unsigned int kProggyTinyFontSize = 10;

// Topaz (modern Amiga)
// https://gitlab.com/Screwtapello/topaz-unicode
// Monospaced, 16 pixels high (10x16) pixel-perfect font
// It looks great at multiples of 16, but terrible if scaled smaller, or non-integer scaled (24 is OK though)
static const char kTopazFilename[] = "topaz_unicode_ks13_regular.ttf";
static const unsigned int kTopazFontSize = 16;

static const unsigned int kMaxDpiScales = Displays::kMaxDisplays; // potentially different DPI scale for each display
static float s_dpiScales[kMaxDpiScales]; // this is a fraction, not a percentage
static unsigned int s_dpiScaleCount = 0;

// Proggy Vector (by Tristan Grimmer)
// Monospaced, scalable font 
// #TODO: Rebuild this font for whatever size the user wants
static const char kProggyVectorFilename[] = "ProggyVector-Regular.ttf";
static const unsigned int kProggyVectorFontSize = 14; // Minimum size at which looks OK.
static ImFont* s_pProggyVectorFonts[kMaxDpiScales];

//--------------------------------------------------------------------------------------------

static ImFont* addFontFromFileTTF(const char* fontsPath, const char* filename, float fontSizePixels, const ImFontConfig* pFontConfig = nullptr)
{
	char fontPath[kMaxPath] = {};
	FileSystem::MakePath(fontPath, sizeof(fontPath), fontsPath, filename); // e.g. "C:\GitHub\howprice\hoffgui\build\Debug\fonts\ProggyClean.ttf"

	ImGuiIO& io = ImGui::GetIO();
	ImFont* pFont = io.Fonts->AddFontFromFileTTF(fontPath, fontSizePixels, pFontConfig);
	if (pFont == nullptr)
	{
		LOG_ERROR("Failed to load font: %s\n", fontPath);
		return nullptr;
	}

	LOG_TRACE("Loaded font: %s size %.2f\n", fontPath, fontSizePixels);
	return pFont;
}

//
// From imgui FAQ.md:
// Default is ProggyClean.ttf, monospace, rendered at size 13, embedded in dear imgui's source code.
//
static bool addPixelPerfectFonts(const char* fontsPath)
{
	// ProggyClean
	// See ProggyClean loading in ImFontAtlas.AddFontDefault and imgui/docs/FONTS.md

	// ProggyClean13
	ImFontConfig fontConfig;
	fontConfig.SizePixels = 1.0f * (float)kProggyCleanFontSize;
	fontConfig.OversampleH = 1; // no oversampling
	fontConfig.OversampleV = 1; // no oversampling
	fontConfig.PixelSnapH = true; // pixel perfect
	fontConfig.EllipsisChar = (ImWchar)0x0085;
	fontConfig.GlyphOffset.y = 1.0f * IM_TRUNC(fontConfig.SizePixels / kProggyCleanFontSize);  // Add +1 offset per 13 units
	s_pFonts[ToNumber(FontType::ProggyClean13)] = addFontFromFileTTF(fontsPath, kProggyCleanFilename, fontConfig.SizePixels, &fontConfig);
	if (s_pFonts[ToNumber(FontType::ProggyClean13)] == nullptr)
	{
		LOG_ERROR("Failed to add font\n");
		return false;
	}

	// ProggyClean26
	fontConfig.SizePixels = 2.0f * (float)kProggyCleanFontSize;
	fontConfig.GlyphOffset.y = 1.0f * IM_TRUNC(fontConfig.SizePixels / kProggyCleanFontSize);  // Add +1 offset per 13 units
	s_pFonts[ToNumber(FontType::ProggyClean26)] = addFontFromFileTTF(fontsPath, kProggyCleanFilename, fontConfig.SizePixels, &fontConfig);
	if (s_pFonts[ToNumber(FontType::ProggyClean26)] == nullptr)
	{
		LOG_ERROR("Failed to add font\n");
		return false;
	}

	// ProggyClean39
	fontConfig.SizePixels = 3.0f * (float)kProggyCleanFontSize;
	fontConfig.GlyphOffset.y = 1.0f * IM_TRUNC(fontConfig.SizePixels / kProggyCleanFontSize);  // Add +1 offset per 13 units
	s_pFonts[ToNumber(FontType::ProggyClean39)] = addFontFromFileTTF(fontsPath, kProggyCleanFilename, fontConfig.SizePixels, &fontConfig);
	if (s_pFonts[ToNumber(FontType::ProggyClean39)] == nullptr)
	{
		LOG_ERROR("Failed to add font\n");
		return false;
	}

	// ProggyTiny10
	fontConfig.SizePixels = 1.0f * (float)kProggyTinyFontSize;
	fontConfig.EllipsisChar = (ImWchar)0x0085;
	fontConfig.GlyphOffset.y = 1.0f * IM_TRUNC(fontConfig.SizePixels / kProggyTinyFontSize);  // Add +1 offset per 10 units
	s_pFonts[ToNumber(FontType::ProggyTiny10)] = addFontFromFileTTF(fontsPath, kProggyTinyFilename, fontConfig.SizePixels, &fontConfig);
	if (s_pFonts[ToNumber(FontType::ProggyTiny10)] == nullptr)
	{
		LOG_ERROR("Failed to add font\n");
		return false;
	}

	// ProggyTiny20
	fontConfig.SizePixels = 2.0f * (float)kProggyTinyFontSize;
	fontConfig.GlyphOffset.y = 1.0f * IM_TRUNC(fontConfig.SizePixels / kProggyTinyFontSize);  // Add +1 offset per 10 units
	s_pFonts[ToNumber(FontType::ProggyTiny20)] = addFontFromFileTTF(fontsPath, kProggyTinyFilename, fontConfig.SizePixels, &fontConfig);
	if (s_pFonts[ToNumber(FontType::ProggyTiny20)] == nullptr)
	{
		LOG_ERROR("Failed to add font\n");
		return false;
	}

	// ProggyTiny30
	fontConfig.SizePixels = 3.0f * (float)kProggyTinyFontSize;
	fontConfig.GlyphOffset.y = 1.0f * IM_TRUNC(fontConfig.SizePixels / kProggyTinyFontSize);  // Add +1 offset per 10 units
	s_pFonts[ToNumber(FontType::ProggyTiny30)] = addFontFromFileTTF(fontsPath, kProggyTinyFilename, fontConfig.SizePixels, &fontConfig);
	if (s_pFonts[ToNumber(FontType::ProggyTiny30)] == nullptr)
	{
		LOG_ERROR("Failed to add font\n");
		return false;
	}

	// Topaz16
	fontConfig.SizePixels = 1.0f * (float)kTopazFontSize;
	fontConfig.EllipsisChar = (ImWchar)0x002e; // No ellipsis character in font, so use . instead
	fontConfig.GlyphOffset.y = 1.0f * IM_TRUNC(fontConfig.SizePixels / kTopazFontSize);  // Add +1 offset per 16 units
	s_pFonts[ToNumber(FontType::Topaz16)] = addFontFromFileTTF(fontsPath, kTopazFilename, fontConfig.SizePixels, &fontConfig);
	if (s_pFonts[ToNumber(FontType::Topaz16)] == nullptr)
	{
		LOG_ERROR("Failed to add font\n");
		return false;
	}

	// Topaz32
	fontConfig.SizePixels = 2.0f * (float)kTopazFontSize;
	fontConfig.GlyphOffset.y = 1.0f * IM_TRUNC(fontConfig.SizePixels / kTopazFontSize);  // Add +1 offset per 16 units
	s_pFonts[ToNumber(FontType::Topaz32)] = addFontFromFileTTF(fontsPath, kTopazFilename, fontConfig.SizePixels, &fontConfig);
	if (s_pFonts[ToNumber(FontType::Topaz32)] == nullptr)
	{
		LOG_ERROR("Failed to add font\n");
		return false;
	}

	// Topaz48
	fontConfig.SizePixels = 3.0f * (float)kTopazFontSize;
	fontConfig.GlyphOffset.y = 1.0f * IM_TRUNC(fontConfig.SizePixels / kTopazFontSize);  // Add +1 offset per 16 units
	s_pFonts[ToNumber(FontType::Topaz48)] = addFontFromFileTTF(fontsPath, kTopazFilename, fontConfig.SizePixels, &fontConfig);
	if (s_pFonts[ToNumber(FontType::Topaz48)] == nullptr)
	{
		LOG_ERROR("Failed to add font\n");
		return false;
	}

	// #TODO: Topaz bold

	return true;
}

//
// From imgui FAQ.md:
//
// Q: How should I handle DPI in my application?
// A: The short answer is: obtain the desired DPI scale, load your fonts resized with that scale (always
// round down fonts size to the nearest integer), and scale your Style structure accordingly using `style.ScaleAllSizes()`.
//
static bool addScalableFonts(const char* fontsPath, float zoomFactor)
{
	// build array of unique DPI scales
	const unsigned int displayCount = Displays::GetCount();
	s_dpiScaleCount = 0;
	for (unsigned int displayIndex = 0; displayIndex < displayCount; displayIndex++)
	{
		const float dpiScale = Displays::GetDpiScale(displayIndex) * zoomFactor;
		bool found = false;
		for (unsigned int i = 0; i < s_dpiScaleCount; i++)
		{
			if (s_dpiScales[i] == dpiScale)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			HP_ASSERT(s_dpiScaleCount < COUNTOF_ARRAY(s_dpiScales));
			s_dpiScales[s_dpiScaleCount++] = dpiScale;
		}
	}

	// Create a scalable font for each DPI scale, so can support imgui viewports on different windows simultaneously
	for (unsigned int dpiScaleIndex = 0; dpiScaleIndex < s_dpiScaleCount; dpiScaleIndex++)
	{
		const float dpiScale = s_dpiScales[dpiScaleIndex];
		LOG_TRACE("Creating font for DPI scale: %.2f (%.0f%%)\n", dpiScale, 100.0f * dpiScale);

		float fontSizePixels = ImFloor((float)kProggyVectorFontSize * dpiScale); // round down to nearest integer (ImGui advice)

		// ProggyVector
		HP_ASSERT(s_pProggyVectorFonts[dpiScaleIndex] == nullptr);
		s_pProggyVectorFonts[dpiScaleIndex] = addFontFromFileTTF(fontsPath, kProggyVectorFilename, fontSizePixels);
		if (s_pProggyVectorFonts[dpiScaleIndex] == nullptr)
		{
			LOG_ERROR("Failed to add font\n");
			return false;
		}

		// TEST
		if (dpiScaleIndex == 0)
			s_pFonts[ToNumber(FontType::ProggyVector)] = s_pProggyVectorFonts[dpiScaleIndex];
	}

	return true;
}

bool Fonts::Load(float zoomFactor)
{
	// Fonts are next to executable
	char fontsPath[kMaxPath] = {};
	static const char* kDefaultFontsDir = "fonts"; // relative to application directory i.e. next to executable
	FileSystem::MakePath(fontsPath, sizeof(fontsPath), FileSystem::GetApplicationDirectory(), kDefaultFontsDir); // e.g. "C:\dev\howprice\hoffgui\build\Debug\fonts"

//	ImGuiIO& io = ImGui::GetIO();
//	io.Fonts->AddFontDefault();  // Disabled: Want to load fonts manually

	// Examples from imgui:
	//io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f *dpiScale); // not monospaced
	//io.Fonts->AddFontFromFileTTF("fonts/DroidSans.ttf", 16.0f * dpiScale); // not monospaced
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != nullptr);

	if (!addPixelPerfectFonts(fontsPath))
	{
		LOG_ERROR("Failed to load pixel perfect fonts\n");
		return false;
	}

	if (!addScalableFonts(fontsPath, zoomFactor))
	{
		LOG_ERROR("Failed to load scalable fonts\n");
		return false;
	}

	return true;
}

void Fonts::Clear()
{
	for (unsigned int i = 0; i < COUNTOF_ARRAY(s_pFonts); i++)
	{
		s_pFonts[i] = nullptr;
	}

	for (unsigned int i = 0; i < COUNTOF_ARRAY(s_pProggyVectorFonts); i++)
	{
		s_pProggyVectorFonts[i] = nullptr;
	}

	ImGui::GetIO().Fonts->Clear();
}

ImFont* Fonts::GetFont(FontType fontType)
{
	return s_pFonts[ToNumber(fontType)];
}
