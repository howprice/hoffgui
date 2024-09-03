#include "CommandLineArgs.h"

#include "Utils/Parse.h"
#include "Core/Window.h"
#include "Core/Log.h"

#include <stdio.h>
#include <stdlib.h> // errno
#include <string.h>
#include <errno.h> // errno (Linux)
#include <limits.h>

static CommandLineArgs s_commandLineArgs;

const CommandLineArgs& GetCommandLineArgs()
{
	return s_commandLineArgs;
}

void PrintUsage()
{
	puts(  "Usage: hoffgui [OPTIONS]");
	puts(  "Options:\n"
		   "  --help                                Shows this message\n"
		   "  --log-level <value>                   Specify log level: 2 (trace), 1 (debug), 0 (info), -1 (warn), -2 (error) -3 (none)  Default: 0"
	);
	puts(  "  -d --display-index                    Window display (monitor) index Default = 0 (primary)\n");
	puts(  "  -x --window-x                         Window pos x. Default: centred\n");
	puts(  "  -y --window-y                         Window pos y. Default: centred\n");
	puts(  "  -w --window-width                     Window width. Default: 75%% display width\n");
	puts(  "  -h --window-height                    Window height. Default: 75%% display height\n");
	puts(  "  -m --maximised                        Window maximised Default: false\n");
	puts(  "  -f --fullscreen                       Full screen\n");
	puts(  "  -ignore-ini-file                      Don't load hoffgui.ini\n");
}

void ParseCommandLine(int argc, char** argv)
{
	for (int i = 1; i < argc; i++)
	{
		const char* arg = argv[i];

		if (strcmp(arg, "--help") == 0)
		{
			PrintUsage();
			exit(EXIT_SUCCESS);
		}
		else if (strcmp(arg, "--log-level") == 0)
		{
			if (i + 1 == argc)
			{
				PrintUsage();
				exit(EXIT_FAILURE);
			}

			arg = argv[++i];
			int logLevel;
			if (!ParseInt(arg, logLevel) || logLevel < LOG_LEVEL_MIN || logLevel > LOG_LEVEL_MAX)
			{
				fprintf(stderr, "ERROR: Invalid log-level value\n");
				PrintUsage();
				exit(EXIT_FAILURE);
			}
			SetLogLevel(logLevel);
		}
		else if (strcmp(arg, "--ignore-ini-file") == 0)
		{
			s_commandLineArgs.ignoreIniFile = true;
		}
		else if (strcmp(arg, "-d") == 0 || strcmp(arg, "--display-index") == 0)
		{
			if (i + 1 == argc)
			{
				PrintUsage();
				exit(EXIT_FAILURE);
			}

			arg = argv[++i];
			if (!ParseInt(arg, s_commandLineArgs.displayIndex))
			{
				fprintf(stderr, "ERROR: Specified display index is invalid\n");
				PrintUsage();
				exit(EXIT_FAILURE);
			}
		}
		else if (strcmp(arg, "-x") == 0 || strcmp(arg, "--window-x") == 0)
		{
			if (i + 1 == argc)
			{
				PrintUsage();
				exit(EXIT_FAILURE);
			}

			arg = argv[++i];
			if (!ParseInt(arg, s_commandLineArgs.windowX))
			{
				fprintf(stderr, "ERROR: Specified window x is invalid\n");
				PrintUsage();
				exit(EXIT_FAILURE);
			}
			s_commandLineArgs.hasWindowX = true;
		}
		else if (strcmp(arg, "-y") == 0 || strcmp(arg, "--window-y") == 0)
		{
			if (i + 1 == argc)
			{
				PrintUsage();
				exit(EXIT_FAILURE);
			}

			arg = argv[++i];
			if (!ParseInt(arg, s_commandLineArgs.windowY))
			{
				fprintf(stderr, "ERROR: Specified window y is invalid\n");
				PrintUsage();
				exit(EXIT_FAILURE);
			}
			s_commandLineArgs.hasWindowY = true;
		}
		else if (strcmp(arg, "-w") == 0 || strcmp(arg, "--window-width") == 0)
		{
			if (i + 1 == argc)
			{
				PrintUsage();
				exit(EXIT_FAILURE);
			}

			arg = argv[++i];
			if (!ParseUnsignedInt(arg, s_commandLineArgs.windowWidth))
			{
				fprintf(stderr, "ERROR: Specified window width is invalid\n");
				PrintUsage();
				exit(EXIT_FAILURE);
			}
		}
		else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--window-height") == 0)
		{
			if (i + 1 == argc)
			{
				PrintUsage();
				exit(EXIT_FAILURE);
			}

			arg = argv[++i];
			if (!ParseUnsignedInt(arg, s_commandLineArgs.windowHeight))
			{
				fprintf(stderr, "ERROR: Specified window height is invalid\n");
				PrintUsage();
				exit(EXIT_FAILURE);
			}
		}
		else if (strcmp(arg, "--maximised") == 0 || strcmp(arg, "-m") == 0)
		{
			s_commandLineArgs.hasMaximised = true;
			s_commandLineArgs.maximised = true;
		}
		else if (strcmp(arg, "--fullscreen") == 0 || strcmp(arg, "-f") == 0)
		{
			s_commandLineArgs.hasFullscreen = true;
			s_commandLineArgs.fullscreen = true;
		}
		else
		{
			fprintf(stderr, "Unrecognised command line arg: %s\n", arg);
			PrintUsage();
			exit(EXIT_FAILURE);
		}
	}
}
