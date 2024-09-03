#include "Parse.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h> // GCC/Clang
#include <limits.h> // GCC/Clang

bool ParseInt(const char* arg, int& val)
{
	errno = 0;
	char* pEnd = NULL;
	long longVal = strtol(arg, &pEnd, 0);
	if (arg == pEnd || errno != 0 || longVal == LONG_MIN || longVal == LONG_MAX)
	{
		return false;
	}

	val = (int)longVal;
	return true;
}

bool ParseUnsignedInt(const char* arg, unsigned int& val)
{
	errno = 0;
	char* pEnd = NULL;
	long int iVal = strtol(arg, &pEnd, 0);
	if (iVal < 0 || arg == pEnd || errno != 0 || iVal == LONG_MIN || iVal == LONG_MAX)
	{
		return false;
	}
	val = (unsigned int)iVal;
	return true;
}

bool ParseHexUnsignedInt(const char* arg, unsigned int& val)
{
	errno = 0;
	char* pEnd = NULL;
	long int iVal = strtol(arg, &pEnd, 16);
	if (iVal < 0 || arg == pEnd || errno != 0 || iVal == LONG_MIN || iVal == LONG_MAX)
	{
		return false;
	}
	val = (unsigned int)iVal;
	return true;
}

char* ParseHelpers::GetLine(char* s, char** ppNextLine)
{
	if (s == nullptr)
		s = *ppNextLine;

	if (*s == '\0')
		return nullptr;

	char* pLine = s;

	// find the end of the line
	s = strchr(s, '\n'); // #TODO: Roll own strchr if expensive
	if (s == nullptr)
		*ppNextLine = nullptr;
	else
	{
		// null-terminate the line
		*s = '\0';
		*ppNextLine = s + 1;
	}

	return pLine;
}
