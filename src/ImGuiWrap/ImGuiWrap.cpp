#include "ImGuiWrap.h"

#include "Core/Window.h"
#include "Core/StringHelpers.h"
#include "Core/Displays.h"
#include "Core/Log.h"

#include "ImGuiWrap/Fonts.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/imgui_internal.h"

#include "SDL.h"
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include "SDL_opengles2.h"
#else
#include "SDL_opengl.h"
#endif

static ImGuiStyle s_unscaledStyle; // unscaled reference style to use when DPI changes
static unsigned int s_zoomFactorIndex;
static unsigned int s_zoomFactorIndexPrev;

bool ImGuiWrap::Init(void* gl_context, const char* glsl_version)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
//	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
//	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	// Setup Dear ImGui style
	// #TODO: Expose to user
#if 1
	ImGui::StyleColorsDark();
#else
	ImGui::StyleColorsLight();
#endif

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	s_unscaledStyle = style;

	// You may modify the ImGui::GetStyle() main instance during initialization and before NewFrame().
	// Scale style for DPI
	// Scaling is required to ensure that the UI is not too fine on high DPI displays.
	unsigned int displayIndex = Window::GetDisplayIndex();
	float displayDpiScale = Displays::GetDpiScale(displayIndex);
	style.ScaleAllSizes(displayDpiScale);

	// Setup Platform/Renderer backends
	SDL_Window& window = Window::GetSDLWindow();
	if (!ImGui_ImplSDL2_InitForOpenGL(&window, gl_context))
	{
		fprintf(stderr, "Failed to initialise SDL2 for OpenGL\n");
		return false;
	}

	if (!ImGui_ImplOpenGL3_Init(glsl_version))
	{
		fprintf(stderr, "Failed to initialise OpenGL3\n");
		return false;
	}

	return true;
}

void ImGuiWrap::Shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

bool ImGuiWrap::ProcessEvent(const SDL_Event& event)
{
	return ImGui_ImplSDL2_ProcessEvent(&event);
}

//
// Needs to be called before ImGui::NewFrame();
//
static void updateDpiScale()
{
	if (ImGui::GetIO().KeyMods == ImGuiMod_Ctrl)
	{
		if (ImGui::IsKeyPressed(ImGuiKey_Equal, /*repeat*/false)) // + key
			ImGuiWrap::IncreaseZoom();
		else if (ImGui::IsKeyPressed(ImGuiKey_Minus, /*repeat*/false)) // - key
			ImGuiWrap::DecreaseZoom();
	}

	// Rebuild fonts if DPI scaling has changed
	// https://gist.github.com/ocornut/b3a9ecf13502fd818799a452969649ad
	if (s_zoomFactorIndex != s_zoomFactorIndexPrev)
	{
		s_zoomFactorIndexPrev = s_zoomFactorIndex;

		unsigned int displayIndex = Window::GetDisplayIndex();
		float displayDpiScale = Displays::GetDpiScale(displayIndex);

		float zoom = kZoomFactors[s_zoomFactorIndex];

		ImGuiStyle& style = ImGui::GetStyle();
		style = s_unscaledStyle; // set to unscaled reference style
		style.ScaleAllSizes(displayDpiScale * zoom);

		// Rebuild (scaled) fonts for this DPI
		Fonts::Clear();
		Fonts::Load(zoom);

		// Rebuild font atlas
		ImFontAtlas* pFontAtlas = ImGui::GetIO().Fonts;
		if (pFontAtlas->Build())
		{
			LOG_TRACE("Rebuilt font atlas\n");

			// REUPLOAD FONT TEXTURE TO GPU
			ImGui_ImplOpenGL3_DestroyFontsTexture();
			ImGui_ImplOpenGL3_CreateFontsTexture();

			// ImGuiIO.FontDefault will have been invalidated, so set to new font 
			ImGui::GetIO().FontDefault = nullptr;
		}
		else
		{
			LOG_ERROR("Failed to rebuild font atlas\n");
		}
	}
}

void ImGuiWrap::NewFrame(ImFont* pFont)
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();

	updateDpiScale();

	if (ImGui::GetIO().FontDefault == nullptr)
		ImGui::GetIO().FontDefault = pFont;

	ImGui::NewFrame();
}

void ImGuiWrap::Render(const ImVec4& clearColor)
{
	ImGui::Render();

	ImGuiIO& io = ImGui::GetIO();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);

	glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Update and Render additional Platform Windows
	// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
	//  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
		SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
	}

	SDL_Window& window = Window::GetSDLWindow();
	SDL_GL_SwapWindow(&window);
}

bool ImGuiWrap::CanIncreaseZoom()
{
	return s_zoomFactorIndex < IM_ARRAYSIZE(kZoomFactors) - 1;
}

void ImGuiWrap::IncreaseZoom()
{
	if (s_zoomFactorIndex < IM_ARRAYSIZE(kZoomFactors) - 1)
	{
		s_zoomFactorIndex++;
		LOG_TRACE("Increased UI zoom to %.2f\n", kZoomFactors[s_zoomFactorIndex]);
	}
}

bool ImGuiWrap::CanDecreaseZoom()
{
	return s_zoomFactorIndex > 0;
}

void ImGuiWrap::DecreaseZoom()
{
	if (s_zoomFactorIndex > 0)
	{
		s_zoomFactorIndex--;
		LOG_TRACE("Decreased UI zoom to %.2f\n", kZoomFactors[s_zoomFactorIndex]);
	}
}

float ImGuiWrap::GetZoom()
{
	return kZoomFactors[s_zoomFactorIndex];
}

unsigned int ImGuiWrap::GetZoomFactorIndex()
{
	return s_zoomFactorIndex;
}

void ImGuiWrap::SetZoomFactorIndex(unsigned int zoomFactorIndex)
{
	if (zoomFactorIndex < COUNTOF_ARRAY(kZoomFactors))
	{
		s_zoomFactorIndex = zoomFactorIndex;
	}
	else
	{
		LOG_ERROR("Invalid zoom factor index %u\n", zoomFactorIndex);
	}
}

//-----------------------------------------------------------------------------

namespace ImGui
{

void HelpMarker(const char* fmt, ...)
{
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(450.0f);
		va_list args;
		va_start(args, fmt);
		ImGui::TextV(fmt, args);
		va_end(args);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void CopyHexByteToClipboard(ImU8 val)
{
	char valueText[16];
	SafeSnprintf(valueText, sizeof(valueText), "%02X", val);
	ImGui::SetClipboardText(valueText);
}

void CopyHexWordToClipboard(ImU16 val)
{
	char valueText[16];
	SafeSnprintf(valueText, sizeof(valueText), "%04X", val);
	ImGui::SetClipboardText(valueText);
}

void CopyHexLongToClipboard(ImU32 val)
{
	char valueText[16];
	SafeSnprintf(valueText, sizeof(valueText), "%08X", val);
	ImGui::SetClipboardText(valueText);
}

void PushStyleCompact()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, (float)(int)(style.FramePadding.y * 0.60f)));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, (float)(int)(style.ItemSpacing.y * 0.60f)));
}

void PopStyleCompact()
{
	ImGui::PopStyleVar(2);
}

int FindWindowFocusIndexByName(const char* name)
{
	ImGuiWindow* pWindow = ImGui::FindWindowByName(name);
	if (!pWindow)
		return -1;
	return pWindow->FocusOrder;
}

} // namespace ImGui
