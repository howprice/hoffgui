#include "OptionsWindow.h"

#include "HoffGui/Dialogues/FileDialogue.h"
#include "HoffGui/Options.h"

#include "Core/StringHelpers.h"
#include "Core/Log.h"

#include "ImGuiWrap/ImGuiWrap.h"

static bool s_visible = false;
static bool s_focus;

enum class OptionsView
{
	Resources,
	Fonts,
	Logging,

	Max = Logging
};

static OptionsView s_optionsView = OptionsView::Resources;

static void showResourceOptions()
{
	ResourceOptions& resourceOptions = g_options.resource;

	// VST directory
	{
		ImGui::Text("VST directory:");
		ImGui::HelpMarker("Directory containing VST files");

		static char kDefaultString[] = "<default>";
		if (resourceOptions.vstDirectory[0])
			ImGui::InputText("##VSTDirectoryInputText", resourceOptions.vstDirectory, sizeof(resourceOptions.vstDirectory), ImGuiInputTextFlags_ReadOnly);
		else
			ImGui::InputText("##VSTDirectoryInputText", kDefaultString, sizeof(kDefaultString), ImGuiInputTextFlags_ReadOnly);

		ImGui::SameLine();
		if (ImGui::Button("...##SelectVSTDirectory"))
			FileDialogue::FolderDialogue("VST directory", resourceOptions.vstDirectory, sizeof(resourceOptions.vstDirectory)); // blocking call
		ImGui::SameLine();
		if (ImGui::Button("Default##DefaultVSTDirectory"))
			resourceOptions.vstDirectory[0] = '\0';
	}
}

void showFontsOptions()
{
	ViewOptions& viewOptions = g_options.view;

	const float fontComboWidth = DIM_96_PPI(256.0f);

	ImGui::PushItemWidth(fontComboWidth);
	ImGui::Combo("Default font", (int*)&viewOptions.defaultFontType, kFontTypeNames, COUNTOF_ARRAY(kFontTypeNames));
	ImGui::PopItemWidth();

	// Output Window font
	ImGui::Spacing();
	if (viewOptions.outputWindow.useDefaultFont)
		ImGui::BeginDisabled();
	ImGui::PushItemWidth(fontComboWidth);
	ImGui::Combo("Output window font", (int*)&viewOptions.outputWindow.fontType, kFontTypeNames, COUNTOF_ARRAY(kFontTypeNames));
	ImGui::PopItemWidth();
	if (viewOptions.outputWindow.useDefaultFont)
		ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::Checkbox("Use default font", &viewOptions.outputWindow.useDefaultFont);
}

void showLoggingOptions()
{
	// Careful here: log levels are both -ve and +ve
	int logLevel = GetLogLevel() - LOG_LEVEL_MIN; // convert to 0-based index
	ImGui::PushItemWidth(DIM_96_PPI(100.0f));
	if (ImGui::Combo("Log level", &logLevel, "None\0Error\0Warn\0Info\0Debug\0Trace\0"))
		SetLogLevel(logLevel + LOG_LEVEL_MIN);
	ImGui::PopItemWidth();
}

static void clickableLeafNodeSelector(const char* label, OptionsView view)
{
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
	flags |= ImGuiTreeNodeFlags_NoTreePushOnOpen; // avoid need to call ImGui::TreePop()
	if (s_optionsView == view)
		flags |= ImGuiTreeNodeFlags_Selected;
	ImGui::TreeNodeEx(label, flags);
	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
		s_optionsView = view;
}

typedef void (*OptionsFunc)(void);

static const OptionsFunc kOptionsFuncs[] =
{
	showResourceOptions,
	showFontsOptions,
	showLoggingOptions,
};
static_assert(COUNTOF_ARRAY(kOptionsFuncs) == ENUM_COUNT(OptionsView));

void OptionsWindow::Update()
{
	if (!s_visible)
		return;

	if (s_focus)
	{
		ImGui::SetNextWindowFocus();
		s_focus = false;
	}

	ImGui::SetNextWindowSize(VEC2_96_PPI(640, 480), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin(kWindowName, &s_visible, ImGuiWindowFlags_NoDocking))
	{
		ImGui::End();
		return;
	}

	// Selection tree on the left
	ImGui::BeginChild("ChildL", VEC2_96_PPI(200, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX, ImGuiWindowFlags_HorizontalScrollbar);

	// #TODO: Could macro this up
	clickableLeafNodeSelector("Resources", OptionsView::Resources);
	clickableLeafNodeSelector("Fonts", OptionsView::Fonts);
	clickableLeafNodeSelector("Logging", OptionsView::Logging);

	ImGui::EndChild();

	// Options view on the right for selected category
	ImGui::SameLine();
	ImGui::BeginChild("ChildR", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
	kOptionsFuncs[ToNumber(s_optionsView)]();
	ImGui::EndChild();
	ImGui::End();
}

void OptionsWindow::Focus()
{
	s_focus = true;
}

bool OptionsWindow::IsVisible()
{
	return s_visible;
}

void OptionsWindow::SetVisible(bool visible)
{
	s_visible = visible;
}
