#pragma once

const char* GetAppVersion();
unsigned int GetAppVersionMajor();
unsigned int GetAppVersionMinor();
unsigned int GetAppVersionPatch();
unsigned int GetAppVersionTweak();

inline const char* GetAppVersonSuffix()
{
	return "-WIP";
//	return "-alpha";
//	return "-beta";
//	return ""; // release!
}
