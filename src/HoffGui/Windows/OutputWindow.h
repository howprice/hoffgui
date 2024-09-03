#pragma once

#include "ImGuiWrap/Fonts.h"

#include "Core/Helpers.h"

#include <stdarg.h> // va_list

class OutputWindow
{
public:
	NON_INSTANTIABLE_STATIC_CLASS(OutputWindow);

	static constexpr char kWindowName[] = "Output";

	struct Options
	{
		bool useDefaultFont = true;
		FontType fontType = FontType::ProggyClean13;
	};

	static void Init(Options* pOptions);
	static void Shutdown();

	static void Clear();
	static void AppendString(const char* str, size_t len);
	static void Printf(const char* format, ...);
	static void Vfprintf(const char* format, va_list argList);

	// Call once per frame
	static void Update();

	static bool IsVisible();
	static void SetVisible(bool visible);

	static void Focus();
};
