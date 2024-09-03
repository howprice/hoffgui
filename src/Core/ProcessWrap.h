#pragma once

#include "Core/Helpers.h"

class Process
{
public:
	NON_INSTANTIABLE_STATIC_CLASS(Process);

	// argv[] must be null terminated
	// returns return code e.g. EXIT_SUCCESS
	static unsigned int Launch(const char* argv[]);
};
