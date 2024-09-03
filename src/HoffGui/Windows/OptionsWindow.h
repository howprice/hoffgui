#pragma once

#include "Core/Helpers.h"

//
// #TODO: Consider making Options window a modal dialogue. If so remove from WindowList so state is not serialised.
//
class OptionsWindow
{
public:
	NON_INSTANTIABLE_STATIC_CLASS(OptionsWindow);

	static constexpr char kWindowName[] = "Options";

	// Call once per frame
	static void Update();

	static void	Focus();

	static bool IsVisible();
	static void SetVisible(bool visible);
};
