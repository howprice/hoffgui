#include "ProcessWrap.h"

#include "Core/Log.h"
#include "Core/hp_assert.h"

#include "SDL.h" // SDL_strlcat

#include <stdlib.h> // EXIT_SUCCESS, EXIT_FAILURE

//------------------------------------------------------------------------------------------------
//
// argv is null terminated
//
static bool argsToCommandLine(const char* argv[], char* commandLine, size_t bufferSizeBytes)
{
	HP_ASSERT(argv != nullptr);
	HP_ASSERT(commandLine);

	commandLine[0] = '\0';
	unsigned int i = 0;
	while (argv[i] != NULL)
	{
		if (i > 0)
		{
			if (SDL_strlcat(commandLine, " ", bufferSizeBytes) >= bufferSizeBytes)
			{
				LOG_ERROR("Buffer overflow in argsToCommandLine\n");
				return false;
			}

		}
		if (SDL_strlcat(commandLine, argv[i], bufferSizeBytes) >= bufferSizeBytes)
		{
			LOG_ERROR("Buffer overflow in argsToCommandLine\n");
			return false;
		}
		i++;
	}

	return true;
}

#ifdef _MSC_VER

#include <Windows.h> //_splitpath_s, _makepath_s
#include <comdef.h> // _com_error

static const unsigned int kBufferSize = 4096;

static HANDLE s_hChildStdErrRead;
static char s_stderrBuffer[kBufferSize + 1] = {};
static DWORD s_stderrStatus;
static OVERLAPPED s_stderrOverlapped;

static HANDLE s_hChildStdOutRead;
static char s_stdoutBuffer[kBufferSize + 1] = {};
static DWORD s_stdoutStatus;
static OVERLAPPED s_stdoutOverlapped;

static volatile long s_pipeSerialNumber;

//------------------------------------------------------------------------------------------------

// From https://stackoverflow.com/a/419736
/*++
Routine Description:
	The CreatePipeEx API is used to create an anonymous pipe I/O device.
	Unlike CreatePipe FILE_FLAG_OVERLAPPED may be specified for one or
	both handles.
	Two handles to the device are created.  One handle is opened for
	reading and the other is opened for writing.  These handles may be
	used in subsequent calls to ReadFile and WriteFile to transmit data
	through the pipe.
Arguments:
	lpReadPipe - Returns a handle to the read side of the pipe.  Data
		may be read from the pipe by specifying this handle value in a
		subsequent call to ReadFile.
	lpWritePipe - Returns a handle to the write side of the pipe.  Data
		may be written to the pipe by specifying this handle value in a
		subsequent call to WriteFile.
	lpPipeAttributes - An optional parameter that may be used to specify
		the attributes of the new pipe.  If the parameter is not
		specified, then the pipe is created without a security
		descriptor, and the resulting handles are not inherited on
		process creation.  Otherwise, the optional security attributes
		are used on the pipe, and the inherit handles flag effects both
		pipe handles.
	nSize - Supplies the requested buffer size for the pipe.  This is
		only a suggestion and is used by the operating system to
		calculate an appropriate buffering mechanism.  A value of zero
		indicates that the system is to choose the default buffering
		scheme.
Return Value:
	TRUE - The operation was successful.
	FALSE/NULL - The operation failed. Extended error status is available
		using GetLastError.
--*/
BOOL
APIENTRY
MyCreatePipeEx(
	OUT LPHANDLE lpReadPipe,
	OUT LPHANDLE lpWritePipe,
	IN LPSECURITY_ATTRIBUTES lpPipeAttributes,
	IN DWORD nSize,
	DWORD dwReadMode,
	DWORD dwWriteMode
)
{
	HANDLE ReadPipeHandle, WritePipeHandle;
	DWORD dwError;
	char PipeNameBuffer[MAX_PATH];

	//
	// Only one valid OpenMode flag - FILE_FLAG_OVERLAPPED
	//

	if ((dwReadMode | dwWriteMode) & (~FILE_FLAG_OVERLAPPED)) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	//
	//  Set the default timeout to 120 seconds
	//

	if (nSize == 0) {
		nSize = 4096;
	}

	sprintf_s(PipeNameBuffer,
		"\\\\.\\Pipe\\RemoteExeAnon.%08x.%08x",
		GetCurrentProcessId(),
		InterlockedIncrement(&s_pipeSerialNumber)
	);

	ReadPipeHandle = CreateNamedPipeA(
		PipeNameBuffer,
		PIPE_ACCESS_INBOUND | dwReadMode,
		PIPE_TYPE_BYTE | PIPE_WAIT,
		1,             // Number of pipes
		nSize,         // Out buffer size
		nSize,         // In buffer size
		120 * 1000,    // Timeout in ms
		lpPipeAttributes
	);

	if (!ReadPipeHandle) {
		return FALSE;
	}

	WritePipeHandle = CreateFileA(
		PipeNameBuffer,
		GENERIC_WRITE,
		0,                         // No sharing
		lpPipeAttributes,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | dwWriteMode,
		NULL                       // Template file
	);

	if (INVALID_HANDLE_VALUE == WritePipeHandle) {
		dwError = GetLastError();
		CloseHandle(ReadPipeHandle);
		SetLastError(dwError);
		return FALSE;
	}

	*lpReadPipe = ReadPipeHandle;
	*lpWritePipe = WritePipeHandle;
	return(TRUE);
}

