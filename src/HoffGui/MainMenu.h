#pragma once

#include "Core/Helpers.h"

class MainMenu
{
public:
	NON_INSTANTIABLE_STATIC_CLASS(MainMenu);

	// Call once per frame
	static void Update(bool& quit);

	static bool IsVisible();
	static void SetVisible(bool visible);
};
