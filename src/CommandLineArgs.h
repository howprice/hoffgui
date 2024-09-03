#pragma once

struct CommandLineArgs
{
	bool ignoreIniFile = false;

	int displayIndex = -1;

	bool hasWindowX = false;
	int windowX = 0;

	bool hasWindowY = false;
	int windowY = 0;

	unsigned int windowWidth = 0;
	unsigned int windowHeight = 0;

	bool hasMaximised = false;
	bool maximised = false;

	bool hasFullscreen = false;
	bool fullscreen = false;
};

void PrintUsage();
void ParseCommandLine(int argc, char** argv);
const CommandLineArgs& GetCommandLineArgs();
