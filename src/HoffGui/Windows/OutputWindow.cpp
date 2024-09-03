#include "OutputWindow.h"

#include "Core/StringHelpers.h"
#include "Core/hp_assert.h"
#include "Core/Log.h"

#include "ImGuiWrap/ImGuiWrap.h"

#include <string.h> // memcpy
#include <stdlib.h> // malloc, free

#define DEBUG_RING_BUFFER 0

// Ring buffer
static const unsigned int kOutputBufferSizeBytes = 16 * 1024 * 1024;

static char s_outputBuffer[kOutputBufferSizeBytes];
static unsigned int s_startIndex; // read pos
static unsigned int s_endIndex;   // write pos
static unsigned int s_bytesWritten; // required to know if buffer is empty or full

static bool s_visible = true;
static bool s_focus;
static bool s_autoScroll = true;
static bool s_scrollToBottom = false;

static OutputWindow::Options* s_pOptions;

void OutputWindow::Init(Options* pOptions)
{
	HP_ASSERT(pOptions != nullptr);
	s_pOptions = pOptions;
}

void OutputWindow::Shutdown()
{
	s_pOptions = nullptr;
}

void OutputWindow::Clear()
{
	s_startIndex = 0;
	s_endIndex = 0;
	s_bytesWritten = 0;

	// no need to zero the buffer memory
}

static char getPrevChar(unsigned int index)
{
	unsigned int prevIndex = index > 0 ? index - 1 : kOutputBufferSizeBytes - 1;
	return s_outputBuffer[prevIndex];
}

//
// Increments the start index by a line (past next EOL or until it catches up with the end index)
// to make room for more writes
//
static void advanceStartIndex()
{
	bool reachedEOL = false;
	do
	{
		// Increment and wrap
		s_startIndex++;
		if (s_startIndex == kOutputBufferSizeBytes)
			s_startIndex = 0;

		char c = s_outputBuffer[s_startIndex];
		bool isEolChar = (c == '\r' || c == '\n');
		if (isEolChar)
			reachedEOL = true;
		else if (reachedEOL && !isEolChar)
			return; //  passed EOL

	} while (s_startIndex != s_endIndex);
}

static void appendChar(const char c)
{
	HP_ASSERT(s_endIndex <= kOutputBufferSizeBytes);
	if (s_endIndex == kOutputBufferSizeBytes)
		s_endIndex = 0; // wrap

	if (s_endIndex == s_startIndex && s_bytesWritten > 0) // buffer full?
		advanceStartIndex();

	// Append the character
	HP_ASSERT(s_endIndex < kOutputBufferSizeBytes);
	s_outputBuffer[s_endIndex++] = c;
	s_bytesWritten++;
}

void OutputWindow::AppendString(const char* str, size_t len)
{
	// Copy to Output buffer character by character
	// To support console style "progress bars", if a CR (\r 0xd) is found that is not followed by a LF (\n 0xa),
	// then back up to start of line. IRA uses this for percentages I think.
	for (size_t charIndex = 0; charIndex < len; charIndex++)
	{
		char c = str[charIndex];
		HP_ASSERT(c != 0);
		if (c == '\r') // CR?
		{
			bool isFinalChar = (charIndex == len - 1);
			if (isFinalChar || (str[charIndex + 1] != '\n'))
			{
				// Carriage Return back to start of current line, accounting for ring buffer wrapping
				while (s_endIndex != s_startIndex && getPrevChar(s_endIndex) != '\n')
				{
					s_endIndex = s_endIndex > 0 ? s_endIndex - 1 : kOutputBufferSizeBytes - 1;
					s_bytesWritten--;
				}
				continue; // nothing to append
			}
		}

		appendChar(c);
	}

	if (s_autoScroll)
		s_scrollToBottom = true;
}

void OutputWindow::Printf(const char* format, ...)
{
	HP_ASSERT(format != nullptr);

	va_list argList;
	va_start(argList, format);
	Vfprintf(format, argList);
	va_end(argList);
}

