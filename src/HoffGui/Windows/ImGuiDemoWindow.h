#pragma once

#include "Core/Helpers.h"

struct ImVec4;

class ImGuiDemoWindow
{
	NON_INSTANTIABLE_STATIC_CLASS(ImGuiDemoWindow);
public:

	struct Options
	{
		bool showAboutWindow = false;
	};

	// Call once per frame
	static void Update(ImVec4& clearColor);

	static bool IsVisible();
	static void SetVisible(bool visible);

	static Options& GetOptions();
};
