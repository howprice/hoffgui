#include "SystemInfo.h"

#include "Core/Log.h"

#include "SDL.h"

#ifdef _MSC_VER
#include <Windows.h>

//
// https://stackoverflow.com/a/48749703
// 
void LogSystemInfo()
{
	const char* platform = SDL_GetPlatform();
	LOG_INFO("Platform: %s\n", platform);

	OSVERSIONINFOEX osVersionInfoEx = {};

	// Function pointer to driver function
	NTSTATUS(WINAPI * pRtlGetVersion)(
		PRTL_OSVERSIONINFOW lpVersionInformation) = NULL;

	// load the System-DLL
	HINSTANCE hNTdllDll = LoadLibrary("ntdll.dll");

	// successfully loaded?
	if (hNTdllDll != NULL)
	{
		// get the function pointer to RtlGetVersion
		pRtlGetVersion = (NTSTATUS(WINAPI*)(PRTL_OSVERSIONINFOW))
			GetProcAddress(hNTdllDll, "RtlGetVersion");

		// if successfull then read the function
		if (pRtlGetVersion != NULL)
			pRtlGetVersion((PRTL_OSVERSIONINFOW)&osVersionInfoEx);

		// free the library
		FreeLibrary(hNTdllDll);
	} // if (hNTdllDll != NULL)

#if 0 // no longer available
	// if function failed, use fallback to old version
	if (pRtlGetVersion == NULL)
		GetVersionEx((OSVERSIONINFO*)&osVersionInfoEx);
#endif

	LOG_INFO("Windows %u.%u (OS Build %u)\n", osVersionInfoEx.dwMajorVersion, osVersionInfoEx.dwMinorVersion, osVersionInfoEx.dwBuildNumber);
}
#else

void LogSystemInfo()
{
	const char* platform = SDL_GetPlatform();
	LOG_INFO("Platform: %s\n", platform);
}

#endif
