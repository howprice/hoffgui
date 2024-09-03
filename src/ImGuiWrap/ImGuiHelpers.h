#pragma once

#include "Core/Helpers.h"

class ImGuiHelpers
{
public:

	NON_INSTANTIABLE_STATIC_CLASS(ImGuiHelpers);

	// Fixed width buttons for use as dialog box "commit buttons" e.g. OK and Cancel
	// 
	// Microsoft guidelines https://learn.microsoft.com/en-us/windows/win32/uxguide/win-dialog-box
	// Commit buttons should be at the bottom right of the dialog box.
	// 
	// Present the commit buttons in the following order:
	// - OK/[Do it]/Yes
	// - [Don't do it]/No
	// - Cancel
	// - Apply (if present)
	// - Help (if present)
	// 
	// It is often a good idea to precede the buttons with vertical space i.e. ImGui::Separator or ImGui::Spacing
	static bool FixedWidthButton(const char* label);
	static float CalcRightAlignButtonX(int index); // index is negative number where -1 is rightmost

};
