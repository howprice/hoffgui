#include "hp_assert.h"

#include "Core/Log.h"
#include "Core/StringHelpers.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h> // malloc, free

void HpAssertMessage(const char* expr, const char* type, const char* file, int line, const char* func, const char* format /*= nullptr*/, ...)
{
	unsigned int messageBufferSize = 1; // default to '\0'
	char* message = nullptr;

	if (format)
	{
		va_list argList;
		va_start(argList, format);
		messageBufferSize = _vscprintf(format, argList) * sizeof(char) + 1; // + 1 to null terminate
		va_end(argList);

		message = (char*)malloc(messageBufferSize);
		
		va_start(argList, format);
		vsnprintf(message, messageBufferSize, format, argList); // Populate the buffer with the contents of the format string.
		va_end(argList);
	}

	LogMsg(stderr,
		"\n"
		"------------------------------------------------------------------------------------------------------------\n"
		"%20s:\t%s\n"
		"%20s:\t%s\n"
		"%20s:\t%s\n"
		"%20s:\t%s(%d)\n"
		"------------------------------------------------------------------------------------------------------------\n\n",
		type, expr, 
		"Message", message ? message : "",
		"Function", func,
		"File", file, line
	);

#if HP_WRITE_CRASH_LOG
	// Write the assert message to a log file.
	// Open file in append mode to maintain a complete history of all assert messages over time.
	static const char kLogFilename[] = "hoffguilog.txt";
	FILE* logFile = fopen(kLogFilename, "a");
	if (logFile != nullptr)
	{
		fprintf(logFile, "Version: %s\n", HpAssertGetApplicationVersionNumberString());

		fprintf(logFile,
			"\n"
			"------------------------------------------------------------------------------------------------------------\n"
			"%s:\t%s\n"
			"Message:\t%s\n"
			"Function:\t%s\n"
			"File:\t%s(%d)\n"
			"------------------------------------------------------------------------------------------------------------\n\n",
			type, expr,
			message ? message : "",
			func,
			file, line
		);
		fclose(logFile);

		LogMsg(stderr, "Assert logged to file: %s\n", kLogFilename);
	}
	logFile = nullptr;
#endif

	if (message != nullptr)
	{
		free(message);
	}
}
