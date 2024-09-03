#pragma once

#include "Core/Log.h"
#include "Core/Helpers.h"

#include <stdio.h> // FILE
#include <stdlib.h> // strtoul

class IniFile
{
public:
	NON_INSTANTIABLE_STATIC_CLASS(IniFile);

	// Returns true if the value was parsed
	typedef bool Handler(const char* section, const char* key, const char* value, void* pUserData, unsigned int lineNumber);

	static bool Parse(const char* path, Handler* pHandler, void* pUserData);

	// Helpers

	static void WriteSection(FILE* pFile, const char* section);

	static void WriteComment(FILE* pStream, const char* format, ...);

	static void WriteBool(FILE* pFile, const char* key, bool val);
	static bool ParseBool(const char* pValue);

	static void WriteInt(FILE* pFile, const char* key, int val);
	static int ParseInt(const char* pValue);

	static void WriteUint(FILE* pFile, const char* key, unsigned int val);
	static unsigned int ParseUint(const char* pValue);

	static void WriteHex(FILE* pFile, const char* key, unsigned int val);
	static unsigned int ParseHex(const char* pValue);

	static void WriteFloat(FILE* pFile, const char* key, float val);
	static float ParseFloat(const char* pValue);

	static void WriteString(FILE* pFile, const char* key, const char* val);
	static void ParseString(const char* pValue, char* buffer, size_t bufferSize);

	template <typename T>
	static void WriteEnum(FILE* pFile, const char* key, T val);

	template <typename T>
	static T ParseEnum(const char* pValue);
};

template <typename T>
void IniFile::WriteEnum(FILE* pFile, const char* key, T val)
{
	fprintf(pFile, "%s=%d\n", key, ToNumber(val));
}

template <typename T>
T IniFile::ParseEnum(const char* pValue)
{
	unsigned int val = strtoul(pValue, nullptr, 10);
	if (val < ENUM_COUNT(T))
		return (T)val;
	else
	{
		LOG_ERROR("Invalid enum value: %s\n", pValue);
		return (T)0;
	}
}
