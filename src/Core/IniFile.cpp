#include "IniFile.h"

#include "Core/FileSystem.h"
#include "Core/Log.h"
#include "Core/hp_assert.h"
#include "Core/StringHelpers.h"

#include <string.h> // strchr
#include <stdlib.h> // strtoul
#include <stdarg.h> // va_list

//
// Specialised strtok, with fixed \n delimiter, and don't skip multiple delimiters to ensure consistent line numbering.
// 
// n.b. Lke strtok, this is destructive. \n will be replace with \0
//
static char* getLine(char* s, char** ppNextLine)
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


bool IniFile::Parse(const char* path, Handler* pHandler, void* pUserData)
{
	HP_ASSERT(pHandler != nullptr);

	FILE* pFile = fopen(path, "r");
	if (!pFile)
	{
		LOG_ERROR("Failed to open ini file for read: %s\n", path);
		return false;
	}

	// Read the entire ini file into memory

	LOG_TRACE("Opened ini file for parsing: %s\n", path);

	fseek(pFile, 0, SEEK_END);
	unsigned int fileSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	unsigned int bufferSize = fileSize + 1;
	char* pText = new char[bufferSize];
	size_t bytesRead = fread(pText, 1, fileSize, pFile);
	fclose(pFile);
	pFile = nullptr;

	HP_ASSERT(bytesRead < bufferSize);
	pText[bytesRead] = '\0';

	unsigned int lineIndex = 0;  // 0-based
	char* pNextLine = nullptr;
	char* pLineText = getLine(pText, &pNextLine);
	char* pSection = nullptr;
	while (pLineText)
	{
		const unsigned int lineNumber = lineIndex + 1; // 1-indexed. For error reporting. Text file line indices are 1-indexed by convention

		char c = pLineText[0];
		if (c == '\0' || c == '#' || c == ';') // Empty line or comment
		{
			// Ignore line
		}
		else if (c == '[')
		{
			// Section
			pSection = pLineText + 1;
			char* pEnd = strchr(pSection, ']');
			if (pEnd == nullptr)
			{
				LOG_ERROR("Ini file parse error: Missing closing ']' in section header on line %u\n", lineNumber);
				delete [] pText;
				pText = nullptr;
				return false;
			}
			*pEnd = '\0';
		}
		else
		{
			// Key/Value pair
			char* pKey = pLineText;
			char* pValue = strchr(pKey, '=');
			if (pValue == nullptr)
			{
				LOG_ERROR("Ini file parse error: Missing '=' in key/value pair on line %u\n", lineNumber);
				delete[] pText;
				pText = nullptr;
				return false;
			}
			*pValue = '\0';
			pValue++;

			// Trim whitespace
			while (*pKey == ' ' || *pKey == '\t')
				pKey++;
			char* pEnd = pKey + strlen(pKey) - 1;
			while (pEnd > pKey && (*pEnd == ' ' || *pEnd == '\t'))
				*pEnd-- = '\0';

			while (*pValue == ' ' || *pValue == '\t')
				pValue++;
			pEnd = pValue + strlen(pValue) - 1;
			while (pEnd > pValue && (*pEnd == ' ' || *pEnd == '\t'))
				*pEnd-- = '\0';

			// Call the handler
			if (!pHandler(pSection, pKey, pValue, pUserData, lineNumber))
			{
				LOG_TRACE("Failed to parse ini file key/value pair on line %u\n", lineNumber);
			}
		}

		lineIndex++;
		pLineText = getLine(nullptr, &pNextLine);
	}

	delete [] pText;
	pText = nullptr;

	return true;
}

void IniFile::WriteSection(FILE* pFile, const char* section)
{
	fprintf(pFile, "\n[%s]\n", section);
}

void IniFile::WriteComment(FILE* pStream, const char* format, ...)
{
	fprintf(pStream, "# ");

	va_list argList;
	va_start(argList, format);
	vfprintf(pStream, format, argList);
	va_end(argList);

	fprintf(pStream, "\n");
}

void IniFile::WriteBool(FILE* pFile, const char* key, bool val)
{
	fprintf(pFile, "%s=%s\n", key, val ? "1" : "0");
}

bool IniFile::ParseBool(const char* pValue)
{
	int x = strtol(pValue, nullptr, 10);
	return x != 0;
}

void IniFile::WriteInt(FILE* pFile, const char* key, int val)
{
	fprintf(pFile, "%s=%d\n", key, val);
}

int IniFile::ParseInt(const char* pValue)
{
	int x = strtol(pValue, nullptr, 10);
	return x;
}

void IniFile::WriteUint(FILE* pFile, const char* key, unsigned int val)
{
	fprintf(pFile, "%s=%u\n", key, val);
}

unsigned int IniFile::ParseUint(const char* pValue)
{
	unsigned int x = strtoul(pValue, nullptr, 10);
	return x;
}

void IniFile::WriteHex(FILE* pFile, const char* key, unsigned int val)
{
	fprintf(pFile, "%s=%08X\n", key, val);
}

unsigned int IniFile::ParseHex(const char* pValue)
{
	unsigned val = strtoul(pValue, nullptr, 16);
	return val;
}

void IniFile::WriteFloat(FILE* pFile, const char* key, float val)
{
	fprintf(pFile, "%s=%f\n", key, val);
}

float IniFile::ParseFloat(const char* pValue)
{
	float x = strtof(pValue, nullptr);
	return x;
}

void IniFile::WriteString(FILE* pFile, const char* key, const char* val)
{
	fprintf(pFile, "%s=%s\n", key, val);
}

void IniFile::ParseString(const char* pValue, char* buffer, size_t bufferSize)
{
	SafeStrcpy(buffer, bufferSize, pValue);
}