//------------------------------------------------------------------------------------------------

static void CALLBACK stdErrReadCompleted(const DWORD errorCode, const DWORD bytesRead, OVERLAPPED* pOverlapped)
{
	s_stderrStatus = errorCode;

	if (errorCode != ERROR_SUCCESS)
		return;

	HP_ASSERT(bytesRead < COUNTOF_ARRAY(s_stderrBuffer));
	s_stderrBuffer[bytesRead] = '\0';
	LogMsg(stderr, s_stderrBuffer);
	s_stderrBuffer[0] = '\0';

	if (!ReadFileEx(s_hChildStdErrRead, s_stderrBuffer, /*nNumberOfBytesToRead*/kBufferSize, pOverlapped, stdErrReadCompleted))
	{
		// ERROR_BROKEN_PIPE: The pipe has been ended.
		// Occurs when the pipe is empty and the child process has termiated
		s_stderrStatus = GetLastError();
		HRESULT hr = HRESULT_FROM_WIN32(s_stderrStatus);
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		LOG_TRACE("ReadFileEx stderr GetLastError() = %u, HR = 0x%X, %s\n", s_stderrStatus, hr, errMsg);
	}
}

static void CALLBACK stdOutReadCompleted(const DWORD errorCode, const DWORD bytesRead, OVERLAPPED* pOverlapped)
{
	s_stdoutStatus = errorCode;

	if (errorCode != ERROR_SUCCESS)
		return;

	HP_ASSERT(bytesRead < COUNTOF_ARRAY(s_stdoutBuffer));
	s_stdoutBuffer[bytesRead] = '\0';
	LogMsg(stdout, s_stdoutBuffer);
	s_stdoutBuffer[0] = '\0';

	if (!ReadFileEx(s_hChildStdOutRead, s_stdoutBuffer, /*nNumberOfBytesToRead*/kBufferSize, pOverlapped, stdOutReadCompleted))
	{
		// ERROR_BROKEN_PIPE: The pipe has been ended.
		// Occurs when the pipe is empty and the child process has termiated
		s_stdoutStatus = GetLastError();
		HRESULT hr = HRESULT_FROM_WIN32(s_stdoutStatus);
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		LOG_TRACE("ReadFileEx stdout GetLastError() = %u, HR = 0x%X, %s\n", s_stdoutStatus, hr, errMsg);
	}
}

//------------------------------------------------------------------------------------------------

