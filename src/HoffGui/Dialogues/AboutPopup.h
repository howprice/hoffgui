#pragma once

#include "Core/Helpers.h"

class AboutPopup
{
public:
	NON_INSTANTIABLE_STATIC_CLASS(AboutPopup);

	static constexpr char kName[] = "About";

	static void Update();
};
