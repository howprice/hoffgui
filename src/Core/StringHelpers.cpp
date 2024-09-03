#include "StringHelpers.h"

#include "Core/Log.h"

#include <stdarg.h>

#ifndef _MSC_VER

int _vscprintf(const char * format, va_list pargs)
{
	int retval;
	retval = vsnprintf(NULL, 0, format, pargs);
	return retval;
}

#endif

void SafeStrcpy(char* dst, size_t dstSize, const char* src)
{
	HP_ASSERT(dst != nullptr);
	HP_ASSERT(dstSize > 0);
	HP_ASSERT(src != nullptr);
	size_t srcLen = strlen(src); 
	if (dstSize < srcLen + 1) // include null-terminator
	{
		HP_FATAL_ERROR("Destination buffer too small for string copy");
		srcLen = dstSize - 1; // avoid buffer overflow if asserts are disabled
	}
	memcpy(dst, src, srcLen);
	dst[srcLen] = '\0';
}

void SafeStrncpy(char* dst, size_t dstSize, const char* src, size_t count)
{
	HP_ASSERT(dst != nullptr);
	HP_ASSERT(dstSize > 0);
	HP_ASSERT(src != nullptr);
	if (dstSize < count + 1) // include null-terminator
	{
		HP_FATAL_ERROR("Destination buffer too small for string copy");
		count = dstSize - 1; // avoid buffer overflow if asserts are disabled
	}
	memcpy(dst, src, count);
	dst[count] = '\0';
}

//
// SDL_snprintf seems to return the number of characters that would have been written if the buffer was large enough
// *not* including the null-terminator
// e.g. SDL_snprintf(buffer, sizeof(buffer), "%u", 1234) returns 4
// This is no good. The application and user needs to know if the output was truncated.
//
bool SafeSnprintf(char* buffer, size_t bufferSize, const char* format, ...)
{
	va_list argList;
	va_start(argList, format);
	bool ret = SafeVsnprintf(buffer, bufferSize, format, argList);
	va_end(argList);
	return ret;
}

bool SafeVsnprintf(char* buffer, size_t bufferSize, const char* format, va_list argList)
{
	int ret = vsnprintf(buffer, bufferSize, format, argList);
	if (ret < 0)
	{
		LOG_ERROR("SDL_vsnprintf failed with code: %d\n", ret);
		return false;
	}

	if (ret >= (int)bufferSize)
	{
		HP_FATAL_ERROR("SDL_vsnprintf output was truncated. Attempted to write %u characters (including null terminator) to buffer of size %u", ret + 1, bufferSize);
		return false;
	}

	return true;
}
//------------------------------------------------------------------------

size_t Strlcat(char* dst, const char* src, size_t dstSize)
{
	size_t srclen = strlen(src);
	size_t dstlen = strlen(dst);
	size_t totalLen = dstlen + srclen;

	if (dstlen >= dstSize)
		return totalLen; // already too long

	size_t copylen = dstSize - dstlen - 1; // include null-terminator
	if (copylen > srclen)
		copylen = srclen;

	memcpy(dst + dstlen, src, copylen);
	dst[dstlen + copylen] = '\0';

	return totalLen;
}

bool SafeStrcat(char* dst, size_t dstSize, const char* src)
{
	HP_ASSERT(dst != nullptr);
	HP_ASSERT(src != nullptr);

	// Strlcat returns the size of the *string* that would have been written if the buffer was large enough
	size_t len = Strlcat(dst, src, dstSize);
	if (dstSize < len + 1) // include null-terminator
	{
		HP_FATAL_ERROR("Destination buffer too small for string concatenation");
		return false;
	}

	return true;
}