void OutputWindow::Vfprintf(const char* format, va_list argList)
{
	va_list argcopy; 
	va_copy(argcopy, argList);
	int ret = _vscprintf(format, argcopy) + 1; // + 1 to null terminate (_vscprintf return value doesn't include null-terminator)
	va_end(argcopy);
	if (ret == -1)
	{
		LOG_ERROR("_vscprintf failed with code %d\n", ret);
		return;
	}

	size_t bufferSize = (size_t)ret;

	char* buffer = (char*)malloc(bufferSize);

	// Populate the buffer with the contents of the format string.
	va_copy(argcopy, argList);
	vsnprintf(buffer, bufferSize, format, argcopy);
	va_end(argcopy);

	HP_ASSERT(bufferSize > 0 && (strlen(buffer) == bufferSize - 1));
	AppendString(buffer, bufferSize - 1);

	free(buffer);
}

void OutputWindow::Update()
{
	if (!s_visible)
		return;

	if (s_focus)
	{
		s_focus = false;
		ImGui::SetNextWindowFocus();
	}

	if (!ImGui::Begin(kWindowName, &s_visible))
	{
		ImGui::End();
		return;
	}

#if DEBUG_RING_BUFFER
	ImGui::Text("Size: %u  Start: %u  End: %u  Written: %u\n", kOutputBufferSizeBytes, s_startIndex, s_endIndex, s_bytesWritten);

	if (ImGui::Button("Append number"))
	{
		static unsigned int s_i = 0;
		char text[64];
		SafeSnprintf(text, sizeof(text), "%03u\n", s_i);
		size_t len = strlen(text);
		AppendString(text, len);
		s_i++;
	}
	ImGui::SameLine();
	if (ImGui::Button("Append string with CR"))
	{
		const char* text = "ABC\r";
		size_t len = strlen(text);
		AppendString(text, len);
	}
#endif

	bool copyToClipboard = false;
	if (ImGui::BeginPopupContextWindow())
	{
		if (ImGui::Selectable("Clear"))
			Clear();
		if (ImGui::Selectable("Copy"))
			copyToClipboard = true;
		ImGui::Checkbox("Auto-scroll", &s_autoScroll);
		if (ImGui::Selectable("Scroll to bottom"))
			s_scrollToBottom = true;
		ImGui::EndPopup();
	}

	if (copyToClipboard)
		ImGui::LogToClipboard();

	const bool useDefaultFont = s_pOptions->useDefaultFont;
	if (!useDefaultFont)
		ImGui::PushFont(Fonts::GetFont(s_pOptions->fontType));

	// Depending on the state of the ring buffer, there will be 0, 1 or 2 ranges to display
	if (s_bytesWritten == 0)
	{
		// nothing to display
	}
	else if (s_startIndex < s_endIndex)
		ImGui::TextUnformatted(s_outputBuffer + s_startIndex, s_outputBuffer + s_endIndex);
	else // s_startIndex >= s_endIndex
	{
		// #TODO: ImGui inserts a little bit of vertical space between the two blocks, but really a minor concern
		// If a line wraps round the end of the buffer it is broken in two, but this is really a minor concern.
		// Could increase buffer size, and memcpy(s_outputBuffer + kOutputBufferSizeBytes, s_endIndex) to display
		// or better still do it during appending to hide the cost, but don't think it's worth it.
		ImGui::TextUnformatted(s_outputBuffer + s_startIndex, s_outputBuffer + kOutputBufferSizeBytes);
		ImGui::TextUnformatted(s_outputBuffer, s_outputBuffer + s_endIndex);
	}

	if (!useDefaultFont)
		ImGui::PopFont();

	if (copyToClipboard)
		ImGui::LogFinish();

	if (s_scrollToBottom)
	{
		s_scrollToBottom = false;
		ImGui::SetScrollHereY(1.0f);
	}

	ImGui::End();
}

bool OutputWindow::IsVisible()
{
	return s_visible;
}

void OutputWindow::SetVisible(bool visible)
{
	s_visible = visible;
}

void OutputWindow::Focus()
{
	s_focus = true;
}
