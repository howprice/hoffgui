#pragma once

#include "Core/Helpers.h"

struct ImVec4;

class HoffGui
{
public:
	NON_INSTANTIABLE_STATIC_CLASS(HoffGui);

	static bool Init();
	static void Shutdown();

	// call once per frame
	// returns false when finished i.e. user quit
	static bool Update();

	static void ResetWindowLayout();

	static const ImVec4& GetClearColor();
};
