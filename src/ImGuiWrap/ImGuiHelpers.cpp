#include "ImGuiHelpers.h"

#include "Core/FileSystem.h"
#include "Core/Log.h"
#include "Core/hp_assert.h"

#include "ImGuiWrap/ImGuiWrap.h"

static const float kOkCancelButtonWidthFontUnits = 6.0f;

bool ImGuiHelpers::FixedWidthButton(const char* label)
{
	return ImGui::Button(label, VEC2_FONT_UNITS(kOkCancelButtonWidthFontUnits, 0));
}

float ImGuiHelpers::CalcRightAlignButtonX(int index)
{
	HP_ASSERT(index < 0);
	float windowWidth = ImGui::GetWindowWidth();
	float buttonWidth = DIM_FONT_UNITS(kOkCancelButtonWidthFontUnits);
	float spacingX = ImGui::GetStyle().ItemSpacing.x;
	float x = windowWidth + index * (buttonWidth + spacingX);
	return x;
}
