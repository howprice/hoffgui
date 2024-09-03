#pragma once

#include <stdio.h>

#ifndef HP_ASSERTS_ENABLED  // allow HP_ASSERTS_ENABLED to be defined on the compiler command line
#ifdef RELEASE
#define HP_ASSERTS_ENABLED 1 // enable asserts and fatal errors in release. This is a user-facing tool.
#else
#define HP_ASSERTS_ENABLED 1
#endif
#endif

#ifndef HP_WRITE_CRASH_LOG
#define HP_WRITE_CRASH_LOG 0
#endif

// #TODO: Update this to support Apple silicon
#if defined _MSC_VER
#define HP_BREAK __debugbreak();
#elif defined __arm__ // 32bit arm, and 32bit arm only.
#define HP_BREAK __builtin_trap();
#elif defined __aarch64__ // 64bit arm, and 64bit arm only.
#define HP_BREAK __builtin_trap();
#elif defined __GNUC__
#define HP_BREAK __asm__ ("int $3");
#else
#error Unsupported compiler
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments" // error: token pasting of ',' and __VA_ARGS__ is a GNU extension [-Werror,-Wgnu-zero-variadic-macro-arguments]
#endif

void HpAssertMessage(const char* expr, const char* type, const char* file, int line, const char* func, const char* format = nullptr, ...);

#ifndef HP_ASSERTS_ENABLED
#error HP_ASSERTS_ENABLED must be defined to 0 or 1
#endif

#if HP_ASSERTS_ENABLED

// http://cnicholson.net/2009/02/stupid-c-tricks-adventures-in-assert/
#define HP_ASSERT(expr, ...) \
	do \
	{ \
		if( !(expr) ) \
		{ \
			HpAssertMessage(#expr, "ASSERT", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
		HP_BREAK \
		} \
	} while (0)

#define HP_FATAL_ERROR(...) \
	do \
	{ \
		HpAssertMessage("", "FATAL ERROR", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
		HP_BREAK \
	} while (0)

#else

// Support macros without braces e.g.
//     if (whatever)
//         HP_ASSERT(something);

#define HP_ASSERT(expr, ...) \
	do \
	{ \
	} while (0)

#define HP_FATAL_ERROR(...) \
	do \
	{ \
	} while (0)

#endif

// this macro is always defined
#define HP_VERIFY(expr, ...) \
	do \
	{ \
		if( !(expr) ) \
		{ \
			HpAssertMessage(#expr, "VERIFY FAIL", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
		HP_BREAK \
		} \
	} while (0)

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#if HP_WRITE_CRASH_LOG
// Returns the version number for crash log.
// Application should implement this.
const char* HpAssertGetApplicationVersionNumberString();
#endif
