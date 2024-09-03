#pragma once

#include "Core/Helpers.h"

//
// Manages displays aka monitors
//
class Displays
{
public:
	NON_INSTANTIABLE_STATIC_CLASS(Displays);

	static const unsigned int kMaxDisplays = 16;

	static void Enumerate();
	static unsigned int GetCount();
	static float GetDpiScale(unsigned int displayIndex);
};
