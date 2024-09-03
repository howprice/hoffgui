#pragma once

#include "Core/Helpers.h"

class ModWindow
{
public:

	NON_INSTANTIABLE_STATIC_CLASS(ModWindow);

	// So can replace visible section but keep hash the same. 
	// n.b For "label###id" the "###" is included in the hash(!) but only "label" gets displayed. See ImHashStr comment.
	static constexpr char kWindowName[] = "MOD###MOD";

	static void Init();
	static void Shutdown();

	// Call once per frame
	static void Update();

	static void SetFilePath(const char* path);

	static bool IsVisible();
	static void SetVisible(bool visible);

	static const char* GetWindowName();
};
