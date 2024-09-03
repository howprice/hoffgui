#pragma once

#include "Core/Helpers.h"

bool ParseInt(const char* arg, int& val);

bool ParseUnsignedInt(const char* arg, unsigned int& val);

[[nodiscard]]
bool ParseHexUnsignedInt(const char* arg, unsigned int& val);

class ParseHelpers
{
public:
	NON_INSTANTIABLE_STATIC_CLASS(ParseHelpers);

	//
	// Specialised strtok, with fixed \n delimiter, and don't skip multiple delimiters to ensure consistent line numbering.
	// 
	// n.b. Lke strtok, this is destructive. \n will be replace with \0
	//
	static char* GetLine(char* s, char** ppNextLine);
};