//
// https://learn.microsoft.com/en-gb/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output
// https://stackoverflow.com/questions/56499041/capture-output-from-console-program-with-overlapping-and-events
//
static unsigned int launchProcess(const char* argv[])
{
	HP_ASSERT(argv && argv[0]);

	char commandLine[2048]; // command lines can be very long if path to executable and path to any passed files are long
	if (!argsToCommandLine(argv, commandLine, sizeof(commandLine)))
	{
		LOG_ERROR("Failed to convert process arguments to command line\n");
		return EXIT_FAILURE;
	}
		
	// Create pipes for the child process's stdout and stderr
	// ira seems to write to both
	SECURITY_ATTRIBUTES pipeAttributes;
	pipeAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	pipeAttributes.bInheritHandle = TRUE; // Set the bInheritHandle flag so pipe handles are inherited. 
	pipeAttributes.lpSecurityDescriptor = NULL;

	s_hChildStdOutRead = NULL;  // Allows child processes stdout to be read back by parent process
	HANDLE hChildStdOutWrite = NULL;
	if (!MyCreatePipeEx(&s_hChildStdOutRead, &hChildStdOutWrite, &pipeAttributes, kBufferSize, /*dwReadMode*/FILE_FLAG_OVERLAPPED, /*dwWriteMode*/FILE_FLAG_OVERLAPPED))
	{
		HP_FATAL_ERROR("MyCreatePipeEx failed\n");
	}

	s_hChildStdErrRead = NULL;  // Allows child processes stderr to be read back by parent process
	HANDLE hChildStdErrWrite = NULL;
	if (!MyCreatePipeEx(&s_hChildStdErrRead, &hChildStdErrWrite, &pipeAttributes, kBufferSize, /*dwReadMode*/FILE_FLAG_OVERLAPPED, /*dwWriteMode*/FILE_FLAG_OVERLAPPED))
	{
		HP_FATAL_ERROR("MyCreatePipeEx failed\n");
	}

	// Ensure the read handle to the pipe for STDOUT is *not* inherited.
	if (!SetHandleInformation(s_hChildStdOutRead, HANDLE_FLAG_INHERIT, 0))
	{
		HP_FATAL_ERROR("SetHandleInformation failed.");
	}

	// Ensure the read handle to the pipe for STDERR is *not* inherited.
	if (!SetHandleInformation(s_hChildStdErrRead, HANDLE_FLAG_INHERIT, 0))
	{
		HP_FATAL_ERROR("SetHandleInformation failed.");
	}

	STARTUPINFO startupInfo;
	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);

	startupInfo.dwFlags |= STARTF_USESTDHANDLES;
	startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	startupInfo.hStdOutput = hChildStdOutWrite;
	startupInfo.hStdError = hChildStdErrWrite;

	startupInfo.dwFlags |= STARTF_USESHOWWINDOW;
	startupInfo.wShowWindow = SW_HIDE; // Prevents cmd window from flashing. Requires STARTF_USESHOWWINDOW in dwFlags.

	PROCESS_INFORMATION processInformation;
	ZeroMemory(&processInformation, sizeof(processInformation));

	LOG_INFO("Creating process: %s\n", commandLine);

	if (!CreateProcess(
		NULL,             // n.b. The lpApplicationName parameter can be NULL. In that case, the module name must be the first white space–delimited token in the lpCommandLine string.
		(LPSTR)commandLine,
		/*lpProcessAttributes*/NULL,
		/*lpThreadAttributes*/NULL,
		/*bInheritHandles*/TRUE,     // handles are inherited *IMPORTANT*
		/*dwCreationFlags*/0,
		/*lpEnvironment*/NULL,       // Use parent's environment block
		/*lpCurrentDirectory*/NULL,  // Use parent's current directory. #TODO: May want to allow user to specify the working directory.
		&startupInfo,
		/*out*/&processInformation)
		)
	{
		DWORD error = GetLastError();
		HRESULT hr = HRESULT_FROM_WIN32(error);
		if (error == ERROR_FILE_NOT_FOUND)
			HP_FATAL_ERROR("ERROR_FILE_NOT_FOUND for command line: %s\n", commandLine);
		else
			HP_FATAL_ERROR("CreateProcess failed (%d) HR = 0x%08X for command line: %s\n", error, hr, commandLine);
	}

	// Close the write end of the pipe before reading from the read end of the pipe.
	// After the child process inherits the write handle, the parent process no longer needs its copy.
	CloseHandle(hChildStdOutWrite);
	CloseHandle(hChildStdErrWrite);

	// Read output from the child process's pipe for STDOUT and write to the parent process's pipe for STDOUT. 
	// Stop when there is no more data.

	// read stderr
	s_stderrStatus = ERROR_SUCCESS;
	s_stderrOverlapped = {};
	if (!ReadFileEx(s_hChildStdErrRead, s_stderrBuffer, /*nNumberOfBytesToRead*/kBufferSize, &s_stderrOverlapped, stdErrReadCompleted))
	{
		// ERROR_BROKEN_PIPE: The pipe has been ended.
		// Occurs when the pipe is empty and the child process has termiated
		s_stderrStatus = GetLastError();
		HRESULT hr = HRESULT_FROM_WIN32(s_stderrStatus);
		HP_UNUSED(hr);
	}

	// read stdout
	s_stdoutStatus = ERROR_SUCCESS;
	s_stdoutOverlapped = {};
	if (!ReadFileEx(s_hChildStdOutRead, s_stdoutBuffer, /*nNumberOfBytesToRead*/kBufferSize, &s_stdoutOverlapped, stdOutReadCompleted))
	{
		// ERROR_BROKEN_PIPE: The pipe has been ended.
		// Occurs when the pipe is empty and the child process has termiated
		s_stdoutStatus = GetLastError();
		HRESULT hr = HRESULT_FROM_WIN32(s_stdoutStatus);
		HP_UNUSED(hr);
	}

	// #TODO: Don't block main thread while ira process is running. Maybe add state machine.
	while (s_stderrStatus == ERROR_SUCCESS || s_stdoutStatus == ERROR_SUCCESS)
	{
		// Wait until one or more APCs are queued.
		const DWORD sleepResult = ::SleepEx(
			INFINITE, // Suspend indefinitely
			TRUE); // Alertable

		// Since the thread is suspended indefinitely it will only return
		// after one or more APCs are called.
		HP_ASSERT(WAIT_IO_COMPLETION == sleepResult);
	}

	// Wait until child process exits.
	WaitForSingleObject(processInformation.hProcess, INFINITE);

	DWORD exitCode;
	GetExitCodeProcess(processInformation.hProcess, &exitCode);

	CloseHandle(s_hChildStdOutRead);
	s_hChildStdOutRead = NULL;
	CloseHandle(s_hChildStdErrRead);
	s_hChildStdErrRead = NULL;

	// Close process and thread handles. 
	CloseHandle(processInformation.hProcess);
	CloseHandle(processInformation.hThread);

	return exitCode;
}
#else

