#include "AboutPopup.h"

#include "Core/StringHelpers.h"
#include "Core/Log.h"

#include "AppVersion.h"

#include "ImGuiWrap/ImGuiHelpers.h"
#include "ImGuiWrap/ImGuiWrap.h"
#include "imgui_markdown.h"

#include "SDL.h"

#ifdef _MSC_VER
#include <Windows.h> // ShellExecuteA
#endif

static const char kCreditsText[] =
	"Dear ImGui by Omar Cornut\n"
	"[https://github.com/ocornut/imgui](https://github.com/ocornut/imgui)\n"
	"\n"
	"Portable File Dialogs by Sam Hocevar\n"
	"[https://github.com/samhocevar/portable-file-dialogs](https://github.com/samhocevar/portable-file-dialogs)\n"
	"\n"
	"SDL2 by Sam Lantinga and contributors\n"
	"[https://www.libsdl.org/](https://www.libsdl.org/)\n"
	"\n"
	"tinyxml2 by Lee Thomason\n"
	"[https://github.com/leethomason/tinyxml2](https://github.com/leethomason/tinyxml2)\n"
	"\n"
	"imgui_markdown by Juliette Foucaut & Doug Binks\n"
	"[https://github.com/juliettef/imgui_markdown](https://github.com/juliettef/imgui_markdown)\n"
	"\n"
	"Squizz icon by gerbil/TTE\n"
	"\n"
	"proggy_fonts by Tristan Grimmer\n"
	"[https://github.com/bluescan/proggyfonts](https://github.com/bluescan/proggyfonts)\n"
	"\n"
	"topaz-unicode by Screwtape\n"
	"[https://gitlab.com/Screwtapello/topaz-unicode](https://gitlab.com/Screwtapello/topaz-unicode)\n"
	"\n"
	;

static const char kLinkText[] = "[https://www.twitch.tv/djh0ffman](https://www.twitch.tv/djh0ffman)";

static void aboutLinkCallback(ImGui::MarkdownLinkCallbackData data)
{
	LOG_TRACE("About link clicked. Text: %.*s  Link: %.*s\n", data.textLength, data.text, data.linkLength, data.link);

	if (data.linkLength == 0)
		return;

	if (data.isImage)
		return;

	// copy to local string so can null terminate
	char url[1024];
	SDL_memcpy(url, data.link, data.linkLength);
	url[data.linkLength] = '\0';

	if (strncmp(url, "http", 4) != 0)
		return;

#if defined _WIN32
	LOG_TRACE("Opening URL: %s\n", url);
	ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
#elif defined __linux__
	LOG_TRACE("Opening URL: %s\n", url);
	char command[1024];
	SafeSnprintf(command, sizeof(command), "xdg-open %s", url);
	int ret = system(command);
	HP_UNUSED(ret); // GCC warn_unused_result
#else
	LOG_WARN("Opening URL not supported on this platform\n");
#endif
}

void AboutPopup::Update()
{
	bool open = true; // required to add close button to popup
	if (ImGui::BeginPopupModal(kName, &open, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::IsWindowAppearing()) // see https://github.com/ocornut/imgui/issues/1061
			ImGui::SetKeyboardFocusHere();

		ImGui::MarkdownConfig mdConfig;
		mdConfig.linkCallback = aboutLinkCallback;

		ImGui::Text("hoffgui version %s%s", GetAppVersion(), GetAppVersonSuffix());
		ImGui::Spacing();
		ImGui::Markdown(kLinkText, sizeof(kLinkText), mdConfig);
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Text("Credits:");
		ImGui::BeginChild("CreditsChild", VEC2_FONT_UNITS(32, 16), ImGuiChildFlags_Border);
		ImGui::Markdown(kCreditsText, sizeof(kCreditsText), mdConfig);
		ImGui::EndChild();

		ImGui::SetCursorPosX(ImGuiHelpers::CalcRightAlignButtonX(-1)); // OK button right aligned at index -1
		if (ImGuiHelpers::FixedWidthButton("OK"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}
