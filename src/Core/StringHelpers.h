#pragma once

#include "Core/hp_assert.h"

#include <string.h>

#ifdef _MSC_VER
#define strtok_r strtok_s
#else
int _vscprintf(const char * format, va_list pargs);
#endif

//
// Substitue for SDL_strlcat to remove dependency on SDL
// Returns the size of the *string* that would have been written if the buffer was large enough
//
size_t Strlcat(char* dst, const char* src, size_t dstSize);

//
// Asserts if the destination buffer is too small
// Use instead of SDL_strlcpy, which truncates the output without any indication
// n.b. Parameter order differs from SDL_strlcpy(char *dst, const char *src, size_t maxlen);
//
void SafeStrcpy(char* dst, size_t dstSize, const char* src);

//
// Asserts if the destination buffer is too small
// Ensures dst string is null terminated
// Use instead of strcpy to avoid GCC warning: 'strncpy' specified bound depends on the length of the source argument
//
void SafeStrncpy(char* dst, size_t dstSize, const char* src, size_t count);

//
// Asserts if the output was truncated
// Use instead of SDL_snprintf, which silently truncates the output.
//
bool SafeSnprintf(char* buffer, size_t bufferSize, const char* format, ...);
bool SafeVsnprintf(char* buffer, size_t bufferSize, const char* format, va_list argList);

bool SafeStrcat(char* dst, size_t dstSize, const char* src);

inline unsigned int constexpr strlen_constexpr(const char* str)
{
	return *str ? 1 + strlen_constexpr(str + 1) : 0;
}

inline size_t CountLeadingWhitespace(const char* line)
{
	HP_ASSERT(line != nullptr);

	// See https://stackoverflow.com/questions/43592911/first-index-of-a-non-whitespace-character-in-astring-in-c
	const char whitespace[] = " \f\n\r\t\v";
	size_t count = strspn(line, whitespace);
	return count;
}

// Standard libary isascii includes all characters from 0 to 127.
// It is more likely that these are metadata than ASCII, so limit to range from 0x20
// (space) to 0x7E (~)
// n.b. NULL (0) IS included because used for null-terminated strings
inline bool IsASCII(char c)
{
	return c == 0x00 || (c >= 0x20 && c <= 0x7e);
}

// Standard libary isalpha takes an int
inline bool IsAlpha(char c)
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

// Standard libary isalpha takes an int
inline bool IsLowercaseAlpha(char c)
{
	return (c >= 'a' && c <= 'z');
}

inline bool IsLowercaseHexDigit(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
}