#include <string.h> // strerr
#include <unistd.h> // fork
//#include <spawn.h> // posix_spawn
#include <sys/wait.h> // waitpid https://www.gnu.org/software/libc/manual/html_node/Process-Completion.html
#include <errno.h>

#define READ_END 0
#define WRITE_END 1

static const unsigned int kBufferSize = 4096;
static char s_childOutputBuffer[kBufferSize];

//
// Ref:
// - https://www.rozmichelle.com/pipes-forks-dups/
// - https://stackoverflow.com/a/5884588
//

// WIFEXITED(status) returns true if the child terminated normally, that is, by calling exit(3) or _exit(2), or by returning from main().
// WEXITSTATUS(status) returns the exit status of the child.  This consists of the least significant 8 bits of the status argument that the child specified in a call to exit(3) or _exit(2) or as the argument for a return statement in main().  This macro should only be employed if WIFEXITED returned true.
// WIFSIGNALED(status) returns true if the child process was terminated by a signal.
// WTERMSIG(status) returns the number of the signal that caused the child process to terminate.  This macro should only be employed if WIFSIGNALED returned true.
// WCOREDUMP(status) returns true if the child produced a core dump.  This macro should only be employed if WIFSIGNALED returned true.  This macro is not specified in POSIX.1-2001 and is not available on some UNIX implementations (e.g., AIX, SunOS).  Only use this enclosed in #ifdef WCOREDUMP ... #endif.
// WIFSTOPPED(status) returns true if the child process was stopped by delivery of a signal; this is only possible if the call was done using WUNTRACED or when the child is being traced (see ptrace(2)).
// WSTOPSIG(status) returns the number of the signal which caused the child to stop.  This macro should only be employed if WIFSTOPPED returned true.
// WIFCONTINUED(status) (since Linux 2.6.10) returns true if the child process was resumed by delivery of SIGCONT.
// https://stackoverflow.com/a/19151751
//
static void printChildExitReason(pid_t cid, int status)
{
	if (WIFEXITED(status))
	{
		LOG_TRACE("child %d terminated normally, that is, by calling exit(3) or _exit(2), or by returning from main().\n", cid);
		if (WEXITSTATUS(status))
		{
			LOG_TRACE("child %d exit status %d.  This consists of the least significant 8 bits of the status argument that the child specified in a call to exit(3) or _exit(2) or as the argument for a return statement in main().\n", cid, WEXITSTATUS(status));
		}
	}
	if (WIFSIGNALED(status))
	{
		LOG_TRACE("child %d process was terminated by a signal.\n", cid);
		if (WTERMSIG(status))
		{
			LOG_TRACE("child %d signal %d that caused the child process to terminate.\n", cid, WTERMSIG(status));
		}
		if (WCOREDUMP(status))
		{
			LOG_TRACE("child %d produced a core dump.  WCOREDUMP() is not specified in POSIX.1-2001 and is not available on some UNIX implementations (e.g., AIX, SunOS).  Only use this enclosed in #ifdef WCOREDUMP ... #endif.\n", cid);
		}
	}
	if (WIFSTOPPED(status))
	{
		LOG_TRACE("child %d process was stopped by delivery of a signal; this is only possible if the call was done using WUNTRACED or when the child is being traced (see ptrace(2)).\n", cid);
		if (WSTOPSIG(status))
		{
			LOG_TRACE("child %d number of the signal which caused the child to stop.\n", cid);
		}
	}
	if (WIFCONTINUED(status))
	{
		LOG_TRACE("child %d process was resumed by delivery of SIGCONT.\n", cid);
	}
}

