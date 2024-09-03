#pragma once

#include "HoffGui/Windows/OutputWindow.h"

#include "Core/FileSystem.h" // kMaxPath

struct ViewOptions
{
	FontType defaultFontType = FontType::ProggyClean13;
	OutputWindow::Options outputWindow;
};

struct ResourceOptions
{
	char vstDirectory[kMaxPath] = {};
};

struct Options
{
	ViewOptions view;
	ResourceOptions resource;
};

// e.g. "C:\Users\Howard\AppData\Roaming\TTE\hoffgui\hoffgui.ini"
void ConstructOptionsPath(char* optionsPath, size_t optionsPathSize);

bool LoadOptions(Options& options);
bool SaveOptions(const Options& options);

inline Options g_options;