static unsigned int launchProcess(const char* argv[])
{
	char commandLine[2048];
	if (!argsToCommandLine(argv, commandLine, sizeof(commandLine)))
	{
		LOG_ERROR("Failed to convert process arguments to command line\n");
		return EXIT_FAILURE;
	}

	LOG_INFO("Creating process: %s\n", commandLine);

	HP_ASSERT(argv[0]);

	// pipe() creates a pipe and two associated file descriptors.
	// These can be used to redirect the child process stdout to parent process stdin
	// fds[0] is the read end of the pipe (from the perspective of the process)
	// fds[1] is the write end of the pipe (from the perspective of the process)
	int pipeFileDescs[2];
	if (pipe(pipeFileDescs) == -1)
	{
		perror("pipe");
		return EXIT_FAILURE;
	}

	// fork() creates a clone of the parent’s memory state and file descriptors.
	pid_t pid = fork();
	if (pid == -1)
	{
		LOG_ERROR("fork() failed\n");
		return EXIT_FAILURE;
	}

	// After the fork() call, changes to the parent process will not be visible to the child 
	// process and vice versa.

	if (pid == 0) // fork() returns zero for the child process
	{
		// The child process does not need to read from the pipe, so that file descriptor can be closed.
		close(pipeFileDescs[READ_END]);

		// Connect child process stdout stream to the write end of the pipe.
		// dup2() copies a file descriptor, closing the descriptor in the destination slot first.
		// n.b. dup2() closes the pre-existing stdout file descriptor.
		dup2(pipeFileDescs[WRITE_END], STDOUT_FILENO);

		// Connect child process stderr stream to the write end of the pipe too
		dup2(pipeFileDescs[WRITE_END], STDERR_FILENO);

		// The descriptor has been copied into place and copy in the original slot can be closed.
		close(pipeFileDescs[WRITE_END]);

		// The exec family of functions execute a file.
		// They replace the current process image with a new process image.
		// n.b. execve() does NOT return on success, and the text, data, bss, and stack of the calling 
		// process are overwritten by that of the program loaded. 
		//
		// These functions are front-ends for execve: execl, execlp, execle, execv, execvp, execvpe 
		// - Want a 'v' function, which take an array of pointers to null terminated strings
		// - Don't want to use a 'p' variety, because always want to use executable next to parent executable, not in system directories
		// - Don't need an 'e' variety, which specifies the environment
		// -> execv is what we want: array of args, path to executable, no environment specified

		// Arguments for execv:
		// - First argument should be the filename i.e. argv[0]
		// - Final argument must be NULL pointer to char

		HP_ASSERT(argv[0] && argv[0][0]);
		LOG_TRACE("Child: execv\n");
		fflush(NULL);
		execv(argv[0], (char *const*)argv); // n.b. need execv*p* for system util ls	

		// if we get here something horribly bad happened
		LOG_ERROR("Child process execv failed: %s\n", strerror(errno));
		HP_FATAL_ERROR("Child process execv failed: %s", strerror(errno));
		exit(1); 
	}

	// Parent process
	LOG_TRACE("Child process ID: %i\n", pid);

	// The parent process does not need to write to the pipe, so that file descriptor can be closed
	close(pipeFileDescs[WRITE_END]);

	// n.b. Don't wait for thc child to exit here. 
	// Pipe can only hold 65536 bytes and ira can produce more than that in stdout+stderr, 
	// so need to drain pipe. then wait for the child process to exit.
	// #TODO: Run asynchronously so GUI remains responsive and Output Window shows progress.

	LOG_TRACE("Capturing child process redirected stdout and stderr\n");
	ssize_t totalBytesRead = 0;
	for (;;)
	{
		ssize_t bytesRead = read(pipeFileDescs[READ_END], s_childOutputBuffer, sizeof(s_childOutputBuffer) - 1);
//		LOG_TRACE("Read %u bytes from child process\n", (unsigned int)bytesRead); // disabled; creates too much spam
		if (bytesRead == 0) // EOF?
			break;

		HP_ASSERT((unsigned int)bytesRead < sizeof(s_childOutputBuffer));
		s_childOutputBuffer[bytesRead] = '\0';

		LogMsg(stderr, s_childOutputBuffer);
		s_childOutputBuffer[0] = '\0';

		totalBytesRead += bytesRead;
	}

	LOG_TRACE("Total bytes read from child process: %u\n", (unsigned int)totalBytesRead);

	// No further need to read from the pipe
	close(pipeFileDescs[READ_END]);

	int status;
	errno = 0;
	pid_t wpid;
	do
	{
		wpid = waitpid(pid, &status, 0);
	} while (wpid == -1 && errno == EINTR); // Mac fix. See https://stackoverflow.com/a/10160656

	LOG_TRACE("waitpid returned %d\n", wpid);
	if (wpid == -1)
	{
		LOG_ERROR("waitpid failed: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	LOG_TRACE("Child exited with status %i\n", status);
	printChildExitReason(pid, status);

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

#endif

unsigned int Process::Launch(const char* argv[])
{
	return launchProcess(argv);
}
